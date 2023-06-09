/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2010, 2017 Oracle and/or its affiliates.  All rights reserved.
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
#include "dbinc/db_page.h"
#include "dbinc/db_swap.h"
#include "dbinc/heap.h"
/*
 * __heap_pgin --
 *	Convert host-specific page layout from the host-independent format
 *	stored on disk.
 *
 * PUBLIC: int __heap_pgin __P((DB *, db_pgno_t, void *, DBT *));
 */
int __heap_pgin(DB *dbp, db_pgno_t pg, void * pp, DBT * cookie)
{
	PAGE * h;
	DB_PGINFO * pginfo = (DB_PGINFO*)cookie->data;
	if(!F_ISSET(pginfo, DB_AM_SWAP))
		return 0;
	h = (PAGE *)pp;
	return (TYPE(h) == P_HEAPMETA ? __heap_mswap(dbp->env, (PAGE *)pp) : __db_byteswap(dbp, pg, (PAGE *)pp, pginfo->db_pagesize, 1));
}

/*
 * __heap_pgout --
 *	Convert host-specific page layout from the host-independent format
 *	stored on disk.
 *
 * PUBLIC: int __heap_pgout __P((DB *, db_pgno_t, void *, DBT *));
 */
int __heap_pgout(DB *dbp, db_pgno_t pg, void * pp, DBT * cookie)
{
	PAGE * h;
	DB_PGINFO * pginfo = (DB_PGINFO*)cookie->data;
	if(!F_ISSET(pginfo, DB_AM_SWAP))
		return 0;
	h = (PAGE *)pp;
	return (TYPE(h) == P_HEAPMETA ?  __heap_mswap(dbp->env, (PAGE *)pp) : __db_byteswap(dbp, pg, (PAGE *)pp, pginfo->db_pagesize, 0));
}
/*
 * __heap_mswap --
 *	Swap the bytes on the heap metadata page.
 *
 * PUBLIC: int __heap_mswap __P((ENV *, PAGE *));
 */
int __heap_mswap(ENV *env, PAGE * pg)
{
	uint8 * p;
	COMPQUIET(env, NULL);
	__db_metaswap(pg);
	p = (uint8*)pg + sizeof(DBMETA);
	SWAP32(p);              /* curregion */
	SWAP32(p);              /* nregions */
	SWAP32(p);              /* gbytes */
	SWAP32(p);              /* bytes */
	SWAP32(p);              /* region_size */
	SWAP32(p);              /* threshold */
	SWAP32(p);              /* file id lo */
	SWAP32(p);              /* file id hi */
	p += 89 * sizeof(uint32); /* unused */
	SWAP32(p);              /* crypto_magic */
	return 0;
}
