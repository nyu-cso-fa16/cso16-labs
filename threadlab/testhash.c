#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <pthread.h>

#include "htable.h"

#define TESTSZ 1000000
#define STRLEN 50

extern int num_threads;

//a set of test keys
static char testkeys[TESTSZ][STRLEN];
static int testvals[TESTSZ];

static htable ht;

typedef enum {
	INSERT,
	MIX
}op_type;

static op_type run_mode;
int htest_allow_resize;

void test_fatal(char *testname, char *errmsg);

static inline
long timediff(const struct timespec *start, const struct timespec *end)
{
	return (end->tv_sec - start->tv_sec)*1000000 + (end->tv_nsec - start->tv_nsec)/1000;
}

void
set_random_str(char *s, int len)
{
	for (int i = 0; i < len-1; i++) {
		s[i] = rand() % 26 + 'A';
	}
	s[len-1] = '\0';
}


void *
test_htable_run(void *arg)
{
	long thread_idx = (long)arg;

	int share = TESTSZ / num_threads;
	int end = (thread_idx+1)*share;
	if (thread_idx == (num_threads -1)) 
		end = TESTSZ;

	for (long i = thread_idx * share; i < end; i++) {
	       	htable_insert(&ht, testkeys[i], &testvals[i]);
		//insert another randomly chosen key/val tuple
		long r = (((thread_idx * i) % 256) * BIG_PRIME) % TESTSZ;
		if (run_mode == INSERT) {
		       	htable_insert(&ht, testkeys[r], &testvals[r]);
		}else{
		       	void *v = htable_lookup(&ht, testkeys[r]);
			if (v != NULL) {
				if (v != &testvals[r]) {
					char *htestname = htest_allow_resize? "RESIZE":"HTABLE";
					test_fatal(htestname, "Concurrent lookup found that the existing tuple val does not match inserted.");
				}
			}
	       	}

	}
	return NULL;
}

void
test_htable(int allow_resize)
{
	htest_allow_resize = allow_resize;
	char *htestname = allow_resize? "RESIZE TEST":"HTABLE TEST";
	char errmsg[1000];
	//initialize testkeys
	for (int i = 0; i < TESTSZ; i++) {
		set_random_str(testkeys[i], STRLEN);
	}
	htable_init(&ht, TESTSZ/100, allow_resize);
	printf("Initialized hash table of size %d\n", htable_size(&ht));

	pthread_t *threads = (pthread_t *)malloc(sizeof(pthread_t)*num_threads);

	run_mode = INSERT;
	struct timespec start, end;
	clock_gettime(CLOCK_REALTIME, &start);
	for (long i = 0; i < num_threads; i++) {
		assert(pthread_create(&threads[i], NULL, test_htable_run, (void *)i) == 0);
	}
	printf("Spawned %d threads, each to insert %d tuples\n", num_threads, 2*TESTSZ/num_threads);

	for (long i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}
	clock_gettime(CLOCK_REALTIME, &end);
	long duration = timediff(&start, &end);
	printf("All %d threads finished. Throughput is %2f inserts/sec\n", num_threads, (double)2*TESTSZ/(double)duration);

	//validate 
	for (int i = 0; i < TESTSZ; i++) {
		void *p = htable_lookup(&ht, testkeys[i]);
		if (p && p == &testvals[i]) 
			continue;
		if (p == NULL) {
			snprintf(errmsg, 1000, "htable %d is missing key %s, value %p", i, testkeys[i], &testvals[i]);	
			test_fatal(htestname, errmsg);
		}else {
			snprintf(errmsg, 1000, "htable has wrong value (%p) for key %s value %p tuple", p, testkeys[i], &testvals[i]);	
			test_fatal(htestname, errmsg);
		}
		// clear up the threads array early due to the exit() call
		free(threads);
		exit(1);
	}


	//test a mix of insert and lookup operations
	htable_destroy(&ht);
	htable_init(&ht, TESTSZ/100, allow_resize);
	run_mode = MIX;
	clock_gettime(CLOCK_REALTIME, &start);
	for (long i = 0; i < num_threads; i++) {
		assert(pthread_create(&threads[i], NULL, test_htable_run, (void *)i) == 0);
	}
	printf("Spawned %d threads, each to insert or lookup %d tuples\n", num_threads, 2*TESTSZ/num_threads);

	for (long i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
	}
	clock_gettime(CLOCK_REALTIME, &end);
	duration = timediff(&start, &end);
	printf("All %d threads finished. Throughput is %2f lookups/sec\n", num_threads, (double)2*TESTSZ/(double)duration);

	int sz = htable_size(&ht);
	htable_destroy(&ht);
	printf("--- %s PASSED (final htable size %d) \n", htestname, sz);
	// clear up the threads array that we malloced earlier
	free(threads);
}
