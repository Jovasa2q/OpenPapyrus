/*-
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2005, 2011 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#include "db_int.h"
#pragma hdrstop
/*
 * __lock_failchk --
 *	Check for locks held by dead threads of control and release
 *	read locks.  If any write locks were held by dead non-trasnactional
 *	lockers then we must abort and run recovery.  Otherwise we release
 *	read locks for lockers owned by dead threads.  Write locks for
 *	dead transactional lockers will be freed when we abort the transaction.
 */
int __lock_failchk(ENV * env)
{
	DB_LOCKER * lip;
	DB_LOCKREQ request;
	uint32 i;
	int ret;
	char buf[DB_THREADID_STRLEN];
	DB_ENV * dbenv = env->dbenv;
	DB_LOCKTAB * lt = env->lk_handle;
	DB_LOCKREGION * lrp = (DB_LOCKREGION *)lt->reginfo.primary;
retry:
	LOCK_LOCKERS(env, lrp);
	ret = 0;
	for(i = 0; i < lrp->locker_t_size; i++)
		SH_TAILQ_FOREACH(lip, &lt->locker_tab[i], links, __db_locker) {
			/*
			 * If the locker is transactional, we can ignore it if
			 * it has no read locks or has no locks at all.  Check
			 * the heldby list rather then nlocks since a lock may
			 * be PENDING.  __txn_failchk aborts any transactional
			 * lockers.  Non-transactional lockers progress to
			 * is_alive test.
			 */
			if((lip->id >= TXN_MINIMUM) && (SH_LIST_EMPTY(&lip->heldby) || lip->nlocks == lip->nwrites))
				continue;
			/* If the locker is still alive, it's not a problem. */
			if(dbenv->is_alive(dbenv, lip->pid, lip->tid, F_ISSET(lip, DB_LOCKER_HANDLE_LOCKER) ? DB_MUTEX_PROCESS_ONLY : 0))
				continue;
			/*
			 * We can only deal with read locks.  If a
			 * non-transactional locker holds write locks we
			 * have to assume a Berkeley DB operation was
			 * interrupted with only 1-of-N pages modified.
			 */
			if(lip->id < TXN_MINIMUM && lip->nwrites != 0) {
				ret = __db_failed(env, DB_STR_A("2052", "locker has write locks", ""), lip->pid, lip->tid);
				break;
			}
			/*
			 * Discard the locker and its read locks.
			 */
			if(!SH_LIST_EMPTY(&lip->heldby)) {
				__db_msg(env, DB_STR_A("2053", "Freeing read locks for locker %#lx: %s", "%#lx %s"), (ulong)lip->id,
					dbenv->thread_id_string(dbenv, lip->pid, lip->tid, buf));
				UNLOCK_LOCKERS(env, lrp);
				memzero(&request, sizeof(request));
				request.op = DB_LOCK_PUT_READ;
				if((ret = __lock_vec(env, lip, 0, &request, 1, NULL)) != 0)
					return ret;
			}
			else
				UNLOCK_LOCKERS(env, lrp);
			/*
			 * This locker is most likely referenced by a cursor
			 * which is owned by a dead thread.  Normally the
			 * cursor would be available for other threads
			 * but we assume the dead thread will never release
			 * it.
			 */
			if(lip->id < TXN_MINIMUM && (ret = __lock_freelocker(lt, lip)) != 0)
				return ret;
			goto retry;
		}

		UNLOCK_LOCKERS(env, lrp);
	return ret;
}
