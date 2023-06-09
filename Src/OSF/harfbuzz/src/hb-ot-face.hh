/*
 * Copyright © 2007,2008,2009  Red Hat, Inc.
 * Copyright © 2012,2013  Google, Inc.
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
#ifndef HB_OT_FACE_HH
#define HB_OT_FACE_HH

#include "hb.hh"
#include "hb-machinery.hh"

/*
 * hb_ot_face_t
 */

/* Declare tables. */
#define HB_OT_TABLE(Namespace, Type) namespace Namespace { struct Type; }
#define HB_OT_ACCELERATOR(Namespace, Type) HB_OT_TABLE(Namespace, Type ## _accelerator_t)
#include "hb-ot-face-table-list.hh"
#undef HB_OT_ACCELERATOR
#undef HB_OT_TABLE

struct hb_ot_face_t {
	HB_INTERNAL void init0(hb_face_t * face);
	HB_INTERNAL void fini();

#define HB_OT_TABLE_ORDER(Namespace, Type) \
	HB_PASTE(ORDER_, HB_PASTE(Namespace, HB_PASTE(_, Type)))
	enum order_t {
		ORDER_ZERO,
#define HB_OT_TABLE(Namespace, Type) HB_OT_TABLE_ORDER(Namespace, Type),
#include "hb-ot-face-table-list.hh"
#undef HB_OT_TABLE
	};

	hb_face_t * face; /* MUST be JUST before the lazy loaders. */
#define HB_OT_TABLE(Namespace, Type) \
	hb_table_lazy_loader_t<Namespace::Type, HB_OT_TABLE_ORDER(Namespace, Type)> Type;
#define HB_OT_ACCELERATOR(Namespace, Type) \
	hb_face_lazy_loader_t<Namespace::Type ## _accelerator_t, HB_OT_TABLE_ORDER(Namespace, Type)> Type;
#include "hb-ot-face-table-list.hh"
#undef HB_OT_ACCELERATOR
#undef HB_OT_TABLE
};

#endif /* HB_OT_FACE_HH */
