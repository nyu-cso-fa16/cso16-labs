all := tester
OBJS:= htable.o rwlock.o testhash.o testrwlock.o tester.o

CC     := gcc
CFLAGS := -g -std=gnu99 -DANSWER=1

%.o : %.c
	$(CC) -c $(CFLAGS) $*.c


tester: $(OBJS)
	$(CC) -o $@ $^ -lpthread -lm 

handin:
	tar czvf lab5.tar.gz Makefile *.c *.h

clean : 
	rm -f tester $(OBJS) lab5.tar.gz
