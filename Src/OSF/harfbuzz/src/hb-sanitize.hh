/*
 * Copyright © 2007,2008,2009,2010  Red Hat, Inc.
 * Copyright © 2012,2018  Google, Inc.
 *
 *  This is part of HarfBuzz, a text shaping library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * Red Hat Author(s): Behdad Esfahbod
 * Google Author(s): Behdad Esfahbod
 */
#ifndef HB_SANITIZE_HH
#define HB_SANITIZE_HH

#include "hb.hh"
#include "hb-blob.hh"
#include "hb-dispatch.hh"

/*
 * Sanitize
 *
 *
 * === Introduction ===
 *
 * The sanitize machinery is at the core of our zero-cost font loading.  We
 * mmap() font file into memory and create a blob out of it.  Font subtables
 * are returned as a readonly sub-blob of the main font blob.  These table
 * blobs are then sanitized before use, to ensure invalid memory access does
 * not happen.  The toplevel sanitize API use is like, eg. to load the 'head'
 * table:
 *
 *   hb_blob_t *head_blob = hb_sanitize_context_t ().reference_table<OT::head> (face);
 *
 * The blob then can be converted to a head table struct with:
 *
 *   const head *head_table = head_blob->as<head> ();
 *
 * What the reference_table does is, to call hb_face_reference_table() to load
 * the table blob, sanitize it and return either the sanitized blob, or empty
 * blob if sanitization failed.  The blob->as() function returns the null
 * object of its template type argument if the blob is empty.  Otherwise, it
 * just casts the blob contents to the desired type.
 *
 * Sanitizing a blob of data with a type T works as follows (with minor
 * simplification):
 *
 *   - Cast blob content to T*, call sanitize() method of it,
 *   - If sanitize succeeded, return blob.
 *   - Otherwise, if blob is not writable, try making it writable,
 *     or copy if cannot be made writable in-place,
 *   - Call sanitize() again.  Return blob if sanitize succeeded.
 *   - Return empty blob otherwise.
 *
 *
 * === The sanitize() contract ===
 *
 * The sanitize() method of each object type shall return true if it's safe to
 * call other methods of the object, and false otherwise.
 *
 * Note that what sanitize() checks for might align with what the specification
 * describes as valid table data, but does not have to be.  In particular, we
 * do NOT want to be pedantic and concern ourselves with validity checks that
 * are irrelevant to our use of the table.  On the contrary, we want to be
 * lenient with error handling and accept invalid data to the extent that it
 * does not impose extra burden on us.
 *
 * Based on the sanitize contract, one can see that what we check for depends
 * on how we use the data in other table methods.  Ie. if other table methods
 * assume that offsets do NOT point out of the table data block, then that's
 * something sanitize() must check for (GSUB/GPOS/GDEF/etc work this way).  On
 * the other hand, if other methods do such checks themselves, then sanitize()
 * does not have to bother with them (glyf/local work this way).  The choice
 * depends on the table structure and sanitize() performance.  For example, to
 * check glyf/loca offsets in sanitize() would cost O(num-glyphs).  We try hard
 * to avoid such costs during font loading.  By postponing such checks to the
 * actual glyph loading, we reduce the sanitize cost to O(1) and total runtime
 * cost to O(used-glyphs).  As such, this is preferred.
 *
 * The same argument can be made re GSUB/GPOS/GDEF, but there, the table
 * structure is so complicated that by checking all offsets at sanitize() time,
 * we make the code much simpler in other methods, as offsets and referenced
 * objects do not need to be validated at each use site.
 */

/* This limits sanitizing time on really broken fonts. */
#ifndef HB_SANITIZE_MAX_EDITS
#define HB_SANITIZE_MAX_EDITS 32
#endif
#ifndef HB_SANITIZE_MAX_OPS_FACTOR
#define HB_SANITIZE_MAX_OPS_FACTOR 8
#endif
#ifndef HB_SANITIZE_MAX_OPS_MIN
#define HB_SANITIZE_MAX_OPS_MIN 16384
#endif
#ifndef HB_SANITIZE_MAX_OPS_MAX
#define HB_SANITIZE_MAX_OPS_MAX 0x3FFFFFFF
#endif
#ifndef HB_SANITIZE_MAX_SUTABLES
#define HB_SANITIZE_MAX_SUTABLES 0x4000
#endif

struct hb_sanitize_context_t : hb_dispatch_context_t<hb_sanitize_context_t, bool, HB_DEBUG_SANITIZE> {
	hb_sanitize_context_t() : start(nullptr), end(nullptr), max_ops(0), max_subtables(0),
		writable(false), edit_count(0), blob(nullptr), num_glyphs(65536), num_glyphs_set(false) 
	{
	}
	const char * get_name() { return "SANITIZE"; }
	template <typename T, typename F> bool may_dispatch(const T * obj CXX_UNUSED_PARAM, const F * format)
	{
		return format->sanitize(this);
	}
	static return_t default_return_value() { return true; }
	static return_t no_dispatch_return_value() { return false; }
	bool stop_sublookup_iteration(const return_t r) const { return !r; }
	bool visit_subtables(uint count)
	{
		max_subtables += count;
		return max_subtables < HB_SANITIZE_MAX_SUTABLES;
	}
private:
	template <typename T, typename ... Ts> auto _dispatch(const T &obj, hb_priority<1>, Ts&&... ds) HB_AUTO_RETURN
		(obj.sanitize(this, hb_forward<Ts> (ds) ...) )
	template <typename T, typename ... Ts> auto _dispatch(const T &obj, hb_priority<0>, Ts&&... ds) HB_AUTO_RETURN
		(obj.dispatch(this, hb_forward<Ts> (ds) ...) )
public:
	template <typename T, typename ... Ts> auto dispatch(const T &obj, Ts&&... ds) HB_AUTO_RETURN
		(_dispatch(obj, hb_prioritize, hb_forward<Ts> (ds) ...) )
	void init(hb_blob_t * b)
	{
		this->blob = hb_blob_reference(b);
		this->writable = false;
	}
	void set_num_glyphs(uint num_glyphs_)
	{
		num_glyphs = num_glyphs_;
		num_glyphs_set = true;
	}
	uint get_num_glyphs() const { return num_glyphs; }
	void set_max_ops(int max_ops_) 
	{
		max_ops = max_ops_;
	}
	template <typename T> void set_object(const T * obj)
	{
		reset_object();
		if(!obj) return;
		const char * obj_start = (const char*)obj;
		if(UNLIKELY(obj_start < this->start || this->end <= obj_start))
			this->start = this->end = nullptr;
		else {
			this->start = obj_start;
			this->end   = obj_start + hb_min(size_t(this->end - obj_start), obj->get_size());
		}
	}
	void reset_object()
	{
		this->start = this->blob->data;
		this->end = this->start + this->blob->length;
		assert(this->start <= this->end); /* Must not overflow. */
	}
	void start_processing()
	{
		reset_object();
		if(UNLIKELY(hb_unsigned_mul_overflows(this->end - this->start, HB_SANITIZE_MAX_OPS_FACTOR)))
			this->max_ops = HB_SANITIZE_MAX_OPS_MAX;
		else
			this->max_ops = hb_clamp((uint)(this->end - this->start) * HB_SANITIZE_MAX_OPS_FACTOR, (uint)HB_SANITIZE_MAX_OPS_MIN, (uint)HB_SANITIZE_MAX_OPS_MAX);
		this->edit_count = 0;
		this->debug_depth = 0;
		DEBUG_MSG_LEVEL(SANITIZE, start, 0, +1, "start [%p..%p] (%lu bytes)", this->start, this->end, (ulong)(this->end - this->start));
	}
	void end_processing()
	{
		DEBUG_MSG_LEVEL(SANITIZE, this->start, 0, -1, "end [%p..%p] %u edit requests", this->start, this->end, this->edit_count);
		hb_blob_destroy(this->blob);
		this->blob = nullptr;
		this->start = this->end = nullptr;
	}
	uint get_edit_count() const { return edit_count; }
	bool check_range(const void * base, uint len) const
	{
		const char * p = (const char*)base;
		bool ok = !len || (this->start <= p && p <= this->end && (uint)(this->end - p) >= len && this->max_ops-- > 0);
		DEBUG_MSG_LEVEL(SANITIZE, p, this->debug_depth+1, 0, "check_range [%p..%p] (%d bytes) in [%p..%p] -> %s",
		    p, p + len, len, this->start, this->end, ok ? "OK" : "OUT-OF-RANGE");
		return LIKELY(ok);
	}
	template <typename T> bool check_range(const T * base, uint a, uint b) const
	{
		return !hb_unsigned_mul_overflows(a, b) && this->check_range(base, a * b);
	}
	template <typename T> bool check_range(const T * base, uint a, uint b, uint c) const
	{
		return !hb_unsigned_mul_overflows(a, b) && this->check_range(base, a * b, c);
	}
	template <typename T> bool check_array(const T * base, uint len) const
	{
		return this->check_range(base, len, hb_static_size(T));
	}
	template <typename T> bool check_array(const T * base, uint a, uint b) const
	{
		return this->check_range(base, a, b, hb_static_size(T));
	}
	template <typename Type> bool check_struct(const Type * obj) const
	{
		return LIKELY(this->check_range(obj, obj->min_size));
	}
	bool may_edit(const void * base, uint len)
	{
		if(this->edit_count >= HB_SANITIZE_MAX_EDITS)
			return false;
		const char * p = (const char*)base;
		this->edit_count++;
		DEBUG_MSG_LEVEL(SANITIZE, p, this->debug_depth+1, 0,
		    "may_edit(%u) [%p..%p] (%d bytes) in [%p..%p] -> %s",
		    this->edit_count,
		    p, p + len, len,
		    this->start, this->end,
		    this->writable ? "GRANTED" : "DENIED");

		return this->writable;
	}

	template <typename Type, typename ValueType>
	bool try_set(const Type * obj, const ValueType &v)
	{
		if(this->may_edit(obj, hb_static_size(Type))) {
			*const_cast<Type *> (obj) = v;
			return true;
		}
		return false;
	}

	template <typename Type>
	hb_blob_t * sanitize_blob(hb_blob_t * blob)
	{
		bool sane;

		init(blob);

retry:
		DEBUG_MSG_FUNC(SANITIZE, start, "start");

		start_processing();

		if(UNLIKELY(!start)) {
			end_processing();
			return blob;
		}

		Type * t = reinterpret_cast<Type *> (const_cast<char *> (start));

		sane = t->sanitize(this);
		if(sane) {
			if(edit_count) {
				DEBUG_MSG_FUNC(SANITIZE, start, "passed first round with %d edits; going for second round", edit_count);

				/* sanitize again to ensure no toe-stepping */
				edit_count = 0;
				sane = t->sanitize(this);
				if(edit_count) {
					DEBUG_MSG_FUNC(SANITIZE, start, "requested %d edits in second round; FAILLING", edit_count);
					sane = false;
				}
			}
		}
		else {
			if(edit_count && !writable) {
				start = hb_blob_get_data_writable(blob, nullptr);
				end = start + blob->length;

				if(start) {
					writable = true;
					/* ok, we made it writable by relocating.  try again */
					DEBUG_MSG_FUNC(SANITIZE, start, "retry");
					goto retry;
				}
			}
		}

		end_processing();

		DEBUG_MSG_FUNC(SANITIZE, start, sane ? "PASSED" : "FAILED");
		if(sane) {
			hb_blob_make_immutable(blob);
			return blob;
		}
		else {
			hb_blob_destroy(blob);
			return hb_blob_get_empty();
		}
	}

	template <typename Type>
	hb_blob_t * reference_table(const hb_face_t * face, hb_tag_t tableTag = Type::tableTag)
	{
		if(!num_glyphs_set)
			set_num_glyphs(hb_face_get_glyph_count(face));
		return sanitize_blob<Type> (hb_face_reference_table(face, tableTag));
	}

	const char * start, * end;
	mutable int max_ops, max_subtables;
private:
	bool writable;
	uint edit_count;
	hb_blob_t * blob;
	uint num_glyphs;
	bool num_glyphs_set;
};

struct hb_sanitize_with_object_t {
	template <typename T>
	hb_sanitize_with_object_t(hb_sanitize_context_t *c, const T &obj) : c(c)
	{
		c->set_object(obj);
	}
	~hb_sanitize_with_object_t ()
	{
		c->reset_object();
	}

private:
	hb_sanitize_context_t * c;
};

#endif /* HB_SANITIZE_HH */
