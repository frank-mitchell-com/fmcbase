# TODO

## PENDING FINAL DESIGN

- common header
- string table (read write lock)
- `C_Port`

## PENDING IMPLEMENTATION

- `C_Conv`
   - Invalid encoding detection & signalling
   - Find *n*th code point in a UTF-8 string.
   - Find *n*th code point after *k*th byte in a UTF-8 string.

- `C_Table`
   - collision metrics

- `C_Ref_Count`
  - "on zero" callback

- `U_String`
   - nearly all functions

- `U_Char_Buffer`
   - all functions
   - reference counting (`C_Ref_Count`)

- `U_String_Array`
   - all functions
   - reference counting (`C_Ref_Count`)

## PENDING TESTING

- `C_Conv`
  - Check endianness of UTF-16 and UTF-32

- `C_Ref_Count` / `C_Any`
  - retain, release, set methods

- `C_Table`
   - custom hash function
   - custom equal, copy, delete
   - look for memory leaks

- `U_String`
   - all functions
   - conversion to codepoints
   - reference counting (`C_Ref_Count`)

- Proper mutex usage (no deadlocks or corrupted data)
  - Update `C_Ref_Count` simultaneously in multiple threads.
  - Create multiple `C_Symbol`s in multiple threads.
  - Get the same non-UTF `U_String`'s UTF data in multiple threads.
  - Thread safety of `U_Char_Buffer`?
  - Thread safety of `U_String_Array`?

- Memory and CPU Performance
  - Add 10000 (or more) elements to a `C_Table` and see if it slows or breaks.
  - Add 10000 (or more) anonymous `C_Symbols` and measure the time to add
    each as a function of number of symbols.
  - Add 10000 (or more) *named* `C_Symbols` and measure the time to add
    each as a function of number of symbols.
  - Refcount 10000 (or more) distinct objects (chunks of memory).
  - Use the collision metrics we eventually build into `C_Table`
    (and `C_Ref_Set` and `C_Ref_Table` and `C_String_Table`)
    to tune the default string and pointer hashing algorithms.
  - Use `getrusage()` to figure out memory usage, esp. of `C_Table`.

## COMPILATION

- Other Thread Libraries (compilation switches)
  - single threaded
  - Windows Threads
  - Netscape Common Runtime
  - Apache Common Runtime

- Namespaces?

- Alternate memory allocation?
  - Thread local
  - "M-Pool" (eventually)

- Common API Marker
  - extern or DLL stuff

## DOCUMENTATION

- Tighter README.md

- Doxygen documentation of all header files

- Design and implementation notes

