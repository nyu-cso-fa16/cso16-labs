//Implement a C function called ex2 such that it achieves 
//the same functionality as the machine code of objs/ex2_sol.o
//Note: you need to figure out ex2()'s function signature yourself; 
//the signature is not void ex2()
  
#include <assert.h>

int
ex2(char* array) {
	int a_count = 0;
	int index = 0;
	while(array[index]){
		if(array[index] == 'a')
			a_count++;
		index++;
	}
	return a_count;
}
