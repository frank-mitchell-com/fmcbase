# Makefile for Frank Mitchell's C library

CC=clang
#CC=tcc
CFLAGS=-c -Wall
LFLAGS=-L. -lfmc -lm

LIB=libfmc.a

HEADERS=ctable.h
OBJECTS=ctable.o
TESTS=t_example.run

all: $(LIB) test

test: $(TESTS)

%.run: %.c $(LIB) $(HEADERS)
	$(CC) $(LFLAGS) -o $@ $<
	./$@

$(LIB): $(OBJECTS)
	ar -rc $(LIB) $(OBJECTS)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $<

.PHONY: clean test

clean:
	rm -rf *.o *.a *.run

