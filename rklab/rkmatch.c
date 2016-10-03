/* Match every k-character snippet of the query_doc document
	 among a collection of documents doc1, doc2, ....

	 ./rkmatch snippet_size query_doc doc1 [doc2...]

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <ctype.h>

#include "bloom.h"

enum algotype { EXACT=0, SIMPLE, RK, RKBATCH};

/* a large prime for RK hash (BIG_PRIME*256 does not overflow)*/
long long BIG_PRIME = 5003943032159437; 


/* constants used for printing debug information */
const int PRINT_RK_HASH = 5;
const int PRINT_BLOOM_BITS = 160;


/* calculate modulo addition, i.e. (a+b) % BIG_PRIME */
long long
madd(long long a, long long b)
{
	assert(a >= 0 && a < BIG_PRIME && b >= 0 && b < BIG_PRIME);
	return ((a+b)>BIG_PRIME?(a+b-BIG_PRIME):(a+b));
}

/* calculate modulo substraction, i.e. (a-b) % BIG_PRIME*/
long long
mdel(long long a, long long b)
{
	assert(a >= 0 && a < BIG_PRIME && b >= 0 && b < BIG_PRIME);
	return ((a>b)?(a-b):(a+BIG_PRIME-b));
}

/* calculate modulo multiplication, i.e. (a*b) % BIG_PRIME */
long long
mmul(long long a, long long b)
{
	assert(a >= 0 && a < BIG_PRIME && b >= 0 && b < BIG_PRIME);
	/* either operand must be no larger than 256, otherwise, there is danger of overflow*/
	assert(a <= 256 || b <= 256); 
	return ((a*b) % BIG_PRIME);
}

/* read the entire content of the file 'fname' into a 
	 character array allocated by this procedure.
	 Upon return, *doc contains the address of the character array
	 *doc_len contains the length of the array
	 */
void
read_file(const char *fname, unsigned char **doc, int *doc_len) 
{

	int fd = open(fname, O_RDONLY);
	if (fd < 0) {
		perror("read_file: open ");
		exit(1);
	}

	struct stat st;
	if (fstat(fd, &st) != 0) {
		perror("read_file: fstat ");
		exit(1);
	}

	*doc = (unsigned char *)malloc(st.st_size);
	if (!(*doc)) {
		fprintf(stderr, " failed to allocate %d bytes. No memory\n", (int)st.st_size);
		exit(1);
	}

	int n = read(fd, *doc, st.st_size);
	if (n < 0) {
		perror("read_file: read ");
		exit(1);
	}else if (n != st.st_size) {
		fprintf(stderr,"read_file: short read!\n");
		exit(1);
	}
	
	close(fd);
	*doc_len = n;
}

/* The normalize procedure normalizes a character array of size len 
   according to the following rules:
	 1) turn all upper case letters into lower case ones
	 2) turn any white-space character into a space character and, 
	    shrink any n>1 consecutive whitespace characters to exactly 1 whitespace

	 When the procedure returns, the character array buf contains the newly 
     normalized string and the return value is the new length of the normalized string.

     hint: you may want to use C library function isupper, isspace, tolower
     To find out how to use isupper function, type "man isupper" at the terminal
*/
int
normalize(unsigned char *buf,	/* The character array contains the string to be normalized*/
	int len	    /* the size of the original character array */)
{
    /* your code here */

    return 0;


}

int
exact_match(const unsigned char *qs, int m, 
        const unsigned char *ts, int n)
{
    /* your code here */
    return 0;
}

/* check if a query string ps (of length k) appears 
	 in ts (of length n) as a substring 
	 If so, return 1. Else return 0
	 You may want to use the library function strncmp
	 */
int
simple_substr_match(const unsigned char *ps,	/* the query string */
	int k,/* the length of the query string */
	const unsigned char *ts,/* the document string (Y) */ 
	int n/* the length of the document Y */)
{
    /* Your code here */
	return 0;
}

/* initialize the Rabin-karp hash computation by calculating 
 * and returning the hash over a charbuf of k characters.
 * i.e. return value should be 256^(k-1)*charbuf[0] + 256*(k-2)*charbuf[1] + ... + charbuf[k-1]
 * it also returns the value 256^k in the long long variable pointed to by h
 * Note: We use the "long long" type to represent an RK hash.
 */
long long
rkhash_init(const unsigned char *charbuf, int k, long long *h)
{
	/* Your code here */
}


/* Given the rabin-karp hash value (curr_hash) over substring Y[i],Y[i+1],...,Y[i+k-1]
 * calculate the hash value over Y[i+1],Y[i+2],...,Y[i+k] 
 * = curr_hash * 256 - prev * h + next
 * Note that the above *,-,+ operator must be done as modular arithematic.
 * The call of rkhash_next must invoke the function with h = 256^k and prev = Y[i] and next = Y[i+k]
 */
long long 
rkhash_next(long long curr_hash, long long h, unsigned char prev, unsigned char next)
{
	/* Your code here */
}

/* Check if a query string ps (of length k) appears 
   in ts (of length n) as a substring using the rabin-karp algorithm
   If so, return 1. Else return 0

   Hint: Complete and use rkhash_init and rkhash_next functions

   In addition, print the hash value of ps (on one line)
   as well as the first 'PRINT_RK_HASH' hash values of ts (on another line)
   Example:
	 $ ./rkmatch -t 2 -k 20 X Y
	 4537305142160169
	 1137948454218999 2816897116259975 4720517820514748 4092864945588237 3539905993503426 
	 2021356707496909
	 1137948454218999 2816897116259975 4720517820514748 4092864945588237 3539905993503426 
	 0 chunks matched (out of 2), percentage: 0.00 
   Hint: Use printf("%lld", x) to print a long long type.
   */
int
rabin_karp_match(const unsigned char *ps,	/* the query string */
	int k,/* the length of the query string */
	const unsigned char *ts,/* the document string (Y) */ 
	int n/* the length of the document Y */ )
{
	
    /* Your code here */
	return 0;
}


/* Allocate a bitmap of bsz bits for the bloom filter (using bloom_init)
   and insert all m/k RK hashes of qs into the bloom filter (using bloom_add).  Compute each of the n-k+1 RK hashes 
   of ts and check if it's in the filter (using bloom_query). 

   This function returns the total number of matched chunks. 

   For testing purpose, you should print out the first PRINT_BLOOM_BITS bits 
   of bloom bitmap in hex after inserting all m/k chunks from qs.
 */
int
rabin_karp_batchmatch(int bsz, /* size of bitmap (in bits) to be used */
    int k,/* chunk length to be matched */
    const unsigned char *qs, /* query docoument (X)*/
    int m,/* query document length */ 
    const unsigned char *ts, /* to-be-matched document (Y) */
    int n /* to-be-matched document length*/)
{

    /* Your code here */
    return 0;
}


