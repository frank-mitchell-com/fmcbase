/*
 * Copyright 2023 Frank Mitchell
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef CSTRTBL_H_INCLUDED
#define CSTRTBL_H_INCLUDED

/** @file */

#include <stddef.h>   /* defines size_t */
#include <stdbool.h>  /* defines bool */
#include <stdint.h>   /* defines {,u}intN_t */

/**
 * A type for a null-terminated C-style string in ASCII or UTF-8.
 * Embedded nulls must be encoded as two-byte UTF-8 characters.
 */
typedef const char* cstring_t;

/**
 * An opaque type for a mapping from C strings to void pointers.
 * The table makes copies of all string keys, and deletes them when done.
 * The client program must free all non-constant strings used to set
 * and get entries *and* the referents of pointers placed in the table.
 */
typedef struct C_String_Table C_String_Table;

/**
 * An opaque type for string table iterator instances.
 */
typedef struct C_String_Table_Iterator C_String_Table_Iterator;

/*
 * A prototype for functions to hash c strings.
 */
typedef uint64_t (*C_String_Hash)(cstring_t ptr);

/**
 * Creates a new string table with at least `minsz` capacity.
 */
void C_String_Table_new(C_String_Table* *tptr, size_t minsz);

/**
 * The number of entries in `t`.
 */
size_t C_String_Table_size(C_String_Table* t);

/**
 * Define a hash function for all the string keys.
 */
void C_String_Table_define_hash_function(C_String_Table* t, C_String_Hash);

/**
 * Return the pointer value for `key`, or NULL if none found.
 * If `key` is null, this always returns NULL, since a key cannot be null.
 */
const void* C_String_Table_get(C_String_Table* t, cstring_t key);

/**
 * Whether `t` contains an entry for `key`.
 * If `key` is null, this always returns false, since a key cannot be null.
 * Returns false if the key was not found.
 */
bool C_String_Table_has(C_String_Table* t, cstring_t key);

/**
 * Put `value` into an entry for `key`.
 * If `key` is null, this function fails, since a key cannot be null.
 * If `value` is null, any existing entry will be removed.
 * The previous value if any is placed in `*oldvalp` if given.
 * Returns false only if the operation could not be completed for some reason.
 */
bool C_String_Table_put(C_String_Table* t, cstring_t key, const void* value, const void* *oldvalp);

/**
 * Remove the entry for `key`.
 * The previous value if any is placed in `*oldvalp` if given.
 * Returns false if the operation could not be completed for some reason.
 */
bool C_String_Table_remove(C_String_Table* t, cstring_t key, const void* *oldvalp);

/**
 * Deletes the table and all memory it allocated.
 */
void C_String_Table_free(C_String_Table* *tptr);


#endif // CSTRTBL_H_INCLUDED

