/* It is crucial to write unit tests in developing your programs.
 * The skeleton code here gives some example simplest unit tests:
 * e.g. test_normalize() and test_exact_match(). 
 * Please add more of your own rest of the units tests using 
 * the examples given.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rkmatch.h"

void
test_normalize()
{
	char test_str[100] = "  heLLO   WORLD  ";
	char *match_str = "hello world";
	int len = normalize(test_str, strlen(test_str));
	test_str[len] = '\0'; //because the normalized str is not null-terminated
	assert(strcmp(test_str, match_str) == 0);
	printf("test_normalize: OK\n");
}

void
test_exact_match()
{
	char str1[100] = "  HEllo 	world  ";
	char str2[100] = " hello world";
	char str3[100] = "helllo 	world";

	int l1 = normalize(str1, strlen(str1));
	int l2 = normalize(str2, strlen(str2));
	int l3 = normalize(str3, strlen(str3));
	assert(exact_match(str1, l1, str2, l2));
	assert(!exact_match(str1, l1, str3, l3));
	assert(!exact_match(str2, l2, str3, l3));
	printf("test_exact_match: OK\n");
}

void
test_simple_substr_match()
{

}

void 
test_rkhash_init()
{
	unsigned char buf[3] = {'\1', '\2', '\3'};
	long long h;
	long long hash = rkhash_init(buf, 3, &h);
	assert(h == mmul(mmul(256,256), 256));
	long long expected = mmul(buf[0],mmul(256,256)); 
	expected = madd(expected, mmul(buf[1],256)); 
	expected = madd(expected, buf[2]);
	assert(hash == expected);
	printf("test_rkhash_init: OK\n");
}

void
test_rkhash_next()
{
	unsigned char buf[10];
	for (int i = 0; i < 10; i++) {
		buf[i] = i;
	}
	long long h,h1;
	long long hash = rkhash_init(buf, 3, &h);
	for (int i = 1; i < 10; i++) {
		hash = rkhash_next(hash, h, buf[i-1], buf[i+2]);
		long long expected = rkhash_init(buf+i, 3, &h1);
		assert(hash == expected);
	}
	printf("test_rkhash_next: OK\n");
}

void
test_rabin_karp_match()
{

}

void
test_rabin_karp_batchmatch()
{

}

int
main(int argc, char **argv)
{
	test_normalize();

	test_exact_match();

	test_simple_substr_match();

	test_rkhash_init();

	test_rkhash_next();

	test_rabin_karp_match();

	test_rabin_karp_batchmatch();
}

