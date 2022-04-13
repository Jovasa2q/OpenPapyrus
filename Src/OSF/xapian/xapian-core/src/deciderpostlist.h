/** @file
 * @brief PostList which applies a MatchDecider
 */
/* Copyright (C) 2017 Olly Betts
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef XAPIAN_INCLUDED_DECIDERPOSTLIST_H
#define XAPIAN_INCLUDED_DECIDERPOSTLIST_H

namespace Xapian {
	class MatchDecider;
}

/// PostList which applies a MatchDecider
class DeciderPostList : public SelectPostList {
	const Xapian::MatchDecider* decider; /// The MatchDecider to apply.
	Xapian::Document doc; /// The document to test.
	bool test_doc(); /// Test the current with the MatchDecider.
public:
	DeciderPostList(PostList * pl_, const Xapian::MatchDecider* decider_, ValueStreamDocument* vsdoc, PostListTree* pltree_) : 
		SelectPostList(pl_, pltree_), decider(decider_), doc(vsdoc)
	{
		// These get zeroed once per shard in use, but that all happens before
		// the match starts so isn't a problem.
		decider->docs_allowed_ = decider->docs_denied_ = 0;
	}
	std::string get_description() const;
};

#endif // XAPIAN_INCLUDED_DECIDERPOSTLIST_H
