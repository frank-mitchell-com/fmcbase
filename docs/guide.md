# A Quick Guide to Project Files

## Source Files

### Pseudo-Classes

Below is simply a brief guide to the pseudo-classes in the library

#### `C_Any`

*Files:* refcount.[ch]

A notional root pseudo-class for reference counting any object registered
with `C_Ref_Count`.


#### `C_Conv`

*Files:* convert.[ch]

Functions to convert characters from one encoding to another.  The most general
one, `C_Conv_transcode`, is simply a wrapper for 
[iconv](https://www.gnu.org/software/libiconv/).  The rest which translate
between UTF-8, UTF-16, and UTF-32 were hand-coded by me.


#### `C_Ref_Count`

*Files:* refcount.[ch]

A global reference counter using [C-Table](#ctable) to maintain[^r] thread-safe 
reference counts for all "listed" entities based on their pointer address
alone.  The "list" prevents users from accidentally reference-counting any
random memory address, which might chew up lots of internal memory.

Naturally it would break if C had copying garbage collection.

[^r]: A single reference table interacts more efficiently with hardware caching
than reference counts in each object.  It's a matter of pre-fetching bits of
a dozen different data structures vs. a single hashtable.


#### `C_Ref_Set`

*Files:* refset.[ch]

A set for unique pointers.  Unlike [Table](#table) below it uses open
addressing, so it's about as memory-efficient as possible.


#### `C_Ref_Table`

*Files:* reftable.[ch]

A map from one pointer to another.  As such the client has responsiblility
for freeing both keys and values.

Unlike [Table](#table) below it uses open addressing, 
so it's about as memory-efficient as possible.


#### `C_String_Table`

*Files:* strtable.[ch]

A map from a "string" -- really any finite array of bytes -- to a `void*`.
The map makes a copy of the key, but simply stores the value by reference.
As such the client has responsiblility for freeing values.

Under the hood it uses [Table](#table).  Ideally I would reimplement it
so that clients could redefine the hash and equality functions freely,
perhaps still using closed addressing and linked lists while saving three
words (24 bytes) per entry over Table (two tags and the value length).
However I simply didn't get to it.


#### `C_Symbol`

*Files:* symbol.[ch]

A global, (hopefully) thread-safe collection of interned strings modeled
on JavaScript's `Symbol` type.


#### `C_Table` {#table}

*Files:* table.[ch]

A *very* general hash table implementation using closed addressing, linked
lists, and a `C_Userdata` structure for keys and values.  Both keys and
values may be plain pointers, memory blocks (e.g. strings) hashed by value
and managed by the table, both with user-defined integer "tags".  Tags help
user-defined hashing, equality, copying, and freeing functions perform
correct deep copies, efficient memory management, and more accurate hashing
and comparison of keys.


#### `C_Userdata`

*Files:* table.[ch]

A container for keys and values used by [Table](#table).


#### `C_Uchar_Buffer`

*Files:* wcharbuf.[ch]

An automatically resizing character buffer to produce [ustrings](#ustring).


#### `C_Ustring` {#ustring}

*Files:* ustring.[ch]

A pseudo-class of immutable strings.  Rather than create a kitchen-sink
interface like many string implementations, I decided for a minimal interface
with powerful operations like slicing and joining.  Supporting functions and
pseudo-classes can handle the rest.


### Other Types

#### `octet_t`, `utf8_t`, `utf16_t`, `utf32_t`

*File:* common.h


#### LOCK / RWLOCK

*File:* thread.h

Simple macros for the mutexes and read/write locks used in the implementation.
If someone wanted to port this library to Windows, they'd simply have to
redefine the macros to use Win32 locking functions.


## Other Files

- **Doxyfile**: a [Doxygen](https://www.doxygen.nl/) configuration file 
  for HTML documentation.

- **Makefile**: a basic GNU `make` file 

- **README.md**: this file.

- **test/**: a directory of unit tests using 
  [minctest](https://github.com/codeplea/minctest).
  Despite using minimalist macros with some possible side effects,
  with a little extra infrastructure it's as good as any other unit test
  framework.

- **TODO.md**: a long list of things undone and issues to address.

