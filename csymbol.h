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

#ifndef CSYMBOL_H_INCLUDED
#define CSYMBOL_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/**
 * A unique value sometimes tied to a string value.
 * The mapping to strings, if any, resides in a thread-safe global hashtable.
 */
typedef struct C_Symbol C_Symbol;

/**
 * Determines whether `p` is a C_Symbol.
 * The implementation checks the value of `p` instead of dereferencing it,
 * in case it points to an invalid memory location.
 */
bool is_C_Symbol(void* p);

/**
 * Create a new Symbol value unique in the current instance of the host program.
 * The value isn't guaranteed to be unique across all parallel or future
 * instances in memory.
 * The Symbol has no corresponding string value.
 */
void C_Symbol_new(C_Symbol* *symptr);

/**
 * Provide a Symbol value unique in current memory, indexed by `cstr`.
 * Since `cstr` is a C string, it cannot contain embedded nulls.
 * By convention, symbols don't contain whitespace or non-printable characters,
 * but this is not a hard and fast rule.
 * Returns whether the symbol was recently allocated.
 */
bool C_Symbol_for_cstring(C_Symbol* *symptr, const char* cstr);

/**
 * Provide a Symbol value unique in current memory, 
 * indexed by a UTF-8 string of length 'len' starting at `uptr`.
 * The string may contain embedded nulls.
 * By convention, symbols don't contain whitespace or non-printable characters,
 * but this is not a hard and fast rule.
 * Returns whether the symbol was recently allocated.
 */
bool C_Symbol_for_utf8_string(C_Symbol* *symptr, size_t len, const uint8_t* uptr);

/**
 * The number of references to this symbol, assuming proper reference counting
 * discipline.
 */
int C_Symbol_references(C_Symbol* sym);

/**
 * Increments the reference count to `sym`.
 * Returns the argument, for convenience.
 */
C_Symbol* C_Symbol_retain(C_Symbol* sym);

/**
 * Decrements the reference count to the symbol and
 * sets the contents of `*symptr` to NULL.
 */
void C_Symbol_release(C_Symbol* *symptr);

/**
 * Notify the system that the caller will replace the reference contained
 * in `*lvalptr` with `value`.  This will adjust reference counts accordingly.
 * Returns `value` for convenience.
 */
C_Symbol* C_Symbol_set(C_Symbol* *lvalptr, C_Symbol* value);

/**
 * The value of the string used to create the symbol.
 * This is a copy of the string passed into `C_Symbol_for_utf8_string`
 * or `C_Symbol_for_cstring`.
 * Returns "" if the symbol was created by `C_Symbol_new`,
 * or NULL if the symbol value is invalid (e.g. another type of object).
 * The string may contain embedded nulls, so
 * `*lenptr` will indicate the strings total length.
 */
const uint8_t* C_Symbol_as_utf8_string(C_Symbol* sym, size_t *lenptr);

/**
 * Copy the first `len` bytes of the string used to create `sym` into `buf`.
 * This is a copy of the string passed into `C_Symbol_for_cstring`
 * or `C_Symbol_for_utf8_string` with embedded nulls replaced with '\xC0\x80',
 * The return value will be the number of UTF-8 bytes written to `buf`;
 * if 0 the symbol has no corresponding string value,
 * and if negative `sym` is not a C_Symbol.
 */
ssize_t C_Symbol_as_cstring(C_Symbol* sym, size_t len, char* buf);

#endif // CSYMBOL_H_INCLUDED
