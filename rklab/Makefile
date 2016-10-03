ANSWER=0
CFLAGS=-g -std=gnu99

all: rkmatch bloom_test rk_unittest

rkmatch: rkmatch.o bloom.o rkmatch_main.o
	gcc $^ -o $@ -lrt -lm

rk_unittest: rkmatch_test.o rkmatch.o bloom.o
	gcc $^ -o $@ -lrt -lm

bloom_test : bloom_test.o bloom.o
	gcc $^ -o $@ -lm

%.o : %.c
	gcc $(CFLAGS) -DANSWER=$(ANSWER) -c ${<}

clean :
	rm -f *.o rkmatch rk_unittest bloom_test *.dvi *.log *.tar 
