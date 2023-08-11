# TODO

## PENDING FINAL DESIGN

- `C_Port`: portable I/O wrapper.

- `C_Xalloc`: customizable allocator in `C_Wstring`.

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

- `C_Wstring`
   - each...
   - slice...
        - negative indices
   - join...

- `C_Wchar_Buffer`
   - all functions
   - reference counting (`C_Ref_Count`)

## PENDING TESTING

### Functionality

- `C_Table`
   - custom hash function
   - custom equal, copy, delete
   - look for memory leaks

- `C_Wstring`
   - all unimplemented
   - new-ascii 
        - ascii
        - non-ascii (locale?)
   - "compressed" strings of 1-byte and 2-byte characters.

### Robustness

- Error codes in error conditions
  - Documented
  - Undocumented

- Proper mutex usage (no deadlocks or corrupted data)
  - Update `C_Ref_Count` simultaneously in multiple threads.
  - Create multiple `C_Symbol`s in multiple threads.
  - Allocate `C_Wstring`s in multiple threads.
  - Get the same non-UTF `C_Wstring`'s UTF data in multiple threads.
  - Thread safety of `C_Wchar_Buffer`?
  - Thread safety of `C_Wstring_Array`?

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
  - Check savings of "compressed strings" and "small strings".

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

