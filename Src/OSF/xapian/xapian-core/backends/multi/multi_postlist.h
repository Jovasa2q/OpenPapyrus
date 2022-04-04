/** @file
 * @brief Class for merging PostList objects from subdatabases.
 */
/* Copyright (C) 2007,2008,2009,2011,2015,2017,2020 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */
#ifndef XAPIAN_INCLUDED_MULTI_POSTLIST_H
#define XAPIAN_INCLUDED_MULTI_POSTLIST_H

#include "backends/postlist.h"
#include "backends/positionlist.h"

/// Class for merging PostList objects from subdatabases.
class MultiPostList : public PostList {
	void operator = (const MultiPostList &) = delete; /// Don't allow assignment.
	MultiPostList(const MultiPostList &) = delete; /// Don't allow copying.
	Xapian::doccount n_shards; /// Number of PostList * entries in @a postlists.
	PostList ** postlists; /// Sub-postlists which we use as a heap.
	Xapian::doccount docids_size = 0; /// Number of entries in docids;
	Xapian::docid* docids = nullptr; /// Heap of docids from the current positions of the postlists.
public:
	MultiPostList(Xapian::doccount n_shards_, PostList ** postlists_) : n_shards(n_shards_), postlists(postlists_)
	{
		try {
			docids = new Xapian::docid[n_shards];
		} catch(...) {
			delete [] postlists;
			throw;
		}
	}
	~MultiPostList();
	Xapian::doccount get_termfreq_min() const; /// Get a lower bound on the number of documents indexed by this term.
	Xapian::doccount get_termfreq_max() const; /// Get an upper bound on the number of documents indexed by this term.
	Xapian::doccount get_termfreq_est() const; /// Get an estimate of the number of documents indexed by this term.
	Xapian::docid get_docid() const; /// Return the current docid.
	Xapian::termcount get_wdf() const; /// Return the wdf for the document at the current position.
	/// Return the weight contribution for the current position.
	double get_weight(Xapian::termcount doclen, Xapian::termcount unique_terms, Xapian::termcount wdfdocmax) const;
	/// Return true if the current position is past the last entry in this list.
	bool at_end() const;
	/// Recalculate the upper bound on what get_weight() can return.
	double recalc_maxweight();
	/// Read the position list for the term in the current document and
	PositionList * open_position_list() const;

	/** Advance the current position to the next document in the postlist.
	 *
	 *  The list starts before the first entry in the list, so next(),
	 *  skip_to() or check() must be called before any methods which need the
	 *  context of the current position.
	 *
	 *  @param w_min	The minimum weight contribution that is needed (this is
	 *			just a hint which PostList subclasses may ignore).
	 *
	 *  @return	If a non-NULL pointer is returned, then the caller should
	 *		substitute the returned pointer for its pointer to us, and then
	 *		delete us.  This "pruning" can only happen for a non-leaf
	 *		subclass of this class.
	 */
	PostList * next(double w_min);

	/** Skip forward to the specified docid.
	 *
	 *  If the specified docid isn't in the list, position ourselves on the
	 *  first document after it (or at_end() if no greater docids are present).
	 *
	 *  @param w_min	The minimum weight contribution that is needed (this is
	 *			just a hint which PostList subclasses may ignore).
	 *
	 *  @return	If a non-NULL pointer is returned, then the caller should
	 *		substitute the returned pointer for its pointer to us, and then
	 *		delete us.  This "pruning" can only happen for a non-leaf
	 *		subclass of this class.
	 */
	PostList * skip_to(Xapian::docid, double w_min);

	// We don't implement check() because we're only used in a PostingIterator
	// wrapper and that doesn't call check().
	//
	// Should that change, we could handle check() a bit more efficiently with
	// some extra bookkeeping on operations after check(), because we know
	// which subdatabase a given docid will be in, and so we only actually need
	// to call check() on that subdatabase.
	std::string get_description() const; /// Return a string description of this object.
};

#endif // XAPIAN_INCLUDED_MULTI_POSTLIST_H
