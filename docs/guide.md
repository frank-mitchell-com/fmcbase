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

