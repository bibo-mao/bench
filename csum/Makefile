all:
	gcc -O2 -c csum.c
	gcc -O2 -c test.c
	gcc -O2 -c csum_partial.S
	gcc -o test test.o csum.o csum_partial.o
clean:
	rm -rf test *.o

