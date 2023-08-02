# `libctable`: A Guide for the Perplexed #

## What is this?

This is a small library of utility "pseudo-classes": tables, sets, strings,
reference-counting, and other basic utilities.

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

### Building the Software

If you have GNU make simply run the Makefile:

```sh
make
```

The default target builds a static library and runs unit tests.
As of this writing I've yet to add an "install" target.

I've only built this on my Linux system so far.  Please let me know (or send
me a patch!) if you need to modify the code for other systems, e.g. Windows,
Mac, another Unix, or embedded systems (?!?).

### Naming Conventions

Each pseudo-class is defined by an opaque type.  All functions creating
and/or manipulating this type have the type name as a prefix.  The general
pattern is "*type*`_`*fcn*" where *type* is always in upper snake case
and *fcn* in lower snake case.  E.g. `C_Ref_Table_size` performs the
`size` function on `C_Ref_Table*`.

### Coding Conventions

Generally I use the Java style of braces.

The only time I omit braces is when I can write a single statement on the 
same line, e.g. `if (!ptr) return;`.  That's probably a bad habit.

For my own sanity when I use a pointer as part of the type, I put the `*`
next to the type, e.g. `const char* s`.  On the other hand, when I use
a pointer to indicate a reference, usually to write to that reference,
I put it next to the variable, sometimes in parentheses, e.g `size_t *szptr`.
When I need the dreaded double pointer, I put the stars on opposite ends,
e.g. `const char* *sptr` says I'm going to write a C string into `*sptr`.
(Sometimes I forget this convention. Sorry.)

Any function that creates a new object, frees an object, or otherwise hands
ownership of a chunk of memory to the caller uses a pointer to a pointer
rather than returning the pointer.  This allays my paranoia about a careless
caller simply dropping memory; at least they will have put it in a variable
at some point.


### A Quick Guide to the Pseudo-Clasees

Below is simply a brief guide to the pseudo-classes in the library

#### `C_Any`

*Files:* crefcnt.[ch]

A notional root pseudo-class for reference counting any object registered
with `C_Ref_Count`.


#### `C_Conv`

*Files:* cconv.[ch]

Functions to convert characters from one encoding to another.  The most general
one, `C_Conv_transcode`, is simply a wrapper for 
[iconv](https://www.gnu.org/software/libiconv/).  The rest which translate
between UTF-8, UTF-16, and UTF-32 were hand-coded by me.


#### `C_Ref_Count`

*Files:* crefcnt.[ch]

A global reference counter using [C-Table](#ctable) to maintain[^r] thread-safe 
reference counts for all "listed" entities based on their pointer address
alone.  The "list" prevents users from accidentally reference-counting any
random memory address, which might chew up lots of internal memory.

Naturally it would break if C had copying garbage collection.

[^r]: A single reference table interacts more efficiently with hardware caching
than reference counts in each object.  It's a matter of pre-fetching bits of
a dozen different data structures vs. a single hashtable.


#### `C_Ref_Set`

*Files:* crefset.[ch]

A set for unique pointers.  Unlike [C-Table](#ctable) below it uses open
addressing, so it's about as memory-efficient as possible.


#### `C_Symbol`

*Files:* csymbol.[ch]

A global, (hopefully) thread-safe collection of interned strings modeled
on JavaScript's `Symbol` type.


#### `C_Table` {#ctable}

*Files:* ctable.[ch]

A *very* general hash table implementation using closed addressing, linked
lists, and a `C_Userdata` structure for keys and values.  Both keys and
values may be plain pointers, memory blocks (e.g. strings) hashed by value
and managed by the table, both with user-defined integer "tags".  Tags help
user-defined hashing, equality, copying, and freeing functions perform
correct deep copies, efficient memory management, and more accurate hashing
and comparison of keys.


#### `C_Userdata`

A container for keys and values used by [C-Table](#ctable).


#### LOCK / RWLOCK

*File:* cthread.h

Simple macros for the mutexes and read/write locks used in the implementation.
If someone wanted to port this library to Windows, they'd simply have to
redefine the macros to use Win32 locking functions.


#### `U_Char_Buffer`

*Files:* ucharbuf.[ch]

An automatically resizing character buffer to produce [U-Strings](#ustring).


#### `U_String` {#ustring}

*Files:* ustring.[ch]

A pseudo-class of immutable strings.  Rather than create a kitchen-sink
interface like many string implementations, I decided for a minimal interface
with powerful operations like slicing and joining.  Supporting functions and
pseudo-classes can handle the rest.


#### `U_String_Array`

*Files:* ustrarr.[ch]

An automatically resizing array of [U-Strings](#ustring) to make concatenating
them more efficient.


### Other Files

- **libctable.dox**: a [Doxygen](https://www.doxygen.nl/) configuration file 
  for HTML documentation.

- **Makefile**: a basic GNU `make` file 

- **README.md**: this file.

- **test/**: a directory of unit tests using 
  [minctest](https://github.com/codeplea/minctest).
  Despite using minimalist macros with some possible side effects,
  with a little extra infrastructure it's as good as any other unit test
  framework.

- **TODO.md**: a long list of things undone and issues to address.

