#ifndef __BLOOM_H_
#define __BLOOM_H_
/***********************************************************
 File Name: bloom.h
 Description: definition of Bloom filter functions
 **********************************************************/
typedef struct {
	char *buf; /* the bitmap representing the bloom filter*/
	int bsz; /* size of bitmap in bits*/
} bloom_filter;

bloom_filter bloom_init(int bsz);
void bloom_free(bloom_filter *f);

void bloom_add(bloom_filter f, long long elm);
int bloom_query(bloom_filter f, long long elm);

void bloom_print(bloom_filter f, int count);
#endif
