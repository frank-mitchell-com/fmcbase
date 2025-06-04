# TODO

## PENDING FINAL DESIGN

- `C_Port`: portable I/O wrapper.

- `C_Xalloc`: customizable allocator in `C_Ustring`.

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

- `C_Ustring`
   - slice...
        - negative indices
   - join...

- `C_Uchar_Buffer`
   - all functions
   - reference counting (`C_Ref_Count`)

## PENDING TESTING

### Functionality

- `Cthreads`
   - verify macros compile and work
   - build a test program with them
   - can we disable them for single threaded? (no)
   - should this maybe be an emulation library?

- `C_Table`
   - custom hash function
   - custom equal, copy, delete
   - look for memory leaks

- `C_Uchar_Buffer`
   - all functions
   - reference counting (`C_Ref_Count`)

- `C_Ustring`
   - all unimplemented
   - new-ascii 
        - ascii
        - non-ascii (locale?)

- `C_Ref_Count`
   - When is the object itself actually freed?
     When the on-zero function is called.
     So rename it from `C_Ref_Count_on_zero` to `C_Ref_Count_on_free`.
   - Will this deadlock when the released object releases its
     own dependents?
   - Add "cleaners" (which hold state that needs cleaning up).

### Robustness

- Error codes in error conditions
  - Documented
  - Undocumented

- Proper mutex usage (no deadlocks or corrupted data)
  - Update `C_Ref_Count` simultaneously in multiple threads.
  - Create multiple `C_Symbol`s in multiple threads.
  - Allocate `C_Ustring`s in multiple threads.
  - Get the same non-UTF `C_Ustring`'s UTF data in multiple threads.
  - Thread safety of `C_Uchar_Buffer`?
  - Thread safety of `C_Ustring_Array`?

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
  - [OpenMP](https://www.openmp.org/)
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

