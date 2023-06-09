/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2001, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
#include "dbinc/blob.h"
#include "dbinc/db_page.h"
#include "dbinc/fop.h"
#include "dbinc/db_am.h"
#include "dbinc/mp.h"
#include "dbinc/txn.h"

typedef enum {
	DB_APP53_NONE = 0,                /* No type (region). */
	DB_APP53_DATA,                  /* Data file. */
	DB_APP53_LOG,                   /* Log file. */
	DB_APP53_META,                  /* Persistent metadata file. */
	DB_APP53_RECOVER,               /* We are in recovery. */
	DB_APP53_TMP                    /* Temporary file. */
} APPNAME53;

static APPNAME __fop_convert_appname __P((APPNAME53));
static int __fop_create_recover_int __P((ENV *, char *, db_recops, int));
static int __fop_rename_recover_int __P((ENV *, DBT *, DB_LSN *, db_recops, void *, int));
static int __fop_rename_60_recover_int __P((ENV *, DBT *, DB_LSN *, db_recops, void *, int));
static int __fop_rename_42_recover_int __P((ENV *, DBT *, DB_LSN *, db_recops, void *, int));
static int __fop_write_file_recover_int __P((ENV *, db_recops,
    APPNAME, uint32, DBT *, DBT *, DBT *, DBT *, off_t, DB_TXN *));

/*
 * The APPNAME enumermation was changed in 6.0 to include DB_APP_BLOB.  APPNAME
 * is used by the log records __fop_create, __fop_write, and __fop_rename.
 * __fop_write_file also includes an APPNAME field, but that record was created
 * in 6.0.
 */
static APPNAME __fop_convert_appname(APPNAME53 appname)
{
	switch(appname) {
		case DB_APP53_NONE: return (DB_APP_NONE);
		case DB_APP53_DATA: return (DB_APP_DATA);
		case DB_APP53_LOG: return (DB_APP_LOG);
		case DB_APP53_META: return (DB_APP_META);
		case DB_APP53_RECOVER: return (DB_APP_RECOVER);
		case DB_APP53_TMP: return (DB_APP_TMP);
	}
	return (DB_APP_NONE);
}

/*
 * The transactional guarantees Berkeley DB provides for file
 * system level operations (database physical file create, delete,
 * rename) are based on our understanding of current file system
 * semantics; a system that does not provide these semantics and
 * guarantees could be in danger.
 *
 * First, as in standard database changes, fsync and fdatasync must
 * work: when applied to the log file, the records written into the
 * log must be transferred to stable storage.
 *
 * Second, it must not be possible for the log file to be removed
 * without previous file system level operations being flushed to
 * stable storage.  Berkeley DB applications write log records
 * describing file system operations into the log, then perform the
 * file system operation, then commit the enclosing transaction
 * (which flushes the log file to stable storage).  Subsequently,
 * a database environment checkpoint may make it possible for the
 * application to remove the log file containing the record of the
 * file system operation.  DB's transactional guarantees for file
 * system operations require the log file removal not succeed until
 * all previous filesystem operations have been flushed to stable
 * storage.  In other words, the flush of the log file, or the
 * removal of the log file, must block until all previous
 * filesystem operations have been flushed to stable storage.  This
 * semantic is not, as far as we know, required by any existing
 * standards document, but we have never seen a filesystem where
 * it does not apply.
 */
static int __fop_create_recover_int(ENV *env, char * real_name, db_recops op, int mode)
{
	DB_FH * fhp;
	DBMETA * meta;
	uint8 mbuf[DBMETASIZE];
	int ret;
	char * path;
#ifdef  HAVE_REPLICATION
	DELAYED_BLOB_LIST * dbl;
	int view_partial;
	dbl = NULL;
#endif
	meta = (DBMETA*)mbuf;
	ret = 0;
	if(DB_UNDO(op)) {
		/*
		 * If the file was opened in mpool, we must mark it as
		 * dead via nameop which will also unlink the file.
		 */
		if(__os_open(env, real_name, 0, 0, 0, &fhp) == 0) {
			if(__fop_read_meta(env,
			    real_name, mbuf, DBMETASIZE, fhp, 1, NULL) == 0 &&
			    __db_chk_meta(env, NULL, meta, DB_CHK_META) == 0) {
				if((ret = __memp_nameop(env,
				    meta->uid, NULL, real_name, NULL, 0)) != 0)
					goto out;
			}
			else {
				(void)__os_closehandle(env, fhp);
				goto do_unlink;
			}
			(void)__os_closehandle(env, fhp);
		}
		else
do_unlink:              (void)__os_unlink(env, real_name, 0);
	}
	else if(DB_REDO(op)) {
		path = real_name;

#ifdef  HAVE_REPLICATION
		/*
		 * Prevent replication of blob files if their owning database
		 * is not replicated.
		 */
		if(IS_VIEW_SITE(env) && IS_BLOB_FILE(path)) {
			if((ret = __rep_call_partial(env,
			    path, &view_partial, 0, &dbl)) != 0)
				goto out;
			DB_ASSERT(env, dbl == NULL);
			if(view_partial == 0)
				goto out;
		}
#endif
		/* Blob directories might not exist yet. */
		if(__os_exists(env, real_name, NULL) != 0 &&
		    (ret = __db_mkpath(env, path)) != 0)
			goto out;

		if((ret = __os_open(env, real_name,
		    0, DB_OSO_CREATE, mode, &fhp)) == 0)
			(void)__os_closehandle(env, fhp);
		else
			goto out;
	}
out:    return ret;
}

/*
 * __fop_create_recover --
 *	Recovery function for create.
 *
 * PUBLIC: int __fop_create_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_create_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_create_args * argp;
	int ret;
	const char * dirname;
	char * real_name = NULL;
	REC_PRINT(__fop_create_print);
	REC_NOOP_INTRO(__fop_create_read);
	if(argp->dirname.size == 0)
		dirname = NULL;
	else
		dirname = (const char*)argp->dirname.data;
	if((ret = __db_appname(env, (APPNAME)argp->appname == DB_APP_DATA ? DB_APP_RECOVER : (APPNAME)argp->appname, (const char*)argp->name.data, &dirname, &real_name)) != 0)
		goto out;
	if((ret = __fop_create_recover_int(env, real_name, op, (int)argp->mode)) != 0)
		goto out;
	*lsnp = argp->prev_lsn;
out:    
	if(real_name != NULL)
		__os_free(env, real_name);
	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}
/*
 * __fop_create_60_recover --
 *	Recovery function for create.
 *
 * PUBLIC: int __fop_create_60_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_create_60_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_create_60_args * argp;
	APPNAME appname;
	int ret;
	char * real_name;
	const char * dirname;
	real_name = NULL;
	REC_PRINT(__fop_create_60_print);
	REC_NOOP_INTRO(__fop_create_60_read);
	if(argp->dirname.size == 0)
		dirname = NULL;
	else
		dirname = (const char*)argp->dirname.data;
	appname = __fop_convert_appname((APPNAME53)argp->appname);
	if((ret = __db_appname(env, appname == DB_APP_DATA ? DB_APP_RECOVER : appname,
	    (const char*)argp->name.data, &dirname, &real_name)) != 0)
		goto out;
	if((ret = __fop_create_recover_int(env, real_name, op, (int)argp->mode)) != 0)
		goto out;
	*lsnp = argp->prev_lsn;
out:    
	if(real_name != NULL)
		__os_free(env, real_name);
	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}

/*
 * __fop_create_42_recover --
 *	Recovery function for create.
 *
 * PUBLIC: int __fop_create_42_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_create_42_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_create_args * argp;
	DB_FH * fhp;
	DBMETA * meta;
	uint8 mbuf[DBMETASIZE];
	APPNAME appname;
	int ret;
	char * real_name;

	real_name = NULL;
	REC_PRINT(__fop_create_print);
	REC_NOOP_INTRO(__fop_create_read);
	meta = (DBMETA*)mbuf;
	appname = __fop_convert_appname((APPNAME53)argp->appname);

	if((ret = __db_appname(env, appname,
	    (const char*)argp->name.data, NULL, &real_name)) != 0)
		goto out;

	if(DB_UNDO(op)) {
		/*
		 * If the file was opened in mpool, we must mark it as
		 * dead via nameop which will also unlink the file.
		 */
		if(__os_open(env, real_name, 0, 0, 0, &fhp) == 0) {
			if(__fop_read_meta(env,
			    real_name, mbuf, DBMETASIZE, fhp, 1, NULL) == 0 &&
			    __db_chk_meta(env, NULL, meta, DB_CHK_META) == 0) {
				if((ret = __memp_nameop(env,
				    meta->uid, NULL, real_name, NULL, 0)) != 0)
					goto out;
			}
			else
				goto do_unlink;
			(void)__os_closehandle(env, fhp);
		}
		else
do_unlink:              (void)__os_unlink(env, real_name, 0);
	}
	else if(DB_REDO(op)) {
		if((ret = __os_open(env, real_name, 0,
		    DB_OSO_CREATE, (int)argp->mode, &fhp)) == 0)
			(void)__os_closehandle(env, fhp);
		else
			goto out;
	}

	*lsnp = argp->prev_lsn;

out:    if(real_name != NULL)
		__os_free(env, real_name);

	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}

/*
 * __fop_remove_recover --
 *	Recovery function for remove.
 *
 * PUBLIC: int __fop_remove_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_remove_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_remove_args * argp;
	int ret;
	char * real_name = NULL;
	REC_PRINT(__fop_remove_print);
	REC_NOOP_INTRO(__fop_remove_read);
	if((ret = __db_appname(env, (APPNAME)argp->appname, (const char*)argp->name.data, NULL, &real_name)) != 0)
		goto out;
	/* Its ok if the file is not there. */
	if(DB_REDO(op))
		(void)__memp_nameop(env, (uint8*)argp->fid.data, NULL, real_name, NULL, 0);
	*lsnp = argp->prev_lsn;
out:    
	if(real_name != NULL)
		__os_free(env, real_name);
	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}

/*
 * __fop_remove_60_recover --
 *	Recovery function for remove.
 *
 * PUBLIC: int __fop_remove_60_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_remove_60_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_remove_60_args * argp;
	APPNAME appname;
	int ret;
	char * real_name = NULL;
	REC_PRINT(__fop_remove_60_print);
	REC_NOOP_INTRO(__fop_remove_60_read);
	appname = __fop_convert_appname((APPNAME53)argp->appname);
	if((ret = __db_appname(env, appname, (const char*)argp->name.data, NULL, &real_name)) != 0)
		goto out;
	/* Its ok if the file is not there. */
	if(DB_REDO(op))
		(void)__memp_nameop(env, (uint8*)argp->fid.data, NULL, real_name, NULL, 0);
	*lsnp = argp->prev_lsn;
out:    
	if(real_name != NULL)
		__os_free(env, real_name);
	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}

/*
 * __fop_write_recover --
 *	Recovery function for writechunk.
 *
 * PUBLIC: int __fop_write_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_write_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_write_args * argp;
	int ret;
#ifndef HAVE_64BIT_TYPES
	COMPQUIET(dbtp, NULL);
	COMPQUIET(lsnp, NULL);
	COMPQUIET(op, 0);
	COMPQUIET(info, NULL);
	__db_errx(env, DB_STR("0243", "External files require 64 integer compiler support."));
	return (DB_OPNOTSUP);
#endif
	REC_PRINT(__fop_write_print);
	REC_NOOP_INTRO(__fop_write_read);
	ret = 0;
	if(DB_UNDO(op))
		DB_ASSERT(env, argp->flag != 0);
	else if(DB_REDO(op))
		ret = __fop_write(env, argp->txnp, (const char *)argp->name.data, argp->dirname.size == 0 ? NULL : (const char *)argp->dirname.data,
			(APPNAME)argp->appname == DB_APP_DATA ? DB_APP_RECOVER : (APPNAME)argp->appname,
			NULL, argp->pgsize, argp->pageno, argp->offset, argp->page.data, argp->page.size, argp->flag, 0);
	if(ret == 0)
		*lsnp = argp->prev_lsn;
	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}

/*
 * __fop_write_60_recover --
 *	Recovery function for writechunk.
 *
 * PUBLIC: int __fop_write_60_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_write_60_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_write_60_args * argp;
	APPNAME appname;
	int ret;
	REC_PRINT(__fop_write_60_print);
	REC_NOOP_INTRO(__fop_write_60_read);
	ret = 0;
	if(DB_UNDO(op))
		DB_ASSERT(env, argp->flag != 0);
	else if(DB_REDO(op)) {
		appname = __fop_convert_appname((APPNAME53)argp->appname);
		ret = __fop_write(env, argp->txnp, (const char *)argp->name.data, argp->dirname.size == 0 ? NULL : (const char *)argp->dirname.data,
			appname == DB_APP_DATA ? DB_APP_RECOVER : appname, NULL, argp->pgsize, argp->pageno, argp->offset, argp->page.data, argp->page.size, argp->flag, 0);
	}
	if(ret == 0)
		*lsnp = argp->prev_lsn;
	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}
/*
 * __fop_write_42_recover --
 *	Recovery function for writechunk.
 *
 * PUBLIC: int __fop_write_42_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_write_42_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_write_args * argp;
	APPNAME appname;
	int ret;
	REC_PRINT(__fop_write_print);
	REC_NOOP_INTRO(__fop_write_read);
	ret = 0;
	if(DB_UNDO(op))
		DB_ASSERT(env, argp->flag != 0);
	else if(DB_REDO(op)) {
		appname = __fop_convert_appname((APPNAME53)argp->appname);
		ret = __fop_write(env, argp->txnp, (const char *)argp->name.data, NULL, appname,
			NULL, argp->pgsize, argp->pageno, argp->offset, argp->page.data, argp->page.size, argp->flag, 0);
	}
	if(ret == 0)
		*lsnp = argp->prev_lsn;
	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}

static int __fop_write_file_recover_int(ENV *env, db_recops op, APPNAME appname, uint32 flag, DBT * dirname, DBT * name, DBT * new_data, DBT * old_data, off_t offset, DB_TXN * txn)
{
	DB_FH * fhp = 0;
	int ret = 0;
	size_t nbytes;
	char * path = 0;
	if(DB_UNDO(op)) {
		if(flag & DB_FOP_CREATE) {
			/*
			 * File was created in this transaction. Do nothing,
			 * destroying the file will undo the write.
			 */
		}
		else {
			if((ret = __db_appname(env, appname == DB_APP_DATA ? DB_APP_RECOVER : appname, (const char *)name->data, NULL, &path)) != 0)
				goto end;
			if(__os_open(env, path, 0, 0, DB_MODE_600, &fhp) != 0)
				goto end;
			if(flag & DB_FOP_APPEND) {
				/*
				 * Appended to the end of the file, undo by
				 * truncating the file.
				 */
				(void)__os_truncate(env, fhp, 0, 0, offset);
			}
			else {
				/*
				 * Data overwritten in the middle of the file,
				 * undo by writing back in the old data.
				 */

				/* Seek to offset. */
				if((__os_seek(env, fhp, 0, 0, offset)) != 0)
					goto end;

				/* Now do the write. */
				ret = __os_write(env, fhp,
					old_data->data, old_data->size, &nbytes);
			}
		}
	}
	else if(DB_REDO(op)) {
		/*
		 * Not all operations log enough data to be redone.  Since
		 * files are flushed before the transaction commit this is
		 * not an issue, unless we are on an HA client or initializing
		 * from a backup.
		 */
		if(flag & DB_FOP_REDO) {
			ret = __fop_write_file(env, txn, (const char *)name->data, dirname->size == 0 ? NULL : (const char *)dirname->data,
				appname == DB_APP_DATA ? DB_APP_RECOVER : appname, NULL, offset, new_data->data, new_data->size, 0);
			/*
			 * It is possible the file was deleted later in
			 * the logs, so ignore any ENOENT errors.
			 */
			if(ret == ENOENT)
				ret = 0;
		}
		else {
			/* DB_ASSERT(env, !IS_REP_CLIENT(env)); */
		}
	}

end:    if(path != NULL)
		__os_free(env, path);
	if(fhp != NULL)
		(void)__os_closehandle(env, fhp);
	return ret;
}

/*
 * __fop_write_file_recover --
 *	Recovery function for writing to a blob file.  Files are flushed before
 *	the transaction is committed, so often the file operations do not need
 *	to be redone or undone.  However, since no lsn is stored in the file,
 *	we always try to redo or undo the operation, since it will not change
 *	the final state of the file if the operation is not needed.  This also
 *	means that this function has to be very tolerant of errors, such as
 *	trying to open a file that was deleted, or truncate a file that is
 *	already short.
 *
 * PUBLIC: int __fop_write_file_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_write_file_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_write_file_args * argp;
	int ret;
#ifndef HAVE_64BIT_TYPES
	COMPQUIET(dbtp, NULL);
	COMPQUIET(lsnp, NULL);
	COMPQUIET(op, 0);
	COMPQUIET(info, NULL);
	__db_errx(env, DB_STR("0244", "External files require 64 integer compiler support."));
	return (DB_OPNOTSUP);
#endif
	REC_PRINT(__fop_write_file_print);
	REC_NOOP_INTRO(__fop_write_file_read);
	ret = __fop_write_file_recover_int(env, op, (APPNAME)argp->appname, argp->flag, &argp->dirname, &argp->name,
		&argp->new_data, &argp->old_data, (off_t)argp->offset, argp->txnp);
	if(ret == 0)
		*lsnp = argp->prev_lsn;
	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}

/*
 * __fop_write_file_60_recover --
 *
 * PUBLIC: int __fop_write_file_60_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_write_file_60_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_write_file_60_args * argp;
	off_t offset;
	int ret;
	REC_PRINT(__fop_write_file_60_print);
	REC_NOOP_INTRO(__fop_write_file_60_read);
	/* The offset is stored as two u_in32_t values. */
	GET_LO_HI(env, argp->offset_lo, argp->offset_hi, offset, ret);
	if(ret != 0)
		goto end;
	ret = __fop_write_file_recover_int(env, op, (APPNAME)argp->appname, argp->flag, &argp->dirname, &argp->name, &argp->new_data, &argp->old_data, offset, argp->txnp);
end:    
	if(ret == 0)
		*lsnp = argp->prev_lsn;
	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}
/*
 * __fop_rename_recover --
 *	Recovery functions for rename.  There are two variants that
 * both use the same utility function.  Had we known about this on day
 * one, we would have simply added a parameter.  However, since we need
 * to retain old records for backward compatibility (online-upgrade)
 * wrapping the two seems like the right solution.
 *
 * PUBLIC: int __fop_rename_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 *
 * PUBLIC: int __fop_rename_noundo_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_rename_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	return (__fop_rename_recover_int(env, dbtp, lsnp, op, info, 1));
}

int __fop_rename_noundo_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	return (__fop_rename_recover_int(env, dbtp, lsnp, op, info, 0));
}

static int __fop_rename_recover_int(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info, int undo)
{
	__fop_rename_args * argp;
	APPNAME appname;
	DB_FH * fhp;
	DBMETA * meta;
	uint8 * fileid, mbuf[DBMETASIZE];
	int ret;
	char * real_new, * real_old, * src;
	const char * dirname;

	fhp = NULL;
	meta = (DBMETA*)&mbuf[0];
	ret = 0;
	real_new = real_old = NULL;

	REC_PRINT(__fop_rename_print);
	REC_NOOP_INTRO(__fop_rename_read);
	fileid = (uint8 *)argp->fileid.data;

	if(argp->dirname.size == 0)
		dirname = NULL;
	else
		dirname = (const char*)argp->dirname.data;

	if((APPNAME)argp->appname == DB_APP_DATA)
		appname = DB_APP_RECOVER;
	else
		appname = (APPNAME)argp->appname;

	if((ret = __db_appname(env, appname, (const char*)argp->newname.data,
	    &dirname, &real_new)) != 0)
		goto out;
	if((ret = __db_appname(env, appname, (const char*)argp->oldname.data,
	    &dirname, &real_old)) != 0)
		goto out;

	/*
	 * Verify that we are manipulating the correct file.  We should always
	 * be OK on an ABORT or an APPLY, but during recovery, we have to
	 * check.
	 */
	if(op != DB_TXN_ABORT && op != DB_TXN_APPLY) {
		src = DB_UNDO(op) ? real_new : real_old;
		/*
		 * Interpret any error as meaning that the file either doesn't
		 * exist, doesn't have a meta-data page, or is in some other
		 * way, shape or form, incorrect, so that we should not restore
		 * it.
		 */
		if(__os_open(env, src, 0, 0, 0, &fhp) != 0)
			goto done;
		if(__fop_read_meta(env,
		    src, mbuf, DBMETASIZE, fhp, 1, NULL) != 0)
			goto done;
		if(__db_chk_meta(env, NULL, meta, DB_CHK_META) != 0)
			goto done;
		if(memcmp(argp->fileid.data, meta->uid, DB_FILE_ID_LEN) != 0)
			goto done;
		(void)__os_closehandle(env, fhp);
		fhp = NULL;
		if(DB_REDO(op)) {
			/*
			 * Check to see if the target file exists.  If it
			 * does and it does not have the proper id then
			 * it is a later version.  We just remove the source
			 * file since the state of the world is beyond this
			 * point.
			 */
			if(__os_open(env, real_new, 0, 0, 0, &fhp) == 0 &&
			    __fop_read_meta(env, src, mbuf,
			    DBMETASIZE, fhp, 1, NULL) == 0 &&
			    __db_chk_meta(env, NULL, meta, DB_CHK_META) == 0 &&
			    memcmp(argp->fileid.data,
			    meta->uid, DB_FILE_ID_LEN) != 0) {
				(void)__memp_nameop(env,
				    fileid, NULL, real_old, NULL, 0);
				goto done;
			}
		}
	}

	if(undo && DB_UNDO(op))
		(void)__memp_nameop(env, fileid,
		    (const char*)argp->oldname.data, real_new, real_old, 0);
	if(DB_REDO(op))
		(void)__memp_nameop(env, fileid,
		    (const char*)argp->newname.data, real_old, real_new, 0);

done:   *lsnp = argp->prev_lsn;
out:    if(real_new != NULL)
		__os_free(env, real_new);
	if(real_old != NULL)
		__os_free(env, real_old);
	if(fhp != NULL)
		(void)__os_closehandle(env, fhp);

	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}

/*
 * __fop_rename_60_recover --
 *
 * PUBLIC: int __fop_rename_60_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 *
 * PUBLIC: int __fop_rename_noundo_60_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */

int __fop_rename_60_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	return (__fop_rename_60_recover_int(env, dbtp, lsnp, op, info, 1));
}

int __fop_rename_noundo_60_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	return (__fop_rename_60_recover_int(env, dbtp, lsnp, op, info, 0));
}

static int __fop_rename_60_recover_int(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info, int undo)
{
	__fop_rename_60_args * argp;
	APPNAME appname;
	DB_FH * fhp;
	DBMETA * meta;
	uint8 * fileid, mbuf[DBMETASIZE];
	int ret;
	char * real_new, * real_old, * src;
	const char * dirname;

	fhp = NULL;
	meta = (DBMETA*)&mbuf[0];
	ret = 0;
	real_new = real_old = NULL;

	REC_PRINT(__fop_rename_60_print);
	REC_NOOP_INTRO(__fop_rename_60_read);
	fileid = (uint8 *)argp->fileid.data;

	if(argp->dirname.size == 0)
		dirname = NULL;
	else
		dirname = (const char*)argp->dirname.data;

	appname = __fop_convert_appname((APPNAME53)argp->appname);
	if(appname == DB_APP_DATA)
		appname = DB_APP_RECOVER;

	if((ret = __db_appname(env, appname, (const char*)argp->newname.data,
	    &dirname, &real_new)) != 0)
		goto out;
	if((ret = __db_appname(env, appname, (const char*)argp->oldname.data,
	    &dirname, &real_old)) != 0)
		goto out;

	/*
	 * Verify that we are manipulating the correct file.  We should always
	 * be OK on an ABORT or an APPLY, but during recovery, we have to
	 * check.
	 */
	if(op != DB_TXN_ABORT && op != DB_TXN_APPLY) {
		src = DB_UNDO(op) ? real_new : real_old;
		/*
		 * Interpret any error as meaning that the file either doesn't
		 * exist, doesn't have a meta-data page, or is in some other
		 * way, shape or form, incorrect, so that we should not restore
		 * it.
		 */
		if(__os_open(env, src, 0, 0, 0, &fhp) != 0)
			goto done;
		if(__fop_read_meta(env,
		    src, mbuf, DBMETASIZE, fhp, 1, NULL) != 0)
			goto done;
		if(__db_chk_meta(env, NULL, meta, DB_CHK_META) != 0)
			goto done;
		if(memcmp(argp->fileid.data, meta->uid, DB_FILE_ID_LEN) != 0)
			goto done;
		(void)__os_closehandle(env, fhp);
		fhp = NULL;
		if(DB_REDO(op)) {
			/*
			 * Check to see if the target file exists.  If it
			 * does and it does not have the proper id then
			 * it is a later version.  We just remove the source
			 * file since the state of the world is beyond this
			 * point.
			 */
			if(__os_open(env, real_new, 0, 0, 0, &fhp) == 0 &&
			    __fop_read_meta(env, src, mbuf,
			    DBMETASIZE, fhp, 1, NULL) == 0 &&
			    __db_chk_meta(env, NULL, meta, DB_CHK_META) == 0 &&
			    memcmp(argp->fileid.data,
			    meta->uid, DB_FILE_ID_LEN) != 0) {
				(void)__memp_nameop(env,
				    fileid, NULL, real_old, NULL, 0);
				goto done;
			}
		}
	}

	if(undo && DB_UNDO(op))
		(void)__memp_nameop(env, fileid,
		    (const char*)argp->oldname.data, real_new, real_old, 0);
	if(DB_REDO(op))
		(void)__memp_nameop(env, fileid,
		    (const char*)argp->newname.data, real_old, real_new, 0);

done:   *lsnp = argp->prev_lsn;
out:    if(real_new != NULL)
		__os_free(env, real_new);
	if(real_old != NULL)
		__os_free(env, real_old);
	if(fhp != NULL)
		(void)__os_closehandle(env, fhp);

	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}
/*
 * __fop_rename_42_recover --
 *	Recovery functions for rename.  There are two variants that
 * both use the same utility function.  Had we known about this on day
 * one, we would have simply added a parameter.  However, since we need
 * to retain old records for backward compatibility (online-upgrade)
 * wrapping the two seems like the right solution.
 *
 * PUBLIC: int __fop_rename_42_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 *
 * PUBLIC: int __fop_rename_noundo_46_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_rename_42_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	return (__fop_rename_42_recover_int(env, dbtp, lsnp, op, info, 1));
}

int __fop_rename_noundo_46_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	return (__fop_rename_42_recover_int(env, dbtp, lsnp, op, info, 0));
}

static int __fop_rename_42_recover_int(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info, int undo)
{
	__fop_rename_args * argp;
	DB_FH * fhp;
	DBMETA * meta;
	uint8 * fileid, mbuf[DBMETASIZE];
	APPNAME appname;
	int ret;
	char * real_new, * real_old, * src;
	fhp = NULL;
	meta = (DBMETA*)&mbuf[0];
	ret = 0;
	real_new = real_old = NULL;

	REC_PRINT(__fop_rename_print);
	REC_NOOP_INTRO(__fop_rename_read);
	fileid = (uint8 *)argp->fileid.data;

	appname = __fop_convert_appname((APPNAME53)argp->appname);
	if((ret = __db_appname(env, appname,
	    (const char*)argp->newname.data, NULL, &real_new)) != 0)
		goto out;
	if((ret = __db_appname(env, appname,
	    (const char*)argp->oldname.data, NULL, &real_old)) != 0)
		goto out;

	/*
	 * Verify that we are manipulating the correct file.  We should always
	 * be OK on an ABORT or an APPLY, but during recovery, we have to
	 * check.
	 */
	if(op != DB_TXN_ABORT && op != DB_TXN_APPLY) {
		src = DB_UNDO(op) ? real_new : real_old;
		/*
		 * Interpret any error as meaning that the file either doesn't
		 * exist, doesn't have a meta-data page, or is in some other
		 * way, shape or form, incorrect, so that we should not restore
		 * it.
		 */
		if(__os_open(env, src, 0, 0, 0, &fhp) != 0)
			goto done;
		if(__fop_read_meta(env,
		    src, mbuf, DBMETASIZE, fhp, 1, NULL) != 0)
			goto done;
		if(__db_chk_meta(env, NULL, meta, DB_CHK_META) != 0)
			goto done;
		if(memcmp(argp->fileid.data, meta->uid, DB_FILE_ID_LEN) != 0)
			goto done;
		(void)__os_closehandle(env, fhp);
		fhp = NULL;
		if(DB_REDO(op)) {
			/*
			 * Check to see if the target file exists.  If it
			 * does and it does not have the proper id then
			 * it is a later version.  We just remove the source
			 * file since the state of the world is beyond this
			 * point.
			 */
			if(__os_open(env, real_new, 0, 0, 0, &fhp) == 0 &&
			    __fop_read_meta(env, src, mbuf,
			    DBMETASIZE, fhp, 1, NULL) == 0 &&
			    __db_chk_meta(env, NULL, meta, DB_CHK_META) == 0 &&
			    memcmp(argp->fileid.data,
			    meta->uid, DB_FILE_ID_LEN) != 0) {
				(void)__memp_nameop(env,
				    fileid, NULL, real_old, NULL, 0);
				goto done;
			}
		}
	}

	if(undo && DB_UNDO(op))
		(void)__memp_nameop(env, fileid,
		    (const char*)argp->oldname.data, real_new, real_old, 0);
	if(DB_REDO(op))
		(void)__memp_nameop(env, fileid,
		    (const char*)argp->newname.data, real_old, real_new, 0);

done:   *lsnp = argp->prev_lsn;
out:    if(real_new != NULL)
		__os_free(env, real_new);
	if(real_old != NULL)
		__os_free(env, real_old);
	if(fhp != NULL)
		(void)__os_closehandle(env, fhp);

	COMPQUIET(info, NULL);
	REC_NOOP_CLOSE;
}

/*
 * __fop_file_remove_recover --
 *	Recovery function for file_remove.  On the REDO pass, we need to
 * make sure no one recreated the file while we weren't looking.  On an
 * undo pass must check if the file we are interested in is the one that
 * exists and then set the status of the child transaction depending on
 * what we find out.
 *
 * PUBLIC: int __fop_file_remove_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_file_remove_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_file_remove_args * argp;
	DBMETA * meta;
	DB_FH * fhp;
	size_t len;
	uint8 mbuf[DBMETASIZE];
	uint32 cstat, ret_stat;
	int is_real, is_tmp, ret;
	char * real_name;

	fhp = NULL;
	meta = (DBMETA*)&mbuf[0];
	is_real = is_tmp = 0;
	real_name = NULL;
	REC_PRINT(__fop_file_remove_print);
	REC_NOOP_INTRO(__fop_file_remove_read);

	/*
	 * This record is only interesting on the backward, forward, and
	 * apply phases.
	 */
	if(op != DB_TXN_BACKWARD_ROLL && op != DB_TXN_FORWARD_ROLL && op != DB_TXN_APPLY)
		goto done;
	if((ret = __db_appname(env, (APPNAME)argp->appname, (const char *)argp->name.data, NULL, &real_name)) != 0)
		goto out;
	/* Verify that we are manipulating the correct file.  */
	len = 0;
	if(__os_open(env, real_name, 0, 0, 0, &fhp) != 0 || (ret = __fop_read_meta(env, real_name, mbuf, DBMETASIZE, fhp, 1, &len)) != 0) {
		/*
		 * If len is non-zero, then the file exists and has something
		 * in it, but that something isn't a full meta-data page, so
		 * this is very bad.  Bail out!
		 */
		if(len != 0)
			goto out;

		/* File does not exist. */
		cstat = TXN_EXPECTED;
	}
	else {
		/*
		 * We can ignore errors here since we'll simply fail the
		 * checks below and assume this is the wrong file.
		 */
		(void)__db_chk_meta(env, NULL, meta, DB_CHK_META);
		is_real =
		    memcmp(argp->real_fid.data, meta->uid, DB_FILE_ID_LEN) == 0;
		is_tmp =
		    memcmp(argp->tmp_fid.data, meta->uid, DB_FILE_ID_LEN) == 0;

		if(!is_real && !is_tmp)
			/* File exists, but isn't what we were removing. */
			cstat = TXN_IGNORE;
		else
			/* File exists and is the one that we were removing. */
			cstat = TXN_COMMIT;
	}
	if(fhp != NULL) {
		(void)__os_closehandle(env, fhp);
		fhp = NULL;
	}

	if(DB_UNDO(op)) {
		/* On the backward pass, we leave a note for the child txn. */
		if((ret = __db_txnlist_update(env, (DB_TXNHEAD *)info, argp->child, cstat, NULL, &ret_stat, 1)) != 0)
			goto out;
	}
	else if(DB_REDO(op)) {
		/*
		 * On the forward pass, check if someone recreated the
		 * file while we weren't looking.
		 */
		if(cstat == TXN_COMMIT)
			(void)__memp_nameop(env, (uint8 *)(is_real ? argp->real_fid.data : argp->tmp_fid.data), NULL, real_name, NULL, 0);
	}
done:   
	*lsnp = argp->prev_lsn;
	ret = 0;
out:    
	if(real_name != NULL)
		__os_free(env, real_name);
	if(fhp != NULL)
		(void)__os_closehandle(env, fhp);
	REC_NOOP_CLOSE;
}
/*
 * __fop_file_remove_60_recover --
 *
 * PUBLIC: int __fop_file_remove_60_recover
 * PUBLIC:  (ENV *, DBT *, DB_LSN *, db_recops, void *);
 */
int __fop_file_remove_60_recover(ENV *env, DBT * dbtp, DB_LSN * lsnp, db_recops op, void * info)
{
	__fop_file_remove_60_args * argp;
	DBMETA * meta;
	DB_FH * fhp;
	size_t len;
	uint8 mbuf[DBMETASIZE];
	uint32 cstat, ret_stat;
	APPNAME appname;
	int is_real, is_tmp, ret;
	char * real_name;

	fhp = NULL;
	meta = (DBMETA*)&mbuf[0];
	is_real = is_tmp = 0;
	real_name = NULL;
	REC_PRINT(__fop_file_remove_60_print);
	REC_NOOP_INTRO(__fop_file_remove_60_read);

	/*
	 * This record is only interesting on the backward, forward, and
	 * apply phases.
	 */
	if(op != DB_TXN_BACKWARD_ROLL && op != DB_TXN_FORWARD_ROLL && op != DB_TXN_APPLY)
		goto done;
	appname = __fop_convert_appname((APPNAME53)argp->appname);
	if((ret = __db_appname(env, appname, (const char *)argp->name.data, NULL, &real_name)) != 0)
		goto out;

	/* Verify that we are manipulating the correct file.  */
	len = 0;
	if(__os_open(env, real_name, 0, 0, 0, &fhp) != 0 || (ret = __fop_read_meta(env, real_name,
	    mbuf, DBMETASIZE, fhp, 1, &len)) != 0) {
		/*
		 * If len is non-zero, then the file exists and has something
		 * in it, but that something isn't a full meta-data page, so
		 * this is very bad.  Bail out!
		 */
		if(len != 0)
			goto out;

		/* File does not exist. */
		cstat = TXN_EXPECTED;
	}
	else {
		/*
		 * We can ignore errors here since we'll simply fail the
		 * checks below and assume this is the wrong file.
		 */
		(void)__db_chk_meta(env, NULL, meta, DB_CHK_META);
		is_real = memcmp(argp->real_fid.data, meta->uid, DB_FILE_ID_LEN) == 0;
		is_tmp = memcmp(argp->tmp_fid.data, meta->uid, DB_FILE_ID_LEN) == 0;
		if(!is_real && !is_tmp)
			/* File exists, but isn't what we were removing. */
			cstat = TXN_IGNORE;
		else
			/* File exists and is the one that we were removing. */
			cstat = TXN_COMMIT;
	}
	if(fhp != NULL) {
		(void)__os_closehandle(env, fhp);
		fhp = NULL;
	}
	if(DB_UNDO(op)) {
		/* On the backward pass, we leave a note for the child txn. */
		if((ret = __db_txnlist_update(env, (DB_TXNHEAD *)info, argp->child, cstat, NULL, &ret_stat, 1)) != 0)
			goto out;
	}
	else if(DB_REDO(op)) {
		/*
		 * On the forward pass, check if someone recreated the
		 * file while we weren't looking.
		 */
		if(cstat == TXN_COMMIT)
			(void)__memp_nameop(env, (uint8 *)(is_real ? argp->real_fid.data : argp->tmp_fid.data), NULL, real_name, NULL, 0);
	}
done:   
	*lsnp = argp->prev_lsn;
	ret = 0;
out:    
	if(real_name != NULL)
		__os_free(env, real_name);
	if(fhp != NULL)
		(void)__os_closehandle(env, fhp);
	REC_NOOP_CLOSE;
}
