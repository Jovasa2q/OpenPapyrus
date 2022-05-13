/*
 * Copyright (c) Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under both the BSD-style license (found in the
 * LICENSE file in the root directory of this source tree) and the GPLv2 (found
 * in the COPYING file in the root directory of this source tree).
 * You may select, at your option, one of the above-listed licenses.
 */
#ifndef ZSTD_FILEIO_ASYNCIO_H
#define ZSTD_FILEIO_ASYNCIO_H

#if defined (__cplusplus)
extern "C" {
#endif

#include <zstd_mem.h> // uint32, uint64
#include "fileio_types.h"
#include "platform.h"
#include "util.h"
#include <pool.h>
#include <threading.h>

#define MAX_IO_JOBS          (10)

typedef struct {
	/* These struct fields should be set only on creation and not changed afterwards */
	POOL_ctx* threadPool;
	int totalIoJobs;
	const FIO_prefs_t* prefs;
	POOL_function poolFunction;

	/* Controls the file we currently write to, make changes only by using provided utility functions */
	FILE* file;

	/* The jobs and availableJobsCount fields are accessed by both the main and worker threads and should
	 * only be mutated after locking the mutex */
	ZSTD_pthread_mutex_t ioJobsMutex;
	void* availableJobs[MAX_IO_JOBS];
	int availableJobsCount;
	size_t jobBufferSize;
} IOPoolCtx_t;

typedef struct {
	IOPoolCtx_t base;
	/* State regarding the currently read file */
	int reachedEof;
	uint64 nextReadOffset;
	uint64 waitingOnOffset;
	void * currentJobHeld; /* We may hold an IOJob object as needed if we actively expose its buffer. */
	/* Coalesce buffer is used to join two buffers in case where we need to read more bytes than left in
	 * the first of them. Shouldn't be accessed from outside ot utility functions. */
	uint8 * coalesceBuffer;
	/* Read buffer can be used by consumer code, take care when copying this pointer aside as it might
	 * change when consuming / refilling buffer. */
	uint8 * srcBuffer;
	size_t srcBufferLoaded;
	/* We need to know what tasks completed so we can use their buffers when their time comes.
	 * Should only be accessed after locking base.ioJobsMutex . */
	void* completedJobs[MAX_IO_JOBS];
	int completedJobsCount;
	ZSTD_pthread_cond_t jobCompletedCond;
} ReadPoolCtx_t;

typedef struct {
	IOPoolCtx_t base;
	uint   storedSkips;
} WritePoolCtx_t;

typedef struct {
	/* These fields are automatically set and shouldn't be changed by non WritePool code. */
	void * ctx;
	FILE* file;
	void * buffer;
	size_t bufferSize;
	/* This field should be changed before a job is queued for execution and should contain the number
	 * of bytes to write from the buffer. */
	size_t usedBufferSize;
	uint64 offset;
} IOJob_t;

/* AIO_supported:
 * Returns 1 if AsyncIO is supported on the system, 0 otherwise. */
int AIO_supported(void);

/* AIO_WritePool_releaseIoJob:
 * Releases an acquired job back to the pool. Doesn't execute the job. */
void AIO_WritePool_releaseIoJob(IOJob_t * job);

/* AIO_WritePool_acquireJob:
 * Returns an available write job to be used for a future write. */
IOJob_t* AIO_WritePool_acquireJob(WritePoolCtx_t * ctx);

/* AIO_WritePool_enqueueAndReacquireWriteJob:
 * Enqueues a write job for execution and acquires a new one.
 * After execution `job`'s pointed value would change to the newly acquired job.
 * Make sure to set `usedBufferSize` to the wanted length before call.
 * The queued job shouldn't be used directly after queueing it. */
void AIO_WritePool_enqueueAndReacquireWriteJob(IOJob_t ** job);

/* AIO_WritePool_sparseWriteEnd:
 * Ends sparse writes to the current file.
 * Blocks on completion of all current write jobs before executing. */
void AIO_WritePool_sparseWriteEnd(WritePoolCtx_t * ctx);

/* AIO_WritePool_setFile:
 * Sets the destination file for future writes in the pool.
 * Requires completion of all queues write jobs and release of all otherwise acquired jobs.
 * Also requires ending of sparse write if a previous file was used in sparse mode. */
void AIO_WritePool_setFile(WritePoolCtx_t * ctx, FILE* file);

/* AIO_WritePool_getFile:
 * Returns the file the writePool is currently set to write to. */
FILE* AIO_WritePool_getFile(const WritePoolCtx_t* ctx);

/* AIO_WritePool_closeFile:
 * Ends sparse write and closes the writePool's current file and sets the file to NULL.
 * Requires completion of all queues write jobs and release of all otherwise acquired jobs.  */
int AIO_WritePool_closeFile(WritePoolCtx_t * ctx);

/* AIO_WritePool_create:
 * Allocates and sets and a new write pool including its included jobs.
 * bufferSize should be set to the maximal buffer we want to write to at a time. */
WritePoolCtx_t* AIO_WritePool_create(const FIO_prefs_t* prefs, size_t bufferSize);

/* AIO_WritePool_free:
 * Frees and releases a writePool and its resources. Closes destination file. */
void AIO_WritePool_free(WritePoolCtx_t* ctx);

/* AIO_ReadPool_create:
 * Allocates and sets and a new readPool including its included jobs.
 * bufferSize should be set to the maximal buffer we want to read at a time, will also be used
 * as our basic read size. */
ReadPoolCtx_t* AIO_ReadPool_create(const FIO_prefs_t* prefs, size_t bufferSize);

/* AIO_ReadPool_free:
 * Frees and releases a readPool and its resources. Closes source file. */
void AIO_ReadPool_free(ReadPoolCtx_t* ctx);

/* AIO_ReadPool_consumeBytes:
 * Consumes byes from srcBuffer's beginning and updates srcBufferLoaded accordingly. */
void AIO_ReadPool_consumeBytes(ReadPoolCtx_t * ctx, size_t n);

/* AIO_ReadPool_fillBuffer:
 * Makes sure buffer has at least n bytes loaded (as long as n is not bigger than the initialized bufferSize).
 * Returns if srcBuffer has at least n bytes loaded or if we've reached the end of the file.
 * Return value is the number of bytes added to the buffer.
 * Note that srcBuffer might have up to 2 times bufferSize bytes. */
size_t AIO_ReadPool_fillBuffer(ReadPoolCtx_t * ctx, size_t n);

/* AIO_ReadPool_consumeAndRefill:
 * Consumes the current buffer and refills it with bufferSize bytes. */
size_t AIO_ReadPool_consumeAndRefill(ReadPoolCtx_t * ctx);

/* AIO_ReadPool_setFile:
 * Sets the source file for future read in the pool. Initiates reading immediately if file is not NULL.
 * Waits for all current enqueued tasks to complete if a previous file was set. */
void AIO_ReadPool_setFile(ReadPoolCtx_t * ctx, FILE* file);

/* AIO_ReadPool_getFile:
 * Returns the current file set for the read pool. */
FILE* AIO_ReadPool_getFile(const ReadPoolCtx_t * ctx);

/* AIO_ReadPool_closeFile:
 * Closes the current set file. Waits for all current enqueued tasks to complete and resets state. */
int AIO_ReadPool_closeFile(ReadPoolCtx_t * ctx);

#if defined (__cplusplus)
}
#endif

#endif /* ZSTD_FILEIO_ASYNCIO_H */
