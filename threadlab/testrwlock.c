#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "rwlock.h"

extern int num_threads;

//global variable for the test_rwl_basic
static rwl l1;

//global variables for test_rwl_priority
static rwl l2;
static int reader_in_cs = 0;
static int who_is_in_cs = 0;
static int reader_getout = 0;
static int peers_getout = 0;

//global variables for test_rwl_integration
#define ARRAYSZ  20000000
#define READOPS  100
#define WRITEOPS 10000
rwl lock;
static int testarray[ARRAYSZ];
static int test_ind = 0;
int num_writers;
int num_readers;

static char *rwltest = "RWLOCK TEST";

pthread_mutex_t test_m;
pthread_cond_t test_peers;
pthread_cond_t test_linger;

//get_expiretime returns an absolute time that's 100ms in the future
static inline void
get_expiretime(struct timespec *expire)
{
	clock_gettime(CLOCK_REALTIME, expire);
	expire->tv_nsec += 100;
}

void
test_fatal(char *testname, char *errmsg)
{
	printf("--- %s FAILED: %s\n", testname, errmsg);
	exit(1);
}

void
test_rwl_basic()
{
	struct timespec expire;

	rwl_init(&l1);
	if (rwl_rlock(&l1, NULL) != 0) {
		test_fatal(rwltest, "Reader failed to lock!");
	}else{
	       	printf("Reader grabbed the lock\n");
	}
	get_expiretime(&expire);
	if (rwl_wlock(&l1, &expire) != ETIMEDOUT) {
		test_fatal(rwltest, "Another writer should not be able to lock");
	}
	get_expiretime(&expire);
	if (rwl_rlock(&l1, &expire) != 0) {
		test_fatal(rwltest, "Another reader should be able to lock");
	}
       	printf("Another reader grabbed the lock\n");
	rwl_runlock(&l1);
       	printf("One reader unlocked\n");
	rwl_runlock(&l1);
       	printf("Another reader unlocked\n");


	if (rwl_wlock(&l1, NULL) != 0) {
		test_fatal(rwltest, "Writer failed to lock!");
	}else{
	       	printf("Writer grabbed the lock\n");
	}

	get_expiretime(&expire);
	if (rwl_wlock(&l1, &expire) != ETIMEDOUT) {
		test_fatal(rwltest, "Another writer should not be able to lock");
	}
	get_expiretime(&expire);
	if (rwl_rlock(&l1, &expire) != ETIMEDOUT) {
		test_fatal(rwltest, "Another reader should not be able to lock\n");
	}
	rwl_wunlock(&l1);
       	printf("Writer unlocked\n");

	printf("--- %s PASSED basic correctness\n", rwltest);
}


void *
linger_reader(void *arg)
{
	if (rwl_rlock(&l2, NULL) != 0) {
		test_fatal(rwltest, "Reader failed to lock!");
	}

	pthread_mutex_lock(&test_m);
	//set global variable indiciating reader in critical section. this is racey.
	reader_in_cs = 1; 
	//wait for master thread to ask me to get out
	while (reader_getout == 0) 
		pthread_cond_wait(&test_linger, &test_m); 
	pthread_mutex_unlock(&test_m);

	rwl_runlock(&l2);

}

void *
competing_writer(void *arg)
{
	if (rwl_wlock(&l2, NULL) != 0) {
		test_fatal(rwltest, "Writer failed to lock");
	}

	pthread_mutex_lock(&test_m);
	//indicate that writer is in CS
	who_is_in_cs = 1; 
	//wait for master thread to ask me to get out
	while (peers_getout == 0) 
		pthread_cond_wait(&test_peers, &test_m);
	pthread_mutex_unlock(&test_m);

	rwl_wunlock(&l2);

}
void *
competing_reader(void *arg) 
{
	if (rwl_rlock(&l2, NULL) != 0) {
		test_fatal(rwltest, "Reader failed to lock");
	}

	pthread_mutex_lock(&test_m);
	//indiciate that reader is in CS
	who_is_in_cs = 2; 
	while (peers_getout == 0)
		pthread_cond_wait(&test_peers, &test_m);
	pthread_mutex_unlock(&test_m);

	rwl_runlock(&l2);
}

void
test_rwl_priority()
{
	pthread_t threads[10];

	assert(pthread_mutex_init(&test_m, NULL) == 0);
	assert(pthread_cond_init(&test_linger, NULL) == 0);
	assert(pthread_cond_init(&test_peers, NULL) == 0);

	rwl_init(&l2);
	assert(pthread_create(&threads[0], NULL, linger_reader, NULL) == 0);

	int reader_in = 0;
	while (!reader_in) {
		pthread_mutex_lock(&test_m);
		reader_in = reader_in_cs;
		pthread_mutex_unlock(&test_m);
	}
	printf("Reader has locked but not yet unlocked\n");

	assert(pthread_create(&threads[1], NULL, competing_writer, NULL) == 0);
	while (rwl_nwaiters(&l2) != 1); //now writer is waiting for the lock
	printf("Spawned a writer who is now waiting to lock\n");

	for (int i = 2; i < 10; i++) {
		assert(pthread_create(&threads[i], NULL, competing_reader, NULL) == 0);
	}

	usleep(100); //well, i'll just wait for 100 msec

	int who = 0;
	//check if any reader has finished
	pthread_mutex_lock(&test_m);
	who = who_is_in_cs;
	pthread_mutex_unlock(&test_m);
	if (who == 1) {
		test_fatal(rwltest, "Competing writer should not have been in critical section while reader is still there\n");
	}else if (who == 2) {
		test_fatal(rwltest, "Competing readers should not have been in critical section while there is a competing writer\n");
	}

	while (rwl_nwaiters(&l2) != 9); //now writer and other 8 readers are all waiting
	printf("Spawned 8 readers who are now waiting to lock\n");

	//ask the lingering reader to get out
	pthread_mutex_lock(&test_m);
	reader_getout = 1; 
	pthread_cond_broadcast(&test_linger);
	pthread_mutex_unlock(&test_m);

	//waiting for one of the competing readers or writers to grab rwlock
	while (!who) {
		pthread_mutex_lock(&test_m);
		who = who_is_in_cs;
		pthread_mutex_unlock(&test_m);
	}
	if (who != 1) {
		test_fatal(rwltest, "Writer should have priority over readers in locking");
	}
	printf("Writer locked before readers\n");

	//asking everybody to get out so i can join
	pthread_mutex_lock(&test_m);
	peers_getout = 1;
	pthread_cond_broadcast(&test_peers);
	pthread_mutex_unlock(&test_m);

	for (int i = 0; i < 10; i++) {
		pthread_join(threads[i], NULL);
	}

	pthread_mutex_destroy(&test_m);
	pthread_cond_destroy(&test_linger);
	pthread_cond_destroy(&test_peers);
	printf("--- %s TEST PASSED (writer is given priority)\n", rwltest);

}

