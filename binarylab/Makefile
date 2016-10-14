OBJS := ex1.o ex2.o ex3.o ex4.o ex5.o
SOLOBJS := objs/ex1_sol.o objs/ex2_sol.o objs/ex3_sol.o objs/ex4_sol.o objs/ex5_sol.o
CFLAGS := -c -Og -g -std=gnu99

all : tester tester-sol
handin :
	tar -czvf lab3.tar.gz *.c *.h Makefile

%.o : %.c
	gcc $(CFLAGS) $*.c

tester : $(OBJS) objs/tester.o
	gcc $^ -o $@

tester-sol : $(SOLOBJS) objs/tester.o
	gcc $^ -o $@

clean :
	rm $(OBJS) tester tester-sol

binarylab.tgz :
	tar czvf binarylab.tgz *.c *.h Makefile objs/*.o
