/*
 * This file is part of the SSH Library
 *
 * Copyright (c) 2018 by Anderson Toshiyuki Sasaki
 *
 * The SSH Library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or (at your option) any later version.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with the SSH Library; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */
#include <libssh-internal.h>
#pragma hdrstop

static int ssh_winlock_mutex_init(void ** priv)
{
	CRITICAL_SECTION * lock = (CRITICAL_SECTION*)SAlloc::M(sizeof(CRITICAL_SECTION));
	if(lock == NULL) {
		return ENOMEM;
	}
	InitializeCriticalSection(lock);
	*priv = lock;
	return 0;
}

static int ssh_winlock_mutex_destroy(void ** lock)
{
	DeleteCriticalSection((CRITICAL_SECTION*)*lock);
	SAlloc::F(*lock);
	return 0;
}

static int ssh_winlock_mutex_lock(void ** lock)
{
	EnterCriticalSection((CRITICAL_SECTION*)*lock);
	return 0;
}

static int ssh_winlock_mutex_unlock(void ** lock)
{
	LeaveCriticalSection((CRITICAL_SECTION*)*lock);
	return 0;
}

static ulong ssh_winlock_thread_id()
{
	return GetCurrentThreadId();
}

static struct ssh_threads_callbacks_struct ssh_threads_winlock = {
	"threads_winlock", ssh_winlock_mutex_init, ssh_winlock_mutex_destroy, ssh_winlock_mutex_lock, ssh_winlock_mutex_unlock, ssh_winlock_thread_id
};

/* Threads interface implementation */

void ssh_mutex_lock(SSH_MUTEX * mutex)
{
	void * rc;
	CRITICAL_SECTION * mutex_tmp = NULL;
	if(*mutex == NULL) {
		mutex_tmp = (CRITICAL_SECTION *)SAlloc::M(sizeof(CRITICAL_SECTION));
		if(mutex_tmp == NULL) {
			exit(ENOMEM);
		}
		InitializeCriticalSection(mutex_tmp);
		rc = InterlockedCompareExchangePointer((PVOID*)mutex, (PVOID)mutex_tmp, NULL);
		if(rc != NULL) {
			DeleteCriticalSection(mutex_tmp);
			SAlloc::F(mutex_tmp);
			exit(ENOMEM);
		}
	}
	EnterCriticalSection(*mutex);
}

void ssh_mutex_unlock(SSH_MUTEX * mutex)
{
	LeaveCriticalSection(*mutex);
}

struct ssh_threads_callbacks_struct * ssh_threads_get_winlock()
{
	return &ssh_threads_winlock;
}

struct ssh_threads_callbacks_struct * ssh_threads_get_default()
{
	return &ssh_threads_winlock;
}
