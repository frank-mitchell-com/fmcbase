# `libfmcbase`: A Guide for the Perplexed #

## What is this?

This is a small library of utility "pseudo-classes": tables, sets, strings,
reference counting APIs, and other basic data structures and utilities.

Since I spent a long time working with Objective-C and Java, I think in
classes, so nearly every module is a set of functions built around a datatype.


## *Why* is this?

C was the first language I ever got paid money writing. With a lull in
my career I wanted to see if I could still write code in the oldest and
most challenging language in my portfolio.  (Fortran or Lisp might have been
challenges of a different kind, but I had neither formulas to translate nor
lists to process, so I decided on C.)

No automatic memory management, no real type safety, segfaults every time 
I step even slightly out of bounds ... definitely a challenge.


## What have you done?

All header files have (incomplete) documentation.  I put documentation there
instead of source files so that even when distributed as libraries some kind
of documentation would always be available.

See also the `doc/` directory for a brief overview of the code and my
coding standards and conventions.

### Building the Software

If you have GNU make simply run the Makefile:

```sh
make
```

The default target builds a static library and runs unit tests.
As of this writing I've yet to test the "install" target.

On some platforms `libiconv.*` is part of the standard libraries; on others
it's a separate library. If the code won't complile because it's looking for
iconv, edit the Makefile from this:

```makefile
LICONV=
#LICONV=-liconv
```

to this:

```makefile
# LICONV=
LICONV=-liconv
 
```

I've only built this on Linux Mint and UCRT64 / MSYS2 (Windows 10) so far.
Please let me know (or send me a patch!) if you need to modify the code for
other systems, e.g. Windows MVSC, Mac, another Unix, or embedded systems (?!?).
