
/***********************************************************
 File Name: bloom.c
 Description: implementation of bloom filter goes here 
 **********************************************************/

#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "bloom.h"

/* Constants for bloom filter implementation */
const int H1PRIME = 4189793;
const int H2PRIME = 3296731;
const int BLOOM_HASH_NUM = 10;

/* The hash function used by the bloom filter */
int
hash_i(int i, /* which of the BLOOM_HASH_NUM hashes to use */ 
       long long x /* the element (a RK value) to be hashed */)
{
	return ((x % H1PRIME) + i*(x % H2PRIME) + 1 + i*i);
}

/* Initialize a bloom filter by allocating a character array that can pack bsz bits.
   Furthermore, clear all bits for the allocated character array. 
   Each char should contain 8 bits of the array.
   Hint:  use the malloc and memset library function 
	 Return value is the allocated character array.*/
bloom_filter
bloom_init(int bsz /* size of bitmap to allocate in bits*/ )
{
	bloom_filter f;

	assert((bsz % 8) == 0);
	f.bsz = bsz;

	/* your code here*/
	f.buf = (char*)malloc(bsz/8);
	memset(f.buf, 0, bsz/8);
	return f;
}

/* Add elm into the given bloom filter 
 * We use a specific bit-ordering convention for the bloom filter implemention.
   Specifically, you are expected to use the character array in big-endian format. As an example,
   To set the 9-th bit of the bitmap to be "1", you should set the left-most
   bit of the second character in the character array to be "1".
*/

void
bloom_add(bloom_filter f,
	long long elm /* the element to be added (a RK hash value) */)
{
	int i;
	for(i=0; i<BLOOM_HASH_NUM; i++){
		//Use f hash functions
		int pos = hash_i(i, elm) % f.bsz;
		//to map v into f positions in the bitmap
		int byte_pos = pos/8;
		int bit_pos = pos%8;
		//and set each position to one.
		f.buf[byte_pos] = f.buf[byte_pos] | (128 >> (bit_pos-1) ) ;
	}
}

/* Query if elm is a probably in the given bloom filter */ 
int
bloom_query(bloom_filter f,
	long long elm /* the query element (a RK hash value) */ )
{
	//Create a bloom filter for the test and fill it.
	bloom_filter test = bloom_init(f.bsz);
	bloom_add(test, elm);
	int i;
	//For each char in the filters
	for(i=0; i<f.bsz; i++){
		//Find the bitwise and of both. If the query element is in the filter, this should be equal to the test filter.
		char n = test.buf[i]&f.buf[i];
		//If not,
		if (n != test.buf[i]){
			//return 0.
			return 0;
		}
	}
	//If all the chars match in that way, though, return 1.
	return 1;
}

void 
bloom_free(bloom_filter *f)
{
	free(f->buf);
	f->buf = (char *)0;
       	f->bsz = 0;
}


/* print out the first count bits in the bloom filter, 
 * where count is the given function argument */
void
bloom_print(bloom_filter f,
            int count     /* number of bits to display*/ )
{
	assert(count % 8 == 0);
	int i;
	for(i=0; i< (f.bsz>>3) && i < (count>>3); i++) {
		printf("%02x ", (unsigned char)(f.buf[i]));
	}
	printf("\n");
	return;
}