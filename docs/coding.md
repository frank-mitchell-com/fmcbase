# Coding Conventions

## Brace Style

Generally I use the Java style of braces.

The only time I omit braces is when I can write a single statement on the 
same line, e.g. `if (!ptr) return;`.  That's probably a bad habit.

## Naming

Each pseudo-class is defined by an opaque type.  All functions creating
and/or manipulating this type have the type name as a prefix.  The general
pattern is "*type*`_`*fcn*" where *type* is always in upper snake case
and *fcn* in lower snake case.  E.g. `C_Ref_Table_size` performs the
`size` function on `C_Ref_Table*`.

`C_Conv` is a collection of contextless character conversion routines so 
it has no associated opaque type.

## Pointers

For my own sanity when I use a pointer as part of the type, I put the `*`
next to the type, e.g. `const char* s`.  On the other hand, when I use
a pointer to indicate a reference, usually to write to that reference,
I put it next to the variable, sometimes in parentheses, e.g `size_t *szptr`.
When I need the dreaded double pointer, I put the stars on opposite ends,
e.g. `const char* *sptr` says I'm going to write a C string into `*sptr`.
(Sometimes I forget this convention. Sorry.)

### Returning Results vs. Setting Pointers

Any function that creates a new object, frees an object, or otherwise hands
ownership of a chunk of memory to the caller uses a pointer to a pointer
rather than returning the pointer.  This allays my paranoia about a careless
caller simply dropping memory; at least they will have put it in a variable
at some point.


