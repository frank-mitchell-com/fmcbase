# Makefile for Frank Mitchell's C library

#CC=gcc
CC=clang
LIBNAME=ctable
CFLAGS=-Wall -fPIC
LFLAGS=-L. -l$(LIBNAME) -lm

LIB=lib$(LIBNAME).a
SHLIB=lib$(LIBNAME).so

HEADERS=ctable.h csymbol.h cconv.h
OBJECTS=ctable.o csymbol.o cconv.o
TESTS=t_table.run

.PHONY: all clean test

all: $(LIB) test

test: $(TESTS)

%.run: %.c $(LIB) $(SHLIB) $(HEADERS)
	$(CC) -static -g -O0 -o $@ $< $(LFLAGS)

$(SHLIB): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(SHLIB) -o $(SHLIB) $<

$(LIB): $(OBJECTS)
	ar rcs $(LIB) $^ 

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) $<

clean:
	rm -rf *.o *.a *.so *.run

