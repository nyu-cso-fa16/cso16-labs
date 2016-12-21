//Implement a C function called ex5 such that it achieves 
//the same functionality as the machine code of objs/ex5_sol.o
//Note: you need to figure out ex5()'s function signature yourself; 
//the signature is not void ex5(node *n). However, as a hint, 
//ex5 does take node * as an argument.
 
#include <assert.h>
#include "ex5.h"

int
ex5(node *n) {//This one checks if a linked list's values are sorted in increasing order.
	if(n){//Checks to make sure that the first node exists.
		node* next = (*n).next;
		while(next){//Increment through each node until you reach the last one
			if((*n).value > (*next).value)//If any of them have a value smaller than the previous one,
				return 0;//return false.
			n = next;
			next = (*next).next;
		}
	}
	return 1;//Otherwise, return true.
}
