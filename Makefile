# Makefile for Frank Mitchell's C library

CC=gcc
#CC=clang

LIBNAME=ctable

SRCDIR=.
OBJDIR=.
LIBDIR=.
TESTDIR=test

LIB=$(LIBDIR)/lib$(LIBNAME).a
SHLIB=$(LIBDIR)/lib$(LIBNAME).so

CFLAGS=-g -Wall -fPIC
LFLAGS=-L$(LIBDIR) -l$(LIBNAME) -lm

HEADERS=$(wildcard $(SRCDIR)/*.h)
OBJECTS=$(patsubst %.c,%.o,$(wildcard $(SRCDIR)/*.c))
TESTS=$(patsubst %.c,%.run,$(wildcard $(TESTDIR)/*.c))

.PHONY: all clean test runtest

all: $(LIB) $(SHLIB) test

test: $(TESTS)

%.run: %.c $(LIB) $(HEADERS)
	$(CC) -static -g -O0 -I $(SRCDIR) -I $(TESTDIR) -o $@ $< $(LFLAGS)
	./$@

$(SHLIB): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(SHLIB) -o $(SHLIB) $<

$(LIB): $(OBJECTS)
	ar rcs $(LIB) $^ 

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -rf $(OBJECTS) $(LIB) $(SHLIB) $(TESTS)


