#include <stdlib.h>
#include <assert.h>
#include <strings.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

#include "htable.h"

#define MAX_COLLISION 10
/* htable implements a hash table that handles collisions by chaining.
 * It contains an array, where each slot stores the head of a singly-linked list.
 * if the length of some chain is found to be longer than MAX_COLLISION after an insertion, the 
 * htable is resized by doubling the array size. 
 */
// calculate modulo addition
int
madd(int a, int b)
{
	return ((a+b)>BIG_PRIME?(a+b-BIG_PRIME):(a+b));
}

// calculate modulo multiplication
int
mmul(int a, int b)
{
	return ((a*b) % BIG_PRIME);
}


//hashcode returns the hashcode for a C string
int
hashcode(char *s) {
	int p = 0;
	int i = 0;
	while (s[i] != '\0') {
		p  = madd(mmul(p, 256), s[i]);
		i++;
	}
	return p;
}

//is_prime returns true if n is a prime number
int
is_prime(int n) {
	if (n <= 3) {
		return 1;
	}
	if ((n % 2) == 0) {
		return 0;
	}
	for (int i = 3; i < n; i+=2) {
		if ((n % i) == 0) {
			return 0;
		}
	}
	return 1;
}

//get_prime returns a prime number larger than min
static int
get_prime(int min) {
	int n = min;
	while (1) {
		if (is_prime(n)) {
			return n;
		}
		n++;	
	}
}

//htable_init returns a new hash table that's been initialized
void
htable_init(htable *ht, int sz, int allow_resize) {
	//initialize hash table with a prime larger than sz entries
	ht->size = get_prime(sz + 1);
	ht->store = (node **)malloc(sizeof(node *)*ht->size);
	ht->allow_resize = allow_resize;
	assert(ht->store);
	bzero(ht->store, sizeof(node *)*ht->size);
}

//htable_size returns the number of slots in the hash table
int
htable_size(htable *ht) {
	int sz = ht->size;
	return sz;
}

void
free_linked_list(node *n) {
	if (n != NULL) {
		free_linked_list(n->next);
		free(n);
	}
}

//htable_destroy destroys the htable, freeing the memory associated with its fields
void
htable_destroy(htable *ht) {
	// need to clean up every single linked list, and each of its nodes
	for (int i = 0; i < ht->size; i++)
		free_linked_list(ht->store[i]);
	free(ht->store);
}

//htable_resize increases the size of an existing htable in order to control
//max number of collsions.
static void
htable_resize(htable *ht) {
	//double the array size
	int new_size = get_prime(2*ht->size);
	node **new_store = (node **)malloc(sizeof(node *)*new_size);
	assert(new_store != NULL);
	bzero(new_store, sizeof(node *)*new_size);

	for (int i = 0; i < ht->size; i++) {
		node *curr = ht->store[i];
		while (curr) {
			node *n = curr;
			curr = curr->next;
			int slot = n->hashcode % new_size;
			//insert at the end of new table's slot
			if (new_store[slot] != NULL) {
				n->next = new_store[slot];
			} else {
				n->next = NULL;
			}
		       	new_store[slot] = n;
		}
	}
	ht->size = new_size;
	free(ht->store);
	ht->store = new_store;

}


//htable_insert inserts the key, val tuple into the htable. If the key already 
//exists, it returns 1 indicating failure.  Otherwise, it inserts the new val and returns 0. 
int
htable_insert(htable *ht, char *key, void *val) {

	int hcode = hashcode(key);	
	//this key/value tuple corresponds to slot "slot"
       	int slot = hcode % ht->size;
	//traverse linked list at slot "slot", insert the new node at the end 
	node *prev = NULL; 
	node *curr = ht->store[slot];
	int collision = 0;
	while (curr) {
		if ((curr->hashcode == hcode ) && (strcmp(curr->key, key) == 0)) {
			return 1; //found an existing key/value tupe with the same key
		}
		prev = curr;
		curr = curr->next;
		collision++;
	}

	//allocate a node to store key/value tuple
	node *n = (node *)malloc(sizeof(node));
	n->hashcode = hcode;
	n->key = key;
	n->val = val;
	n->next = NULL;

	if (prev == NULL) {
		ht->store[slot] = n;
	}else {
		prev->next = n;
	}
	if (ht->allow_resize && collision >= MAX_COLLISION) {
		htable_resize(ht);
	}
	return 0; //success
}

//htable_lookup returns the corresponding val if key exists
//otherwise it returns NULL.
void *
htable_lookup(htable *ht, char *key) {
	int hcode = hashcode(key);
       	int slot = hcode % ht->size;
	node *curr = ht->store[slot];
	while (curr) {
		if ((curr->hashcode == hcode) && (strcmp(curr->key, key) == 0)) {
			return curr->val;
		}
		curr = curr->next;
	}
	return NULL;
}
