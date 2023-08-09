# Makefile for Frank Mitchell's C library

CC=gcc
#CC=clang

LIBNAME=ctable

SRCDIR=.
TESTDIR=test

DESTDIR=/usr/local
DESTHDR=$(DESTDIR)/include/$(LIBNAME)
DESTLIB=$(DESTIB)/lib

LIB=$(SRCDIR)/lib$(LIBNAME).a
SHLIB=$(SRCDIR)/lib$(LIBNAME).so

CFLAGS=-g -Wall -fPIC
IFLAGS= -I $(SRCDIR) -I $(TESTDIR)
LFLAGS=-L$(SRCDIR) -l$(LIBNAME) -lm

HEADERS=$(wildcard $(SRCDIR)/*.h)
OBJECTS=$(patsubst %.c,%.o,$(wildcard $(SRCDIR)/*.c))
TESTS=$(patsubst %.c,%.run,$(wildcard $(TESTDIR)/*.c))

.PHONY: all clean test install

all: $(LIB) $(SHLIB) test

test: $(TESTS)

%.run: %.c $(LIB) $(HEADERS)
	$(CC) -static -g -O0 $(IFLAGS) -o $@ $< $(LFLAGS)
	./$@

$(SHLIB): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(SHLIB) -o $(SHLIB) $<

$(LIB): $(OBJECTS)
	ar rcs $(LIB) $^ 

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

install: $(LIB) $(SHLIB)
	install -p -t $(DESTLIB) $(LIB) $(SHLIB)
	install -p -t $(DESTHDR) $(HEADERS)

clean:
	rm -rf $(OBJECTS) $(LIB) $(SHLIB) $(TESTS)

