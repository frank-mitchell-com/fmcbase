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

- `C_Ref_Table`
   - all functions

- `C_String_Table`
   - all functions

- `U_String`
   - nearly all functions
   - reference counting (`C_Ref_Count`)

- `U_Char_Buffer`
   - reference counting (`C_Ref_Count`)

- `U_String_Array`
   - all functions
   - reference counting (`C_Ref_Count`)

## PENDING TESTING

- `C_Conv`
  - Check endianness of UTF-16 and UTF-32

- `C_Ref_Count`
  - retain, release, set methods

- `C_Symbol`
   - table lock

- `C_Table`
   - custom hash function
   - custom equal, copy, delete
   - look for memory leaks

- `U_String`
   - all functions
   - conversion to codepoints

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

