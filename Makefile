CC=clang
CFLAGS=-O0 -g -std=c89 -D _GNU_SOURCE

atto: atto.o runtime.o

clean:
	rm -f *.o atto
