//Implement a C function called ex4 such that it achieves 
//the same functionality as the machine code of objs/ex4_sol.o
//Note: you need to figure out ex4()'s function signature yourself; 
//the signature is not void ex4()
  
#include <assert.h>

long ex4(long rdi){
	if(rdi<=1) return rdi;
	else return ex4(rdi-1)+ex4(rdi-2);
}