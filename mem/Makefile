CFLAGS= -g -O2

all:test
main:
	gcc $(CFLAGS) -c -o main.o      main.c
loongson3-memcpy:
	gcc $(CFLAGS) -c -o loongson3-memcpy.o loongson3-memcpy.S
	gcc $(CFLAGS) -c -o ls3-memcpy.o ls3-memcpy.S
	gcc $(CFLAGS) -c -o  memcpy.o memcpy.c
test: main loongson3-memcpy
	gcc $(CFLAGS) -o test main.o loongson3-memcpy.o ls3-memcpy.o memcpy.o 

clean:
	rm -rf *.o test
