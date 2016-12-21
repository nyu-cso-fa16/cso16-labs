//Implement a C function called ex1 such that it achieves 
//the same functionality as the machine code of objs/ex1_sol.o
//Note: you need to figure out ex1()'s function signature yourself; 
//the signature is not void ex1()
  
#include <assert.h>

int
ex1(long x, long y, long z){//This program checks to see if the sum of two numbers equals a third.
	return x+y == z;
}
