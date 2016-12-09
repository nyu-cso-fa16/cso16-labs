#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>

void test_htable();
void test_rwl_basic();
void test_rwl_priority();

int num_threads = 4;

int
main(int argc, char **argv)
{
	char *which_test = "all";
	int c;
	while ((c = getopt(argc, argv, "n:t:")) != -1) {
	       	switch (c) {
			case 'n':
				num_threads = atoi(optarg);
				break;
			case 't':
				which_test = optarg;
				break;
			default:
				fprintf(stderr, "Usage: tester \n");
			       	fprintf(stderr, "Options\n");
			       	fprintf(stderr, "\t-t <htable, rwl, resize, all>   Which test to run\n");
			       	fprintf(stderr, "\t-n <num>   Number of testing threads (default is %d)\n", num_threads);
			       	exit(1);
		}
	}

	int tested = 0;
	if (strcmp(which_test, "all") == 0 || strcmp(which_test, "htable") == 0) {
		test_htable(0); 
		tested++;
	}
	
	if (strcmp(which_test, "all") == 0 || strcmp(which_test, "rwl") == 0) {
		test_rwl_basic();
		test_rwl_priority();
		/*
		test_rwl_integration();
		*/
		tested++;
	}
	
	if (strcmp(which_test, "all") == 0 || strcmp(which_test, "resize") == 0) {
		test_htable(1);
		tested++;
	}

	if (tested == 0) {
		printf("No tests performed. Did you specify the wrong test type?\n");
		exit(1);
	} 
	return 0;
}


