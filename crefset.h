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

#ifndef CREFSET_H_INCLUDED
#define CREFSET_H_INCLUDED

/** @file */

#include <stddef.h>   /* defines size_t */
#include <stdbool.h>  /* defines bool */

/**
 *
 */
typedef struct C_Ref_Set C_Ref_Set;

/**
 *
 */
typedef struct C_Ref_Set_Iterator C_Ref_Set_Iterator;

/**
 *
 */
void C_Ref_Set_new(C_Ref_Set* *tptr, size_t minsz);

/**
 *
 */
size_t C_Ref_Set_size(C_Ref_Set* t);

/**
 * Adds a deep copy of `value` into a new entry for `key` if none exists.
 * Returns false and does nothing if an entry for `key` already exists.
 */
bool C_Ref_Set_add(C_Ref_Set* t, const void* key);

/**
 * Whether `t` contains an entry for `key`.
 * Returns false if the key was not found.
 */
bool C_Ref_Set_has(C_Ref_Set* t, const void* key);

/**
 * Remove the entry for `key`.
 * Returns false if the operation could not be completed for some reason.
 */
bool C_Ref_Set_remove(C_Ref_Set* t, const void* key);

/**
 * Deletes the table and all memory it allocated.
 */
void C_Ref_Set_free(C_Ref_Set* *tptr);

/* -------------------- Iterator Functions ------------------------- */

void C_Ref_Set_new_iterator(C_Ref_Set* t, C_Ref_Set_Iterator* *iptr);

bool C_Ref_Set_Iterator_has_next(C_Ref_Set_Iterator* i);

void C_Ref_Set_Iterator_next(C_Ref_Set_Iterator* i);

const void* C_Ref_Set_Iterator_current(C_Ref_Set_Iterator* i);

bool C_Ref_Set_Iterator_free(C_Ref_Set_Iterator* *iptr);

#endif // CREFSET_H_INCLUDED

