# TODO

## PENDING FINAL DESIGN

- common header
- `C_Port`
- `U_Alloc`
  - customizable allocator in `U_String`

## PENDING IMPLEMENTATION

- `C_Conv`
  - Invalid encoding detection & signalling
  - Check endianness for UTF-16 and UTF-32

- `C_Ref_Set`
   - collision metrics

- `C_Ref_Table`
   - collision metrics

- `C_String_Table`
   - collision metrics

- `C_Table`
   - collision metrics

- `U_String`
   - nearly all functions
   - "compressed" strings of 1-byte and 2-byte characters.
   - "small" strings of 0 or 1 character (`wchar_t`)

- `U_Char_Buffer`
   - all functions
   - reference counting (`C_Ref_Count`)

- `U_String_Array`
   - all functions
   - reference counting (`C_Ref_Count`)

## PENDING TESTING

### Functionality

- `C_Table`
   - custom hash function
   - custom equal, copy, delete
   - look for memory leaks

- `U_String`
   - all functions
   - conversion to codepoints

### Robustness

- Proper mutex usage (no deadlocks or corrupted data)
  - Update `C_Ref_Count` simultaneously in multiple threads.
  - Create multiple `C_Symbol`s in multiple threads.
  - Allocate `U_String`s in multiple threads.
  - Get the same non-UTF `U_String`'s UTF data in multiple threads.
  - Thread safety of `U_Char_Buffer`?
  - Thread safety of `U_String_Array`?

### Performance

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

- Doxygen documentation of all header files

