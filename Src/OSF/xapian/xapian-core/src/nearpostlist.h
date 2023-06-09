/** @file
 * @brief Return docs containing terms within a specified window.
 *
 * Copyright (C) 2006,2015,2017 Olly Betts
 * Copyright (C) 2009 Lemur Consulting Ltd
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef XAPIAN_INCLUDED_NEARPOSTLIST_H
#define XAPIAN_INCLUDED_NEARPOSTLIST_H

class PostListTree;

/** Postlist which matches terms occurring within a specified window.
 *
 *  NearPostList only returns a posting for documents contains all the terms
 *  (this part is implemented using an AndPostList) and additionally the terms
 *  occur somewhere in the document within a specified number of term
 *  positions.
 *
 *  The weight of a posting is the sum of the weights of the
 *  sub-postings (just like an AndPostList).
 */
class NearPostList : public SelectPostList {
	Xapian::termpos window;
	std::vector <PostList *> terms;
	PositionList ** poslists;
	bool test_doc(); /// Test if the current document contains the terms within the window.
public:
	NearPostList(PostList * source_, Xapian::termpos window_, const std::vector <PostList *>::const_iterator &terms_begin,
	    const std::vector <PostList *>::const_iterator &terms_end, PostListTree* pltree_);
	~NearPostList();
	Xapian::termcount get_wdf() const;
	Xapian::doccount get_termfreq_est() const;
	TermFreqs get_termfreq_est_using_stats(const Xapian::Weight::Internal & stats) const;
	std::string get_description() const;
};

#endif
