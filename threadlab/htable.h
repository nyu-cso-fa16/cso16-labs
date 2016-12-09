#ifndef HTABLE_H
#define HTABLE_H

#include "rwlock.h"

#define BIG_PRIME 1560007

//node is the type of a linked list node type. Each hash table entry corresponds to a linked list containing key/value tuples that are hashed to the same slot.
typedef struct node {
	int hashcode;
	char *key;
	void *val;
	struct node *next;
}node;

typedef struct {
	int allow_resize;
	node **store; //to contain an array of linked list heads
	int size; //size of the array
}htable;

void htable_init(htable *ht, int sz, int allow_resize);
void htable_destroy(htable *ht);
int htable_size(htable *ht);
int htable_insert(htable *ht, char *key, void *val);
void *htable_lookup(htable *ht, char *key);

#endif
