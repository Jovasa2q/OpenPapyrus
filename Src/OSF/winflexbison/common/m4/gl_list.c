/* Abstract sequential list data type.
   Copyright (C) 2006-2011 Free Software Foundation, Inc.
   Written by Bruno Haible <bruno@clisp.org>, 2006.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

#include <flexbison_common.h>
#pragma hdrstop
#include "gl_list.h" /* Specification.  */

#if !HAVE_INLINE

/* Define all functions of this file as inline accesses to the
   struct gl_list_implementation.
   Use #define to avoid a warning because of extern vs. static.  */

gl_list_t gl_list_nx_create_empty(gl_list_implementation_t implementation,
    gl_listelement_equals_fn equals_fn,
    gl_listelement_hashcode_fn hashcode_fn,
    gl_listelement_dispose_fn dispose_fn,
    bool allow_duplicates)
{
	return implementation->nx_create_empty(implementation, equals_fn,
		   hashcode_fn, dispose_fn,
		   allow_duplicates);
}

gl_list_t gl_list_nx_create(gl_list_implementation_t implementation,
    gl_listelement_equals_fn equals_fn,
    gl_listelement_hashcode_fn hashcode_fn,
    gl_listelement_dispose_fn dispose_fn,
    bool allow_duplicates,
    size_t count, const void ** contents)
{
	return implementation->nx_create(implementation, equals_fn, hashcode_fn,
		   dispose_fn, allow_duplicates, count,
		   contents);
}

size_t gl_list_size(gl_list_t list)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->size(list);
}

const void * gl_list_node_value(gl_list_t list, gl_list_node_t node)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->node_value(list, node);
}

int gl_list_node_nx_set_value(gl_list_t list, gl_list_node_t node,
    const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->node_nx_set_value(list, node, elt);
}

gl_list_node_t gl_list_next_node(gl_list_t list, gl_list_node_t node)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->next_node(list, node);
}

gl_list_node_t gl_list_previous_node(gl_list_t list, gl_list_node_t node)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->previous_node(list, node);
}

const void * gl_list_get_at(gl_list_t list, size_t position)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->get_at(list, position);
}

gl_list_node_t gl_list_nx_set_at(gl_list_t list, size_t position, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->nx_set_at(list, position, elt);
}

gl_list_node_t gl_list_search(gl_list_t list, const void * elt)
{
	size_t size = ((const struct gl_list_impl_base *)list)->vtable->size(list);
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->search_from_to(list, 0, size, elt);
}

gl_list_node_t gl_list_search_from(gl_list_t list, size_t start_index, const void * elt)
{
	size_t size = ((const struct gl_list_impl_base *)list)->vtable->size(list);
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->search_from_to(list, start_index, size, elt);
}

gl_list_node_t gl_list_search_from_to(gl_list_t list, size_t start_index, size_t end_index, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->search_from_to(list, start_index, end_index, elt);
}

size_t gl_list_indexof(gl_list_t list, const void * elt)
{
	size_t size = ((const struct gl_list_impl_base *)list)->vtable->size(list);
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->indexof_from_to(list, 0, size, elt);
}

size_t gl_list_indexof_from(gl_list_t list, size_t start_index, const void * elt)
{
	size_t size = ((const struct gl_list_impl_base *)list)->vtable->size(list);
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->indexof_from_to(list, start_index, size, elt);
}

size_t gl_list_indexof_from_to(gl_list_t list, size_t start_index, size_t end_index, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->indexof_from_to(list, start_index, end_index, elt);
}

gl_list_node_t gl_list_nx_add_first(gl_list_t list, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->nx_add_first(list, elt);
}

gl_list_node_t gl_list_nx_add_last(gl_list_t list, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->nx_add_last(list, elt);
}

gl_list_node_t gl_list_nx_add_before(gl_list_t list, gl_list_node_t node, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->nx_add_before(list, node, elt);
}

gl_list_node_t gl_list_nx_add_after(gl_list_t list, gl_list_node_t node, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->nx_add_after(list, node, elt);
}

gl_list_node_t gl_list_nx_add_at(gl_list_t list, size_t position, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->nx_add_at(list, position, elt);
}

bool gl_list_remove_node(gl_list_t list, gl_list_node_t node)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->remove_node(list, node);
}

bool gl_list_remove_at(gl_list_t list, size_t position)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->remove_at(list, position);
}

bool gl_list_remove(gl_list_t list, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->remove_elt(list, elt);
}

void gl_list_free(gl_list_t list)
{
	((const struct gl_list_impl_base *)list)->vtable->list_free(list);
}

gl_list_iterator_t gl_list_iterator(gl_list_t list)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->iterator(list);
}

gl_list_iterator_t gl_list_iterator_from_to(gl_list_t list, size_t start_index, size_t end_index)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->iterator_from_to(list, start_index, end_index);
}

bool gl_list_iterator_next(gl_list_iterator_t * iterator,
    const void ** eltp, gl_list_node_t * nodep)
{
	return iterator->vtable->iterator_next(iterator, eltp, nodep);
}

void gl_list_iterator_free(gl_list_iterator_t * iterator)
{
	iterator->vtable->iterator_free(iterator);
}

gl_list_node_t gl_sortedlist_search(gl_list_t list, gl_listelement_compar_fn compar, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->sortedlist_search(list, compar, elt);
}

gl_list_node_t gl_sortedlist_search_from_to(gl_list_t list,
    gl_listelement_compar_fn compar,
    size_t start_index,
    size_t end_index,
    const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->sortedlist_search_from_to(list, compar, start_index, end_index,
		   elt);
}

size_t gl_sortedlist_indexof(gl_list_t list, gl_listelement_compar_fn compar, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->sortedlist_indexof(list, compar, elt);
}

size_t gl_sortedlist_indexof_from_to(gl_list_t list, gl_listelement_compar_fn compar, size_t start_index, size_t end_index,
    const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->sortedlist_indexof_from_to(list, compar, start_index, end_index,
		   elt);
}

gl_list_node_t gl_sortedlist_nx_add(gl_list_t list, gl_listelement_compar_fn compar, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->sortedlist_nx_add(list, compar, elt);
}

bool gl_sortedlist_remove(gl_list_t list, gl_listelement_compar_fn compar, const void * elt)
{
	return ((const struct gl_list_impl_base *)list)->vtable
	       ->sortedlist_remove(list, compar, elt);
}

#endif
