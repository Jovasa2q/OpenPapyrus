/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 1999, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
#include "dbinc/db_page.h"
#include "dbinc/db_am.h"
#include "dbinc/lock.h"
#include "dbinc/mp.h"
#include "dbinc/qam.h"
#include "dbinc/txn.h"

static int __qam_rr __P((DB *, DB_THREAD_INFO *, DB_TXN *, const char *, const char *, const char *, qam_name_op));
static int __qam_set_extentsize(DB *, uint32);

/*
 * __qam_db_create --
 *	Queue specific initialization of the DB structure.
 *
 * PUBLIC: int __qam_db_create(DB *);
 */
int __qam_db_create(DB *dbp)
{
	QUEUE * t;
	int ret;
	/* Allocate and initialize the private queue structure. */
	if((ret = __os_calloc(dbp->env, 1, sizeof(QUEUE), &t)) != 0)
		return ret;
	dbp->q_internal = t;
	dbp->get_q_extentsize = __qam_get_extentsize;
	dbp->set_q_extentsize = __qam_set_extentsize;
	t->re_pad = ' ';
	return 0;
}
/*
 * __qam_db_close --
 *	Queue specific discard of the DB structure.
 *
 * PUBLIC: int __qam_db_close(DB *, uint32);
 */
int __qam_db_close(DB *dbp, uint32 flags)
{
	DB_MPOOLFILE * mpf;
	MPFARRAY * array;
	QUEUE * t;
	struct __qmpf * mpfp;
	uint32 i;
	int t_ret;
	int ret = 0;
	if((t = (QUEUE *)dbp->q_internal) == NULL)
		return 0;
	array = &t->array1;
again:
	mpfp = (struct __qmpf *)array->mpfarray;
	if(mpfp != NULL) {
		for(i = array->low_extent; i <= array->hi_extent; i++, mpfp++) {
			mpf = mpfp->mpf;
			mpfp->mpf = NULL;
			if(mpf != NULL && (t_ret = __memp_fclose(mpf, LF_ISSET(DB_AM_DISCARD) ? DB_MPOOL_DISCARD : 0)) != 0 && ret == 0)
				ret = t_ret;
		}
		__os_free(dbp->env, array->mpfarray);
	}
	if(t->array2.n_extent != 0) {
		array = &t->array2;
		array->n_extent = 0;
		goto again;
	}
	if(LF_ISSET(DB_AM_DISCARD) && (t_ret = __qam_nameop(dbp, NULL, NULL, QAM_NAME_DISCARD)) != 0 && ret == 0)
		ret = t_ret;
	if(t->path != NULL)
		__os_free(dbp->env, t->path);
	__os_free(dbp->env, t);
	dbp->q_internal = NULL;
	return ret;
}
/*
 * __qam_get_extentsize --
 *	The DB->q_get_extentsize method.
 *
 * PUBLIC: int __qam_get_extentsize(DB *, uint32 *);
 */
int __qam_get_extentsize(DB *dbp, uint32 * q_extentsizep)
{
	*q_extentsizep = ((QUEUE*)dbp->q_internal)->page_ext;
	return 0;
}

static int __qam_set_extentsize(DB *dbp, uint32 extentsize)
{
	DB_ILLEGAL_AFTER_OPEN(dbp, "DB->set_extentsize");
	if(extentsize < 1) {
		__db_errx(dbp->env, DB_STR("1140", "Extent size must be at least 1"));
		return EINVAL;
	}
	((QUEUE*)dbp->q_internal)->page_ext = extentsize;
	return 0;
}
/*
 * __queue_pageinfo -
 *	Given a dbp, get first/last page information about a queue.
 *
 * PUBLIC: int __queue_pageinfo __P((DB *, db_pgno_t *, db_pgno_t *,
 * PUBLIC:       int *, int, uint32));
 */
int __queue_pageinfo(DB *dbp, db_pgno_t * firstp, db_pgno_t * lastp, int * emptyp, int prpage, uint32 flags)
{
	DB_MPOOLFILE * mpf;
	DB_THREAD_INFO * ip;
	QMETA * meta;
	db_pgno_t first, i, last;
	int empty, ret, t_ret;
	mpf = dbp->mpf;
	ENV_GET_THREAD_INFO(dbp->env, ip);
	/* Find out the page number of the last page in the database. */
	i = PGNO_BASE_MD;
	if((ret = __memp_fget(mpf, &i, ip, NULL, 0, &meta)) != 0)
		return ret;
	first = QAM_RECNO_PAGE(dbp, meta->first_recno);
	last = QAM_RECNO_PAGE(dbp, meta->cur_recno == 1 ? 1 : meta->cur_recno - 1);
	empty = meta->cur_recno == meta->first_recno;
	if(firstp != NULL)
		*firstp = first;
	if(lastp != NULL)
		*lastp = last;
	if(emptyp != NULL)
		*emptyp = empty;
#ifdef HAVE_STATISTICS
	if(prpage)
		ret = __db_prpage(dbp, (PAGE*)meta, flags);
#else
	COMPQUIET(prpage, 0);
	COMPQUIET(flags, 0);
#endif
	if((t_ret = __memp_fput(mpf, ip, meta, dbp->priority)) != 0 && ret == 0)
		ret = t_ret;
	return ret;
}

#ifdef HAVE_STATISTICS
/*
 * __db_prqueue --
 *	Print out a queue
 *
 * PUBLIC: int __db_prqueue(DB *, uint32);
 */
int __db_prqueue(DB *dbp, uint32 flags)
{
	DBC * dbc;
	DB_THREAD_INFO * ip;
	PAGE * h;
	db_pgno_t first, i, last, pg_ext, stop;
	int empty, ret, t_ret;

	if((ret = __queue_pageinfo(dbp, &first, &last, &empty, 1, flags)) != 0)
		return ret;

	if(empty || ret != 0)
		return ret;

	ENV_GET_THREAD_INFO(dbp->env, ip);
	if((ret = __db_cursor(dbp, ip, NULL, &dbc, 0)) != 0)
		return ret;
	i = first;
	if(first > last)
		stop = QAM_RECNO_PAGE(dbp, UINT32_MAX);
	else
		stop = last;

	/* Dump each page. */
	pg_ext = ((QUEUE*)dbp->q_internal)->page_ext;
begin:
	for(; i <= stop; ++i) {
		if((ret = __qam_fget(dbc, &i, 0, &h)) != 0) {
			if(pg_ext == 0) {
				if(ret == DB_PAGE_NOTFOUND && first == last)
					ret = 0;
				goto err;
			}
			if(ret == ENOENT || ret == DB_PAGE_NOTFOUND) {
				i += (pg_ext - ((i - 1) % pg_ext)) - 1;
				ret = 0;
				continue;
			}
			goto err;
		}
		(void)__db_prpage(dbp, h, flags);
		if((ret = __qam_fput(dbc, i, h, dbp->priority)) != 0)
			goto err;
	}

	if(first > last) {
		i = 1;
		stop = last;
		first = last;
		goto begin;
	}

err:
	if((t_ret = __dbc_close(dbc)) != 0 && ret == 0)
		ret = t_ret;
	return ret;
}
#endif

/*
 * __qam_remove --
 *	Remove method for a Queue.
 *
 * PUBLIC: int __qam_remove __P((DB *, DB_THREAD_INFO *, DB_TXN *,
 * PUBLIC:    const char *, const char *, uint32));
 */
int __qam_remove(DB *dbp, DB_THREAD_INFO * ip, DB_TXN * txn, const char * name, const char * subdb, uint32 flags)
{
	COMPQUIET(flags, 0);
	return (__qam_rr(dbp, ip, txn, name, subdb, NULL, QAM_NAME_REMOVE));
}
/*
 * __qam_rename --
 *	Rename method for a Queue.
 *
 * PUBLIC: int __qam_rename __P((DB *, DB_THREAD_INFO *, DB_TXN *,
 * PUBLIC:         const char *, const char *, const char *));
 */
int __qam_rename(DB *dbp, DB_THREAD_INFO * ip, DB_TXN * txn, const char * name, const char * subdb, const char * newname)
{
	return (__qam_rr(dbp, ip, txn, name, subdb, newname, QAM_NAME_RENAME));
}
/*
 * __qam_rr --
 *	Remove/Rename method for a Queue.
 */
static int __qam_rr(DB *dbp, DB_THREAD_INFO * ip, DB_TXN * txn, const char * name, const char * subdb, const char * newname, qam_name_op op)
{
	DB * tmpdbp;
	ENV * env;
	QUEUE * qp;
	int ret, t_ret;
	env = dbp->env;
	ret = 0;
	if(subdb != NULL && name != NULL) {
		__db_errx(env, DB_STR("1141", "Queue does not support multiple databases per file"));
		return EINVAL;
	}

	/*
	 * For rename/remove, there is no need to open the database
	 * via __db_open.  It is good enough to just open its memory pool
	 * which is necessary for in-mem databases.
	 */
	if(F2_ISSET(dbp, DB2_AM_MPOOL_OPENED))
		tmpdbp = dbp;
	else {
		if((ret = __db_create_internal(&tmpdbp, env, 0)) != 0)
			return ret;

		/*
		 * We need to make sure we don't self-deadlock, so give
		 * this dbp the same locker as the incoming one.
		 */
		tmpdbp->locker = dbp->locker;
		if((ret = __db_open(tmpdbp, ip, txn,
		    name, NULL, DB_QUEUE, DB_RDONLY, 0, PGNO_BASE_MD)) != 0)
			goto err;
	}

	qp = (QUEUE*)tmpdbp->q_internal;
	if(qp->page_ext != 0)
		ret = __qam_nameop(tmpdbp, txn, newname, op);

	if(!F2_ISSET(dbp, DB2_AM_MPOOL_OPENED)) {
err:
		/* We need to remove the lock event we associated with this. */
		if(txn != NULL)
			__txn_remlock(env, txn, NULL, tmpdbp->locker);

		/*
		 * Since we copied the locker ID from the dbp, we'd better not
		 * free it here.
		 */
		tmpdbp->locker = NULL;

		if((t_ret = __db_close(tmpdbp,
		    txn, DB_NOSYNC)) != 0 && ret == 0)
			ret = t_ret;
	}
	return ret;
}

/*
 * __qam_map_flags --
 *	Map queue-specific flags from public to the internal values.
 *
 * PUBLIC: void __qam_map_flags __P((DB *, uint32 *, uint32 *));
 */
void __qam_map_flags(DB *dbp, uint32 * inflagsp, uint32 * outflagsp)
{
	COMPQUIET(dbp, NULL);
	if(FLD_ISSET(*inflagsp, DB_INORDER)) {
		FLD_SET(*outflagsp, DB_AM_INORDER);
		FLD_CLR(*inflagsp, DB_INORDER);
	}
}
/*
 * __qam_set_flags --
 *	Set queue-specific flags.
 *
 * PUBLIC: int __qam_set_flags __P((DB *, uint32 *flagsp));
 */
int __qam_set_flags(DB *dbp, uint32 * flagsp)
{
	__qam_map_flags(dbp, flagsp, &dbp->flags);
	return 0;
}
