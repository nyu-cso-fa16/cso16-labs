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

    int whitespace_counter = 0;
    int i;
	for(i=0; i<len; i++){//for each character in the string
		//if that character is uppercase
		if(isupper(buf[i])){
			//make it lowercase
			buf[i] = tolower(buf[i]);
			//if we have more than one whitespace before it
			if(whitespace_counter>1){
				int j;
				//set each character back by one fewer position than the number of whitespaces.
				for(j = i; j<len; j++){
					buf[j-whitespace_counter+1] = buf[j];
				}
				//adjust the string length and counter accordingly
				len = len - whitespace_counter+1;
				i = i- whitespace_counter + 1;
			}
			//reset the whitespace counter
			whitespace_counter = 0;
		}
		//if the character is whitespace
		else if(isspace(buf[i])){
			//make it a regular space
			buf[i] = ' ';
			//and increment the whitespace counter
			whitespace_counter = whitespace_counter + 1;
		//if it's anything else
		}else{
			//just check the whitespace counter and act accordingly.
			if(whitespace_counter>1){
				int j;
				for(j = i; j<len; j++){
					buf[j-whitespace_counter+1] = buf[j];
				}
				len = len - whitespace_counter+1;
				i = i - whitespace_counter + 1;
			}
			whitespace_counter = 0;
		}
	}
	//At the end, if the first character is a space,
	if(buf[0] == ' '){
		//move the string back by 1.
		for(i=1; i<len; i++){
			buf[i-1] = buf[i];
		}
		//and adjust the length accordingly.
		len = len - 1;
	}
	//If the last character is a space,
	while(buf[len-1]== ' '){
		//remove it.
		len = len - 1;
	}
    return len;
}

int
exact_match(const unsigned char *qs, int m, 
        const unsigned char *ts, int n)
{
	//create non-constant versions of the strings
	char* qs2 = malloc(m);
	strcpy(qs2, qs);
	char* ts2 = malloc(n);
	strcpy(ts2, ts);
	//normalize both
    m = normalize(qs2, m);
    int l;
    n = normalize(ts2, n);
    //if not equal in size
    if(n!=m){
    	//return false
    	return 0;
    //otherwise
    }else{
    	int i;
    	//increment through every character in both strings and compare
    	for(i=0; i<n; i++){
    		if(*(ts2+i) != *(qs2+i)){
    			//if any do not match, return 0.
    			return 0;
    		}
    	}
    	//If they all do, return 1.
    	return 1;
    }
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
	int i;
	//increment through all n-k+1 substrings.
	for(i=0; i<n-k+1; i++){
		//If the current one matches the query string
		if(!strncmp(ts+i, ps, k))
			//return true
			return 1;
	}
	//otherwise, if none match, return false.
	return 0;;
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
	int i;
	//set the hash to start at 0, and the h-value to 1.
	long long hash = 0;
	*h = 1;
	//for each of the k characters,
	for(i=0; i<k; i++){
		//multiply the h-value by 256, eventually getting it to 256^k
		*h = mmul(*h, 256);
		//and multiply the hash value by 256, before adding the next character.
		hash = madd(mmul(hash, 256), charbuf[i]);
	}
	return hash;
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
	//This is simply the modulo math version of the above expression.
	return madd(mdel(mmul(curr_hash, 256), mmul(prev, h)), next);
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
	long long* h = (long long*)malloc(sizeof(long long));
	long long query_hash = rkhash_init(ps, k, h);
	printf("%lld\n", query_hash);
	long long test_hash = rkhash_init(ts, k, h);
	//Store the test hash for the printing in a temporary long long variable.
	long long temp = test_hash;
	int i;
	//Print the first few hashes
	for(i=0; i<PRINT_RK_HASH; i++){
		printf("%lld ", temp);
		temp = rkhash_next(temp, *h, ts[i], ts[i+k]);
	}
	printf("\n");
	//For each of the document substrings
	for(i=0; i<n-k; i++){
		//if the query hash matches the test hash,
		if(query_hash == test_hash)
			//check that the strings match,
			if(!strncmp(ts+i, ps, k))
				//and if so, return true.
				return 1;
		//otherwise, calculate the next hash.
		test_hash = rkhash_next(test_hash, *h, ts[i], ts[i+k]);
	}
	//If no matches were found, return false.
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
	bloom_filter qsf = bloom_init(bsz);
	long long* h = (long long*)malloc(sizeof(long long));
	int i;
	int matches = 0;
	//Add each of the m/k chunks to the filter.
	for(i=0; i < m/k; i++){
		bloom_add(qsf, rkhash_init(qs+(i*k), k, h));
	}
	bloom_print(qsf, PRINT_BLOOM_BITS);
	//Create the first hash of the document.
	long long test_hash = rkhash_init(ts, k, h);
	for(i=0; i < n-k; i++){
		//Check if the hash is in the filter.
		if(bloom_query(qsf, test_hash)){
			//If so, then compare the strings.
			int j;
			for(j=0; j < m/k; j++)
				//Increment the number of matches if there's a match.
				if(!strncmp(ts+i, qs+(j*k), k))
					matches++;
		}
		//Move on to the next hash.
		test_hash = rkhash_next(test_hash, *h, ts[i], ts[i+k]);
	}
	//Return the number of matches.
	return matches;
}