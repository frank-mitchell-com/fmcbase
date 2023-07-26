## PENDING FINAL DESIGN

- common header
- reference table (read write lock)
- string table (read write lock)
- C_Conv
- C_Port
- U_String

### TypeLib / ExpObj

- E_Any
- E_String (<- U_String)
- M_Pool
- M_String (<- U_String)

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

## PENDING IMPLEMENTATION

- C_Table

## PENDING TESTING

- C_Symbol
- C_Table

## COMPILATION

- Other Thread Models (compilation switches)
  - no threads
  - Windows Threads
  - Netscape Common Runtime
  - Apache Common Runtime

- Namespaces?

- Common API Marker
  - extern or DLL stuff

- C_Writer, C_Reader prototypes?

### TypeLib / ExpObj

- datatypes
- unignorable error codes? -> exceptions?
- 
