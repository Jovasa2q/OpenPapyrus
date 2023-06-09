/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2002, 2017 Oracle and/or its affiliates.  All rights reserved.
 *
 * $Id$
 */
#include "db_config.h"
#define LOAD_ACTUAL_MUTEX_CODE
#include "db_int.h"
#pragma hdrstop
#include "dbinc/atomic.h"
/*
 * This is where we load in the actual mutex declarations.
 */
#include "dbinc/mutex_int.h"

/*
 * Common code to get an event handle.  This is executed whenever a mutex
 * blocks, or when unlocking a mutex that a thread is waiting on.  We can't
 * keep these handles around, since the mutex structure is in shared memory,
 * and each process gets its own handle value.
 *
 * We pass security attributes so that the created event is accessible by all
 * users, in case a Windows service is sharing an environment with a local
 * process run as a different user.
 */
static _TCHAR hex_digits[] = _T("0123456789abcdef");

static __inline int get_handle(ENV *env, DB_MUTEX * mutexp, HANDLE * eventp)
{
	_TCHAR idbuf[] = _T("db.m00000000");
	_TCHAR * p = idbuf + 12;
	int ret = 0;
	uint32 id;
	for(id = (mutexp)->id; id != 0; id >>= 4)
		*--p = hex_digits[id & 0xf];
#ifndef DB_WINCE
	if(DB_GLOBAL(win_sec_attr) == NULL) {
		InitializeSecurityDescriptor(&DB_GLOBAL(win_default_sec_desc), SECURITY_DESCRIPTOR_REVISION);
		SetSecurityDescriptorDacl(&DB_GLOBAL(win_default_sec_desc), TRUE, 0, FALSE);
		DB_GLOBAL(win_default_sec_attr).nLength = sizeof(SECURITY_ATTRIBUTES);
		DB_GLOBAL(win_default_sec_attr).bInheritHandle = FALSE;
		DB_GLOBAL(win_default_sec_attr).lpSecurityDescriptor = &DB_GLOBAL(win_default_sec_desc);
		DB_GLOBAL(win_sec_attr) = &DB_GLOBAL(win_default_sec_attr);
	}
#endif
	if((*eventp = CreateEvent(DB_GLOBAL(win_sec_attr), FALSE, FALSE, idbuf)) == NULL) {
		ret = __os_get_syserr();
		__db_syserr(env, ret, DB_STR("2002", "Win32 create event failed"));
	}
	return ret;
}

/*
 * __db_win32_mutex_lock --
 *	Internal function to lock a win32 mutex
 *
 *	If the flags includes MUTEX_WAIT, this function will wait for the mutex
 *	to become available; without MUTEX_WAIT it returns DB_LOCK_NOTGRANTED.
 *
 * PUBLIC: int __db_win32_mutex_lock
 * PUBLIC:     __P((ENV *, db_mutex_t, db_timeout_t, int));
 *
 */
int __db_win32_mutex_lock(ENV *env, db_mutex_t mutex, db_timeout_t timeout, int flags)
{
	DB_ENV * dbenv;
	DB_MUTEX * mutexp;
	DB_MUTEXMGR * mtxmgr;
	DB_MUTEXREGION * mtxregion;
	DB_THREAD_INFO * ip;
	HANDLE event;
	uint32 ms, nspins;
	db_timespec now, tempspec, timeoutspec;
	db_timeout_t time_left;
	int ret;
	char buf[DB_THREADID_STRLEN];
#ifdef MUTEX_DIAG
	LARGE_INTEGER now;
#endif
	dbenv = env->dbenv;

	if(!MUTEX_ON(env) || F_ISSET(dbenv, DB_ENV_NOLOCKING))
		return 0;

	mtxmgr = env->mutex_handle;
	mtxregion = (DB_MUTEXREGION *)mtxmgr->reginfo.primary;
	mutexp = MUTEXP_SET(env, mutex);

	if(timeout != 0) {
		timespecclear(&timeoutspec);
		__clock_set_expires(env, &timeoutspec, timeout);
	}

	/*
	 * See WINCE_ATOMIC_MAGIC definition for details.
	 * Use sharecount, because the value just needs to be a db_atomic_t
	 * memory mapped onto the same page as those being Interlocked*.
	 */
	WINCE_ATOMIC_MAGIC(&mutexp->sharecount);

	event = NULL;
	ms = 50;
	ret = 0;

	/*
	 * Fetch the current thread's state struct, if there is a thread hash
	 * table and we are keeping track of this mutex.  We increment the
	 * mtx_ctr before we know that this call succeeds, and decrement it on
	 * failure or in __db_pthread_mutex_unlock.  By incrementing it before
	 * the attempt, we detect crashing that occur during this function.
	 * A crash while holding a non-counted mutex will not be detected by
	 * failchk's mtx_ctr code: those have to be detected some other way.
	 */
	ip = NULL;
	if(env->thr_hashtab != NULL && LF_ISSET(MUTEX_CTR)) {
		if((ret = __env_set_state(env, &ip, THREAD_CTR_VERIFY)) != 0)
			return (__env_panic(env, ret));
		if(ip != NULL) {
			DB_ASSERT(env, ip->mtx_ctr < 20);
			ip->mtx_ctr++;
		}
	}
	/* All non-successful returns after this point should go to failed. */

loop:   /* Attempt to acquire the mutex mutex_tas_spins times, if waiting. */
	for(nspins =
	    mtxregion->stat.st_mutex_tas_spins; nspins > 0; --nspins) {
		/*
		 * We can avoid the (expensive) interlocked instructions if
		 * the mutex is already busy.
		 */
		if(MUTEXP_IS_BUSY(mutexp) || !MUTEXP_ACQUIRE(mutexp)) {
			if(F_ISSET(dbenv, DB_ENV_FAILCHK) && ip != NULL && dbenv->is_alive(dbenv, mutexp->pid, mutexp->tid, 0) == 0) {
				if(ip->dbth_state == THREAD_FAILCHK) {
					ret = USR_ERR(env, DB_RUNRECOVERY);
					__db_err(env, ret, "Failchk blocked by dead process %s on mutex %ld", dbenv->thread_id_string(dbenv, mutexp->pid, mutexp->tid, buf), (u_long)mutex);
					goto failed;
				}
			}
			if(!LF_ISSET(MUTEX_WAIT)) {
				ret = USR_ERR(env, DB_LOCK_NOTGRANTED);
				goto failed;
			}
			/*
			 * Some systems (notably those with newer Intel CPUs)
			 * need a small pause before retrying. [#6975]
			 */
			MUTEX_PAUSE
			continue;
		}
#ifdef HAVE_FAILCHK_BROADCAST
		if(F_ISSET(mutexp, DB_MUTEX_OWNER_DEAD)) {
			MUTEX_UNSET(&mutexp->tas);
			goto died;
		}
#endif
#ifdef DIAGNOSTIC
		if(F_ISSET(mutexp, DB_MUTEX_LOCKED)) {
			__db_errx(env, DB_STR_A("2003", "Win32 lock failed: mutex already locked by %s", "%s"), dbenv->thread_id_string(dbenv, mutexp->pid, mutexp->tid, buf));
			ret = __env_panic(env, EACCES);
			goto failed;
		}
#endif
		F_SET(mutexp, DB_MUTEX_LOCKED);
		dbenv->thread_id(dbenv, &mutexp->pid, &mutexp->tid);
#ifdef HAVE_STATISTICS
		if(event == NULL)
			++mutexp->mutex_set_nowait;
		else
			++mutexp->mutex_set_wait;
#endif
		if(event != NULL) {
			CloseHandle(event);
			InterlockedDecrement(&mutexp->nwaiters);
#ifdef MUTEX_DIAG
			/* "ret" was set by WaitForSingleObject(). */
			if(ret != WAIT_OBJECT_0) {
				QueryPerformanceCounter(&diag_now);
				printf(DB_STR_A("2004", "[%lld]: Lost signal on mutex %p, id %d, ms %d\n", "%lld %p %d %d"), diag_now.QuadPart, mutexp, mutexp->id, ms);
			}
#endif
		}

#ifdef DIAGNOSTIC
		/*
		 * We want to switch threads as often as possible.  Yield
		 * every time we get a mutex to ensure contention.
		 */
		if(F_ISSET(dbenv, DB_ENV_YIELDCPU))
			__os_yield(env, 0, 0);
#endif
		/*
		 * On successful returns do not touch mtx_ctr: if it was
		 * incremented it stays that way.
		 */

		return 0;
	}

	/*
	 * Yield the processor; wait 50 ms initially, up to 1 second.  This
	 * loop is needed to work around a race where the signal from the
	 * unlocking thread gets lost.  We start at 50 ms because it's unlikely
	 * to happen often and we want to avoid wasting CPU.
	 */
	if(timeout != 0) {
		timespecclear(&now);
		if(__clock_expired(env, &now, &timeoutspec)) {
			ret = USR_ERR(env, DB_TIMEOUT);
			goto failed;
		}
		/* Reduce the event wait if the timeout would happen first. */
		tempspec = timeoutspec;
		timespecsub(&tempspec, &now);
		DB_TIMESPEC_TO_TIMEOUT(time_left, &tempspec, 0);
		time_left /= US_PER_MS;
		if(ms > time_left)
			ms = time_left;
	}
	if(event == NULL) {
#ifdef MUTEX_DIAG
		QueryPerformanceCounter(&diag_now);
		printf(DB_STR_A("2005", "[%lld]: Waiting on mutex %p, id %d\n", "%lld %p %d"), diag_now.QuadPart, mutexp, mutexp->id);
#endif
		InterlockedIncrement(&mutexp->nwaiters);
		if((ret = get_handle(env, mutexp, &event)) != 0) {
			InterlockedDecrement(&mutexp->nwaiters);
			goto syserr;
		}
	}
	if((ret = WaitForSingleObject(event, ms)) == WAIT_FAILED) {
		ret = USR_ERR(env, __os_get_syserr());
		goto syserr;
	}
	if((ms <<= 1) > MS_PER_SEC)
		ms = MS_PER_SEC;
#ifdef HAVE_FAILCHK_BROADCAST
	if(F_ISSET(mutexp, DB_MUTEX_OWNER_DEAD) && !F_ISSET(dbenv, DB_ENV_FAILCHK)) {
died:
		ret = __mutex_died(env, mutex);
		goto failed;
	}
#endif
	PANIC_CHECK(env);
	goto loop;
failed:
	/* If this call incremented the counter, decrement it on error. */
	if(ip != NULL) {
		DB_ASSERT(env, ip->mtx_ctr > 0);
		ip->mtx_ctr--;
	}
	if(event != NULL) {
		CloseHandle(event);
		InterlockedDecrement(&mutexp->nwaiters);
	}
	return ret;
syserr: __db_syserr(env, ret, DB_STR("2006", "Win32 lock failed"));
	return (__env_panic(env, __os_posix_err(ret)));
}

/*
 * __db_win32_mutex_init --
 *	Initialize a Win32 mutex.
 *
 * PUBLIC: int __db_win32_mutex_init __P((ENV *, db_mutex_t, uint32));
 */
int __db_win32_mutex_init(ENV *env, db_mutex_t mutex, uint32 flags)
{
	DB_MUTEX * mutexp = MUTEXP_SET(env, mutex);
	mutexp->id = ((getpid() & 0xffff) << 16) ^ P_TO_UINT32(mutexp);
	F_SET(mutexp, flags);
	/*
	 * See WINCE_ATOMIC_MAGIC definition for details.
	 * Use sharecount, because the value just needs to be a db_atomic_t
	 * memory mapped onto the same page as those being Interlocked*.
	 */
	WINCE_ATOMIC_MAGIC(&mutexp->sharecount);
	return 0;
}

#if defined(HAVE_SHARED_LATCHES)
/*
 * __db_win32_mutex_readlock --
 *	Try to lock a mutex, possibly waiting if requested and necessary.
 *
 * PUBLIC: int __db_win32_mutex_readlock __P((ENV *, db_mutex_t, uint32));
 */
int __db_win32_mutex_readlock(ENV *env, db_mutex_t mutex, uint32 flags)
{
	DB_ENV * dbenv;
	DB_MUTEX * mutexp;
	DB_MUTEXMGR * mtxmgr;
	DB_MUTEXREGION * mtxregion;
	DB_THREAD_INFO * ip;
	HANDLE event;
	MUTEX_STATE * state;
	uint32 nspins;
	int max_ms, ms, ret;
	long mtx_val;
#ifdef MUTEX_DIAG
	LARGE_INTEGER diag_now;
#endif
	dbenv = env->dbenv;
	if(!MUTEX_ON(env) || F_ISSET(dbenv, DB_ENV_NOLOCKING))
		return 0;
	mtxmgr = env->mutex_handle;
	mtxregion = (DB_MUTEXREGION *)mtxmgr->reginfo.primary;
	mutexp = MUTEXP_SET(env, mutex);
	/*
	 * See WINCE_ATOMIC_MAGIC definition for details.
	 * Use sharecount, because the value just needs to be a db_atomic_t
	 * memory mapped onto the same page as those being Interlocked*.
	 */
	WINCE_ATOMIC_MAGIC(&mutexp->sharecount);
	event = NULL;
	ms = 50;
	ret = 0;

	ip = NULL;
	state = NULL;
	if(env->thr_hashtab != NULL) {
		if((ret = __env_set_state(env, &ip, THREAD_CTR_VERIFY)) != 0)
			return (__env_panic(env, ret));
		if(env->thr_hashtab != NULL && (ret = __mutex_record_lock(env, mutex, ip, MUTEX_ACTION_INTEND_SHARE, &state)) != 0)
			return ret;
	}
	/* All non-successful returns after this point should go to failed. */

#ifdef HAVE_FAILCHK_BROADCAST
	/*
	 * Limit WaitForSingleObject() sleeps to at most the failchk timeout,
	 * and least 1 millisecond. When failchk broadcasting is not
	 * supported check at least every second.
	 */
	if(dbenv->mutex_failchk_timeout != 0 &&
	    (max_ms = (dbenv->mutex_failchk_timeout / US_PER_MS)) == 0)
		max_ms = 1;
	else
#endif
	max_ms = MS_PER_SEC;
loop:   /* Attempt to acquire the resource for N spins. */
	for(nspins = mtxregion->stat.st_mutex_tas_spins; nspins > 0; --nspins) {
		/*
		 * We can avoid the (expensive) interlocked instructions if
		 * the mutex is already "set".
		 */
retry:          
		mtx_val = atomic_read(&mutexp->sharecount);
		if(mtx_val == MUTEX_SHARE_ISEXCLUSIVE) {
			if(!LF_ISSET(MUTEX_WAIT)) {
				ret = USR_ERR(env, DB_LOCK_NOTGRANTED);
				goto failed;
			}
			continue;
		}
		else if(!atomic_compare_exchange(env, &mutexp->sharecount, mtx_val, mtx_val + 1)) {
			/*
			 * Some systems (notably those with newer Intel CPUs)
			 * need a small pause here. [#6975]
			 */
			MUTEX_PAUSE
			goto retry;
		}
#ifdef HAVE_FAILCHK_BROADCAST
		if(F_ISSET(mutexp, DB_MUTEX_OWNER_DEAD) &&
		    !F_ISSET(dbenv, DB_ENV_FAILCHK)) {
			InterlockedDecrement(
				(interlocked_val)&mutexp->sharecount);
			ret = __mutex_died(env, mutex);
			goto failed;
		}
#endif

#ifdef HAVE_STATISTICS
		if(event == NULL)
			++mutexp->mutex_set_rd_nowait;
		else
			++mutexp->mutex_set_rd_wait;
#endif
		if(event != NULL) {
			CloseHandle(event);
			InterlockedDecrement(&mutexp->nwaiters);
#ifdef MUTEX_DIAG
			if(ret != WAIT_OBJECT_0) {
				QueryPerformanceCounter(&diag_now);
				printf(DB_STR_A("2007", "[%lld]: Lost signal on mutex %p, id %d, ms %d\n", "%lld %p %d %d"), diag_now.QuadPart, mutexp, mutexp->id, ms);
			}
#endif
		}
		if(state != NULL)
			state->action = MUTEX_ACTION_SHARED;

#ifdef DIAGNOSTIC
		/*
		 * We want to switch threads as often as possible.  Yield
		 * every time we get a mutex to ensure contention.
		 */
		if(F_ISSET(dbenv, DB_ENV_YIELDCPU))
			__os_yield(env, 0, 0);
#endif

		return 0;
	}

	/*
	 * Yield the processor; wait 50 ms initially, up to 1 second or the
	 * failchk timeout. This loop works around a race where the signal from
	 * the unlocking thread gets lost.  We start at 50 ms because it's
	 * unlikely to happen often and we want to avoid wasting CPU.
	 */
	if(event == NULL) {
#ifdef MUTEX_DIAG
		QueryPerformanceCounter(&diag_now);
		printf(DB_STR_A("2008", "[%lld]: Waiting on mutex %p, id %d\n", "%lld %p %d"), diag_now.QuadPart, mutexp, mutexp->id);
#endif
		InterlockedIncrement(&mutexp->nwaiters);
		if((ret = get_handle(env, mutexp, &event)) != 0)
			goto err;
	}
	if((ret = WaitForSingleObject(event, ms)) == WAIT_FAILED) {
		ret = __os_get_syserr();
		goto err;
	}

#ifdef HAVE_FAILCHK_BROADCAST
	if(F_ISSET(mutexp, DB_MUTEX_OWNER_DEAD) &&
	    !F_ISSET(dbenv, DB_ENV_FAILCHK)) {
		(void)atomic_compare_exchange(env,
		    &mutexp->sharecount, mtx_val, mtx_val - 1);
		ret = __mutex_died(env, mutex);
		goto failed;
	}
#endif
	PANIC_CHECK(env);
	if((ms <<= 1) > max_ms)
		ms = max_ms;
	goto loop;
failed:
	if(event != NULL) {
		CloseHandle(event);
		InterlockedDecrement(&mutexp->nwaiters);
	}
	if(state != NULL)
		state->action = MUTEX_ACTION_UNLOCKED;
	return ret;
err:    
	__db_syserr(env, ret, DB_STR("2009", "Win32 read lock failed"));
	return (__env_panic(env, __os_posix_err(ret)));
}

#endif

/*
 * __db_win32_mutex_unlock --
 *	Release a mutex.
 *
 * PUBLIC: int __db_win32_mutex_unlock
 * PUBLIC:     __P((ENV *, db_mutex_t, DB_THREAD_INFO *, uint32));
 */
int __db_win32_mutex_unlock(ENV *env, db_mutex_t mutex, DB_THREAD_INFO * ip, uint32 flags)
{
	DB_ENV * dbenv;
	DB_MUTEX * mutexp;
	HANDLE event;
	int ret, sharecount, was_exclusive;
	char description[DB_MUTEX_DESCRIBE_STRLEN];
#ifdef MUTEX_DIAG
	LARGE_INTEGER diag_now;
#endif
	dbenv = env->dbenv;
	if(!MUTEX_ON(env) || F_ISSET(dbenv, DB_ENV_NOLOCKING))
		return 0;
	mutexp = MUTEXP_SET(env, mutex);
	was_exclusive = F_ISSET(mutexp, DB_MUTEX_LOCKED);
#ifdef DIAGNOSTIC
	if(!MUTEXP_IS_BUSY(mutexp) || !(F_ISSET(mutexp, DB_MUTEX_SHARED) ||
	    F_ISSET(mutexp, DB_MUTEX_LOCKED))) {
		__db_errx(env, DB_STR_A("2010", "Win32 unlock failed: lock already unlocked: mutex %d busy %d",
		    "%d %d"), mutex, MUTEXP_BUSY_FIELD(mutexp));
		return (__env_panic(env, EACCES));
	}
#endif
	/*
	 * If the caller hasn't already specified the thread from which to unlock
	 * this mutex, use the current thread.  Failchk can indicate a dead thread.
	 */
	if(env->thr_hashtab != NULL && ip == NULL && (ret = __env_set_state(env, &ip, THREAD_CTR_VERIFY)) != 0)
		return (__env_panic(env, ret));
	/*
	 * If we have a shared latch, and a read lock (DB_MUTEX_LOCKED is only
	 * set for write locks), then decrement the latch. If the readlock is
	 * still held by other threads, just return. Otherwise go ahead and
	 * notify any waiting threads.
	 */
#ifdef HAVE_SHARED_LATCHES
	if(F_ISSET(mutexp, DB_MUTEX_SHARED)) {
		sharecount = atomic_read(&mutexp->sharecount);
		if(sharecount == 0) {
			if(!PANIC_ISSET(env)) {
				__db_errx(env, DB_STR_A("2071", "Shared unlock %s: already unlocked", "%s"), __mutex_describe(env, mutex, description));
				return (DB_RUNRECOVERY);
			}
			return (__env_panic(env, EACCES));
		}
		if(F_ISSET(mutexp, DB_MUTEX_LOCKED)) {
			F_CLR(mutexp, DB_MUTEX_LOCKED);
			if((ret = InterlockedExchange((interlocked_val)(&atomic_read(&mutexp->sharecount)), 0)) != MUTEX_SHARE_ISEXCLUSIVE) {
				ret = DB_RUNRECOVERY;
				goto err;
			}
		}
		else {
			if(InterlockedDecrement((interlocked_val)(&atomic_read(&mutexp->sharecount))) > 0)
				goto finish_shared;
		}
	}
	else
#endif
	{
		if(!F_ISSET(mutexp, DB_MUTEX_LOCKED)) {
			if(!PANIC_ISSET(env)) {
				__db_errx(env, DB_STR_A("2072", "Unlock %s: already unlocked", "%s"), __mutex_describe(env, mutex, description));
				return (DB_RUNRECOVERY);
			}
			return (__env_panic(env, EACCES));
		}
		F_CLR(mutexp, DB_MUTEX_LOCKED);
		MUTEX_UNSET(&mutexp->tas);
	}
	if(mutexp->nwaiters > 0) {
		if((ret = get_handle(env, mutexp, &event)) != 0)
			goto err;
#ifdef MUTEX_DIAG
		QueryPerformanceCounter(&diag_now);
		printf(DB_STR_A("2011", "[%lld]: Signalling mutex %p, id %d\n", "%lld %p %d"), diag_now.QuadPart, mutexp, mutexp->id);
#endif
		if(!PulseEvent(event)) {
			ret = __os_get_syserr();
			CloseHandle(event);
			goto err;
		}
		CloseHandle(event);
	}
	if(was_exclusive) {
		if(ip != NULL && LF_ISSET(MUTEX_CTR)) {
			DB_ASSERT(env, ip->mtx_ctr > 0);
			ip->mtx_ctr--;
		}
	}
	else {
finish_shared:
		if(ip != NULL && (ret = __mutex_record_unlock(env, mutex, ip)) != 0)
			return ret;
	}
	return 0;
err:    
	__db_syserr(env, ret, DB_STR("2012", "Win32 unlock failed"));
	return (__env_panic(env, __os_posix_err(ret)));
}
/*
 * __db_win32_mutex_destroy --
 *	Destroy a mutex.
 *
 * PUBLIC: int __db_win32_mutex_destroy __P((ENV *, db_mutex_t));
 */
int __db_win32_mutex_destroy(ENV * env, db_mutex_t mutex)
{
	return 0;
}
#ifndef DB_WINCE
/*
 * db_env_set_win_security --
 *
 *	Set the SECURITY_ATTRIBUTES to be used by BDB on Windows.
 *	It should not be called while any BDB mutexes are locked.
 *
 * EXTERN: #if defined(DB_WIN32) && !defined(DB_WINCE)
 * EXTERN: int db_env_set_win_security __P((SECURITY_ATTRIBUTES *sa));
 * EXTERN: #endif
 */
int db_env_set_win_security(SECURITY_ATTRIBUTES *sa)
{
	DB_GLOBAL(win_sec_attr) = sa;
	return 0;
}
#endif
