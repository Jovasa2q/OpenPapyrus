/** @file
 *  @brief Class for iterating over term positions.
 */
/* Copyright (C) 2008,2009,2010,2011,2012,2013,2014,2015 Olly Betts
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 */
#ifndef XAPIAN_INCLUDED_POSITIONITERATOR_H
#define XAPIAN_INCLUDED_POSITIONITERATOR_H

#if !defined XAPIAN_IN_XAPIAN_H && !defined XAPIAN_LIB_BUILD
	#error Never use <xapian/positioniterator.h> directly; include <xapian.h> instead.
#endif

#include <xapian/attributes.h>
#include <xapian/derefwrapper.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

namespace Xapian {
/// Class for iterating over term positions.
class XAPIAN_VISIBILITY_DEFAULT PositionIterator {
	void decref();
public:
	/// Class representing the PositionIterator internals.
	class Internal;
	/// @private @internal Reference counted internals.
	Internal * internal;
	/// @private @internal Wrap an existing Internal.
	XAPIAN_VISIBILITY_INTERNAL
	explicit PositionIterator(Internal * internal_);
	PositionIterator(const PositionIterator & o); /// Copy constructor.
	PositionIterator & operator = (const PositionIterator & o); /// Assignment.
	PositionIterator(PositionIterator && o) : internal(o.internal)  /// Move constructor.
	{
		o.internal = nullptr;
	}
	/// Move assignment operator.
	PositionIterator & operator = (PositionIterator && o) {
		if(this != &o) {
			if(internal) 
				decref();
			internal = o.internal;
			o.internal = nullptr;
		}
		return *this;
	}

	/** Default constructor.
	 *
	 *  Creates an uninitialised iterator, which can't be used before being
	 *  assigned to, but is sometimes syntactically convenient.
	 */
	PositionIterator() noexcept : internal(0) 
	{
	}
	~PositionIterator() 
	{
		if(internal) 
			decref();
	}
	/// Return the term position at the current iterator position.
	Xapian::termpos operator*() const;
	/// Advance the iterator to the next position.
	PositionIterator & operator++();
	/// Advance the iterator to the next position (postfix version).
	DerefWrapper_<Xapian::termpos> operator++(int) 
	{
		Xapian::termpos pos(**this);
		operator++();
		return DerefWrapper_<Xapian::termpos>(pos);
	}
	/** Advance the iterator to term position @a termpos.
	 *
	 *  @param termpos	The position to advance to.  If this position isn't in
	 *			the stream being iterated, then the iterator is moved
	 *			to the next term position after it which is.
	 */
	void skip_to(Xapian::termpos termpos);
	/// Return a string describing this object.
	std::string get_description() const;
	/** @private @internal PositionIterator is what the C++ STL calls an
	 *  input_iterator.
	 *
	 *  The following typedefs allow std::iterator_traits<> to work so that
	 *  this iterator can be used with the STL.
	 *
	 *  These are deliberately hidden from the Doxygen-generated docs, as the
	 *  machinery here isn't interesting to API users.  They just need to know
	 *  that Xapian iterator classes are compatible with the STL.
	 */
	// @{
	typedef std::input_iterator_tag iterator_category; /// @private
	typedef Xapian::termpos value_type; /// @private
	typedef Xapian::termpos_diff difference_type; /// @private
	typedef Xapian::termpos * pointer; /// @private
	typedef Xapian::termpos & reference; /// @private
	// @}
};

/// Equality test for PositionIterator objects.
inline bool operator == (const PositionIterator& a, const PositionIterator& b) noexcept
{
	// Use a pointer comparison - this ensures both that (a == a) and correct
	// handling of end iterators (which we ensure have NULL internals).
	return a.internal == b.internal;
}

/// Inequality test for PositionIterator objects.
inline bool operator != (const PositionIterator& a, const PositionIterator& b) noexcept
{
	return !(a == b);
}
}

#endif // XAPIAN_INCLUDED_POSITIONITERATOR_H
