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

#ifndef FMC_REFCOUNT_H_INCLUDED
#define FMC_REFCOUNT_H_INCLUDED

#include "common.h"

typedef void (*C_On_Free_Fcn)(void*);

/**
 * The current reference count for `p`.
 * If not listed, defaults to 1.
 * This query is thread-safe.
 */
FMC_API uint32_t C_Ref_Count_refcount(const void* p);

/**
 * Decrement and return the reference count for `p`.
 * This operation is thread-safe.
 */
FMC_API uint32_t C_Ref_Count_decrement(const void* p);

/**
 * Increment and return the reference count for `p`.
 * This operation is thread-safe.
 */
FMC_API uint32_t C_Ref_Count_increment(const void* p);

/**
 * Whether `p` is listed in the global reference table.
 * This query is thread-safe.
 */
FMC_API bool C_Ref_Count_is_listed(const void* p);

/**
 * Add `p` to the global reference table if not already there.
 * This operation is thread-safe.
 */
FMC_API void C_Ref_Count_list(const void* p);

/**
 * Remove `p` from the global reference table.
 * Usually done after deallocating `p`.
 * This operation is thread-safe.
 */
FMC_API void C_Ref_Count_delist(const void* p);

/**
 * Register callback `onfree` for `p` when `p`'s reference count reaches 0.
 * Usually used to free `p`.  Automatically delists `p` before it's called.
 */
FMC_API void C_Ref_Count_on_free(const void* p, C_On_Free_Fcn onfree);

/**
 * If `p` is listed, increment its reference count and return it.
 */
FMC_API const void* C_Any_retain(const void* p);

/**
 * If `*pptr` is listed, decrement its reference count.
 * `*pptr` will be set to NULL.
 * Return whether `*pptr` was listed.
 */
FMC_API bool C_Any_release(const void* *pptr);

/**
 * Assign `value` to `lvalue`, adjusting reference counts accordingly.
 */
FMC_API void C_Any_set(const void* *lvalue, const void* value);

#endif // FMC_REFCOUNT_H_INCLUDED

