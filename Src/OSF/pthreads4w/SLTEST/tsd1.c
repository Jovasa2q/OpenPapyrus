/*
 * tsd1.c
 * Test Thread Specific Data (TSD) key creation and destruction.
 *      Pthreads4w - POSIX Threads for Windows
 *      Copyright 1998 John E. Bossom
 *      Copyright 1999-2018, Pthreads4w contributors
 *
 *      Homepage: https://sourceforge.net/projects/pthreads4w/
 *
 *      The current list of contributors is contained
 *      in the file CONTRIBUTORS included with the source
 *      code distribution. The list can also be seen at the
 *      following World Wide Web location:
 *
 *      https://sourceforge.net/p/pthreads4w/wiki/Contributors/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <sl_pthreads4w.h>
#pragma hdrstop
#include "test.h"
// 
// Test Method (validation or falsification):
// - validation
// Requirements Tested:
// - keys are created for each existing thread including the main thread
// - keys are created for newly created threads
// - keys are thread specific
// - destroy routine is called on each thread exit including the main thread
// Output:
// - text to stdout
// Assumptions:
// - already validated:     pthread_create(), pthread_once()
// - main thread also has a POSIX thread identity
// 
int PThr4wTest_Tsd1()
{
	const int NUM_THREADS = 100;
	static pthread_key_t key = NULL;
	static int accesscount[NUM_THREADS];
	static int thread_set[NUM_THREADS];
	static int thread_destroyed[NUM_THREADS];
	static pthread_barrier_t startBarrier;

	class InnerBlock {
	public:
		static void destroy_key(void * arg)
		{
			int * j = static_cast<int *>(arg);
			(*j)++;
			assert(*j == 2);
			thread_destroyed[j - accesscount] = 1;
		}
		static void setkey(void * arg)
		{
			int * j = static_cast<int *>(arg);
			thread_set[j - accesscount] = 1;
			assert(*j == 0);
			assert(pthread_getspecific(key) == NULL);
			assert(pthread_setspecific(key, arg) == 0);
			assert(pthread_setspecific(key, arg) == 0);
			assert(pthread_setspecific(key, arg) == 0);
			assert(pthread_getspecific(key) == arg);
			(*j)++;
			assert(*j == 1);
		}
		static void * mythread(void * arg)
		{
			(void)pthread_barrier_wait(&startBarrier);
			setkey(arg);
			return 0;
			/* Exiting the thread will call the key destructor. */
		}
	};
	int i;
	int fail = 0;
	pthread_t thread[NUM_THREADS];
	assert(pthread_barrier_init(&startBarrier, NULL, NUM_THREADS/2) == 0);
	for(i = 1; i < NUM_THREADS/2; i++) {
		accesscount[i] = thread_set[i] = thread_destroyed[i] = 0;
		assert(pthread_create(&thread[i], NULL, InnerBlock::mythread, (void*)&accesscount[i]) == 0);
	}
	/*
	 * Here we test that existing threads will get a key created
	 * for them.
	 */
	assert(pthread_key_create(&key, InnerBlock::destroy_key) == 0);
	(void)pthread_barrier_wait(&startBarrier);
	/*
	 * Test main thread key.
	 */
	accesscount[0] = 0;
	InnerBlock::setkey((void*)&accesscount[0]);
	/*
	 * Here we test that new threads will get a key created
	 * for them.
	 */
	for(i = NUM_THREADS/2; i < NUM_THREADS; i++) {
		accesscount[i] = thread_set[i] = thread_destroyed[i] = 0;
		assert(pthread_create(&thread[i], NULL, InnerBlock::mythread, (void*)&accesscount[i]) == 0);
	}
	/*
	 * Wait for all threads to complete.
	 */
	for(i = 1; i < NUM_THREADS; i++) {
		assert(pthread_join(thread[i], NULL) == 0);
	}
	assert(pthread_key_delete(key) == 0);
	assert(pthread_barrier_destroy(&startBarrier) == 0);
	for(i = 1; i < NUM_THREADS; i++) {
		/*
		 * The counter is incremented once when the key is set to
		 * a value, and again when the key is destroyed. If the key
		 * doesn't get set for some reason then it will still be
		 * NULL and the destroy function will not be called, and
		 * hence accesscount will not equal 2.
		 */
		if(accesscount[i] != 2) {
			fail++;
			fprintf(stderr, "Thread %d key, set = %d, destroyed = %d\n",
			    i, thread_set[i], thread_destroyed[i]);
		}
	}
	fflush(stderr);
	return (fail);
}
// 
// Test Method (validation or falsification):
// - validation
// Requirements Tested:
// - keys are created for each existing thread including the main thread
// - keys are created for newly created threads
// - keys are thread specific
// - destroy routine is called on each thread exit including the main thread
// Output:
// - text to stdout
// Assumptions:
// - already validated:     pthread_create(), pthread_once()
// - main thread also has a POSIX thread identity
// 
int PThr4wTest_Tsd2()
{
	const int NUM_THREADS = 100;
	static pthread_key_t key = NULL;
	static int accesscount[NUM_THREADS];
	static int thread_set[NUM_THREADS];
	static int thread_destroyed[NUM_THREADS];
	static pthread_barrier_t startBarrier;

	class InnerBlock {
	public:
		static void destroy_key(void * arg)
		{
			int * j = static_cast<int *>(arg);
			(*j)++;
			/*
			 * Set TSD key from the destructor to test destructor iteration.
			 * The key value will have been set to NULL by the library before
			 * calling the destructor (with the value that the key had). We
			 * reset the key value here which should cause the destructor to be
			 * called a second time.
			 */
			if(*j == 2)
				assert(pthread_setspecific(key, arg) == 0);
			else
				assert(*j == 3);
			thread_destroyed[j - accesscount] = 1;
		}
		static void setkey(void * arg)
		{
			int * j = static_cast<int *>(arg);
			thread_set[j - accesscount] = 1;
			assert(*j == 0);
			assert(pthread_getspecific(key) == NULL);
			assert(pthread_setspecific(key, arg) == 0);
			assert(pthread_setspecific(key, arg) == 0);
			assert(pthread_setspecific(key, arg) == 0);
			assert(pthread_getspecific(key) == arg);
			(*j)++;
			assert(*j == 1);
		}
		static void * mythread(void * arg)
		{
			(void)pthread_barrier_wait(&startBarrier);
			setkey(arg);
			return 0;
			/* Exiting the thread will call the key destructor. */
		}
	};
	int i;
	int fail = 0;
	pthread_t thread[NUM_THREADS];
	assert(pthread_barrier_init(&startBarrier, NULL, NUM_THREADS/2) == 0);
	for(i = 1; i < NUM_THREADS/2; i++) {
		accesscount[i] = thread_set[i] = thread_destroyed[i] = 0;
		assert(pthread_create(&thread[i], NULL, InnerBlock::mythread, (void*)&accesscount[i]) == 0);
	}
	/*
	 * Here we test that existing threads will get a key created for them.
	 */
	assert(pthread_key_create(&key, InnerBlock::destroy_key) == 0);
	(void)pthread_barrier_wait(&startBarrier);
	/*
	 * Test main thread key.
	 */
	accesscount[0] = 0;
	InnerBlock::setkey((void*)&accesscount[0]);
	/*
	 * Here we test that new threads will get a key created for them.
	 */
	for(i = NUM_THREADS/2; i < NUM_THREADS; i++) {
		accesscount[i] = thread_set[i] = thread_destroyed[i] = 0;
		assert(pthread_create(&thread[i], NULL, InnerBlock::mythread, (void*)&accesscount[i]) == 0);
	}
	/*
	 * Wait for all threads to complete.
	 */
	for(i = 1; i < NUM_THREADS; i++) {
		assert(pthread_join(thread[i], NULL) == 0);
	}
	assert(pthread_key_delete(key) == 0);
	assert(pthread_barrier_destroy(&startBarrier) == 0);
	for(i = 1; i < NUM_THREADS; i++) {
		/*
		 * The counter is incremented once when the key is set to
		 * a value, and again when the key is destroyed. If the key
		 * doesn't get set for some reason then it will still be
		 * NULL and the destroy function will not be called, and
		 * hence accesscount will not equal 2.
		 */
		if(accesscount[i] != 3) {
			fail++;
			fprintf(stderr, "Thread %d key, set = %d, destroyed = %d\n", i, thread_set[i], thread_destroyed[i]);
		}
	}
	fflush(stderr);
	return (fail);
}
// 
// Test Method (validation or falsification):
// - validation
// Requirements Tested:
// - keys are created for each existing thread including the main thread
// - keys are created for newly created threads
// - keys are thread specific
// - key is deleted before threads exit
// - key destructor function is not called
// Output:
// - text to stdout
// Assumptions:
// - already validated:     pthread_create(), pthread_once()
// - main thread also has a POSIX thread identity
// 
int PThr4wTest_Tsd3()
{
	const int NUM_THREADS = 100;
	static pthread_key_t key = NULL;
	static int accesscount[NUM_THREADS];
	static int thread_set[NUM_THREADS];
	static int thread_destroyed[NUM_THREADS];
	static pthread_barrier_t startBarrier;
	static pthread_barrier_t progressSyncBarrier;

	class InnerBlock {
	public:
		static void destroy_key(void * arg)
		{
			// The destructor function should not be called if the key is deleted before the thread exits.
			fprintf(stderr, "The key destructor was called but should not have been.\n");
			exit(1);
		}
		static void setkey(void * arg)
		{
			int * j = static_cast<int *>(arg);
			thread_set[j - accesscount] = 1;
			assert(*j == 0);
			assert(pthread_getspecific(key) == NULL);
			assert(pthread_setspecific(key, arg) == 0);
			assert(pthread_setspecific(key, arg) == 0);
			assert(pthread_setspecific(key, arg) == 0);
			assert(pthread_getspecific(key) == arg);
			(*j)++;
			assert(*j == 1);
		}
		static void * mythread(void * arg)
		{
			(void)pthread_barrier_wait(&startBarrier);
			setkey(arg);
			(void)pthread_barrier_wait(&progressSyncBarrier);
			(void)pthread_barrier_wait(&progressSyncBarrier);
			return 0;
		}
	};
	int i;
	int fail = 0;
	pthread_t thread[NUM_THREADS];
	assert(pthread_barrier_init(&startBarrier, NULL, NUM_THREADS/2) == 0);
	assert(pthread_barrier_init(&progressSyncBarrier, NULL, NUM_THREADS) == 0);
	for(i = 1; i < NUM_THREADS/2; i++) {
		accesscount[i] = thread_set[i] = thread_destroyed[i] = 0;
		assert(pthread_create(&thread[i], NULL, InnerBlock::mythread, (void*)&accesscount[i]) == 0);
	}
	// Here we test that existing threads will get a key created for them.
	assert(pthread_key_create(&key, InnerBlock::destroy_key) == 0);
	(void)pthread_barrier_wait(&startBarrier);
	// Test main thread key.
	accesscount[0] = 0;
	InnerBlock::setkey((void*)&accesscount[0]);
	// Here we test that new threads will get a key created for them.
	for(i = NUM_THREADS/2; i < NUM_THREADS; i++) {
		accesscount[i] = thread_set[i] = thread_destroyed[i] = 0;
		assert(pthread_create(&thread[i], NULL, InnerBlock::mythread, (void*)&accesscount[i]) == 0);
	}
	(void)pthread_barrier_wait(&progressSyncBarrier);
	// Deleting the key should not call the key destructor.
	assert(pthread_key_delete(key) == 0);
	(void)pthread_barrier_wait(&progressSyncBarrier);
	// Wait for all threads to complete.
	for(i = 1; i < NUM_THREADS; i++) {
		assert(pthread_join(thread[i], NULL) == 0);
	}
	assert(pthread_barrier_destroy(&startBarrier) == 0);
	assert(pthread_barrier_destroy(&progressSyncBarrier) == 0);
	for(i = 1; i < NUM_THREADS; i++) {
		// The counter is incremented once when the key is set to a value.
		if(accesscount[i] != 1) {
			fail++;
			fprintf(stderr, "Thread %d key, set = %d, destroyed = %d\n", i, thread_set[i], thread_destroyed[i]);
		}
	}
	fflush(stderr);
	return (fail);
}
