//Implement a C function called ex3 such that it achieves 
//the same functionality as the machine code of objs/ex3_sol.o
//Note: you need to figure out ex3()'s function signature yourself; 
//the signature is not void ex3()
  
#include <assert.h>

int
ex3(char* str1, char* str2, int length){
	int i;
	for(i=0; i<length; i++){
		char temp = str1[i];
		str1[i] = str2[i];
		str2[i] = temp;
	}
	return i;
}
