# Makefile for Frank Mitchell's C library

CC=gcc
#CC=clang

LICONV=
#LICONV=-liconv

LIBNAME=fmcbase

SRCDIR=.
TESTDIR=test

DESTDIR=/usr/local
DESTHDR=$(DESTDIR)/include/$(LIBNAME)
DESTLIB=$(DESTIB)/lib

LIB=$(SRCDIR)/lib$(LIBNAME).a
SHLIB=$(SRCDIR)/lib$(LIBNAME).so

CFLAGS=-g -Wall -fPIC
IFLAGS= -I $(SRCDIR) -I $(TESTDIR)
LFLAGS=-L$(SRCDIR) -l$(LIBNAME) $(LICONV) -lm

HEADERS=$(wildcard $(SRCDIR)/*.h)
OBJECTS=$(patsubst %.c,%.o,$(wildcard $(SRCDIR)/*.c))
TESTS=$(patsubst %.c,%-test,$(wildcard $(TESTDIR)/*.c))

.PHONY: all clean test install posix

all: $(LIB) test

posix: all $(SHLIB)

test: $(TESTS)

%-test: %.c $(LIB) $(HEADERS)
	$(CC) -static -g -O0 $(IFLAGS) -o $@ $< $(LFLAGS)
	./$@

$(SHLIB): $(OBJECTS)
	$(CC) -shared -Wl,-soname,$(SHLIB) -o $(SHLIB) $^

$(LIB): $(OBJECTS)
	ar rcs $(LIB) $^

%.o: %.c $(HEADERS)
	$(CC) -c $(CFLAGS) -o $@ $<

install: $(LIB) $(SHLIB)
	install -p -t $(DESTLIB) $(LIB) $(SHLIB)
	install -p -t $(DESTHDR) $(HEADERS)

clean:
	rm -rf $(OBJECTS) $(LIB) $(SHLIB) $(TESTS)

