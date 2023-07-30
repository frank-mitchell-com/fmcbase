# TODO

## PENDING FINAL DESIGN

- common header
- reference table (read write lock)
- string table (read write lock)

## PENDING IMPLEMENTATION

- `C_Conv`
   - UTF-x to y conversion
   - Invalid encoding detection & signalling

- `C_Port`

- `C_Symbol`
   - table lock

- `C_Table`
   - resizing
   - iterator
   - custom hash function
   - custom equal, copy, delete
   - collision metrics
   - look for memory leaks

- `U_String`

- `U_Char_Buffer`

- `U_String_Array`

## PENDING TESTING

- `C_Conv`
  - Check endianness of UTF-16 and UTF-32

- `C_Table`

## COMPILATION

- Other Thread Models (compilation switches)
  - no threads
  - Windows Threads
  - Netscape Common Runtime
  - Apache Common Runtime

- Namespaces?

- Alternate memory allocation?
  - Thread local
  - "M-Pool" (eventually)

- Common API Marker
  - extern or DLL stuff

---

## TYPElIB / EXPOBJ

- datatypes
- unignorable error codes? -> exceptions?

### Pending Final Design

- `E_Any`
- `E_String` (<- `U_String`)
- `M_Pool`
- `M_String` (<- `U_String`)

### Pending Design / XTIDL

- Basic Types
  - any
  - associative array (a la Lua)
  - bitset
  - boolean [IM]
  - character [IM]
  - integer (<- number)
    - smallint [IM]
    - bigint (inf. precision?)
  - number
    - bigdecimal (<>- integer x10^(integer) )
    - float (double+ precision)
    - rational (<>- integer / integer)
  - null [IM]
  - string
    - zero-length
    - single Unicode character
    - utf-8
    - utf-16?
    - utf-32?
    - arbitrary encoding
  - undefined [IM]

- TypeLib / ExpObj Collections:
    - bag
    - deque
    - array
    - list
    - map
    - map, sorted
    - queue

