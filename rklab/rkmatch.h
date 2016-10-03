#ifndef __RKMATCH_H_
#define __RKMATCH_H_
/***********************************************************
 File Name: rkmatch.h
 Description: definition of Rabin-Karp match functions
 **********************************************************/
long long madd(long long a, long long b);
long long mdel(long long a, long long b);
long long mmul(long long a, long long b);
void read_file(const char *fname, unsigned char **doc, int *doc_len);
int normalize(unsigned char *buf, int len);
int exact_match(const unsigned char *qs, int m, const unsigned char *ts, int n);
int simple_substr_match(const unsigned char *ps, int k, const unsigned char *ts, int n);
long long rkhash_init(const unsigned char *charbuf, int k, long long *h);
long long rkhash_next(long long curr_hash, long long h, unsigned char prev, unsigned char next);
int rabin_karp_match(const unsigned char *ps, int k, const unsigned char *ts, int n);
int rabin_karp_batchmatch(int bsz, int k, const unsigned char *qs, int m, const unsigned char *ts, int n);

extern long long BIG_PRIME;
#endif
