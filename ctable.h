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

#ifndef CTABLE_H_INCLUDED
#define CTABLE_H_INCLUDED

/** @file */

#include <stddef.h>   /* defines size_t */
#include <stdbool.h>  /* defines bool */
#include <stdint.h>   /* defines {,u}intN_t */

/**
 *
 */
typedef struct C_Table          C_Table;

/**
 *
 */
typedef struct C_Table_Iterator C_Table_Iterator;

/**
 * The type of `C_Userdata.tag`.
 */
typedef unsigned int tag_t;

/**
 * The default tag for simple memory blocks with no pointers is 0.
 * Your own tags should use something greater than 0.
 */
#define DEFAULT_TAG     0

/**
 * An open structure to hold key and value parameters.
 */
typedef struct C_Userdata {
    /** user-defined tag to identify 'type' */
    tag_t  tag;

    /** size of data at `*ptr` or 0 if the pointer *is* the data */
    size_t len;

    /** pointer to data */
    void*  ptr;
} C_Userdata;

typedef uint64_t (*C_Table_Hash)(const void* ptr, size_t len);

typedef bool (*C_Userdata_Equals)(const C_Userdata* a, const C_Userdata* b);

typedef bool (*C_Userdata_Copy)(C_Userdata* dest, const C_Userdata* src);

typedef void (*C_Userdata_Free)(C_Userdata* a);


void C_Table_new(C_Table* *tptr, size_t minsz);


size_t C_Table_size(C_Table* t);


void C_Table_define_hash_function(C_Table* t, C_Table_Hash);

void C_Table_define_data_equals(C_Table* t, C_Userdata_Equals);

void C_Table_define_data_copy(C_Table* t, C_Userdata_Copy);

void C_Table_define_data_free(C_Table* t, C_Userdata_Free);

/**
 * Adds a deep copy of `value` into a new entry for `key` if none exists.
 * Returns false and does nothing if an entry for `key` already exists.
 */
bool C_Table_add(C_Table* t, const C_Userdata* key, const C_Userdata* value);

/**
 * Get a shallow copy of the entry for `key` into `value`.
 * Returns false if the key was not found.
 */
bool C_Table_get(C_Table* t, const C_Userdata* key, C_Userdata* value);

/**
 * Whether `t` contains an entry for `key`.
 * Returns false if the key was not found.
 */
bool C_Table_has(C_Table* t, const C_Userdata* key);

/**
 * Puts a deep copy of `value` into an entry for `key`.
 * Returns false only if the operation could not be completed for some reason.
 */
bool C_Table_put(C_Table* t, const C_Userdata* key, const C_Userdata* value);

/**
 * Remove the entry for `key`.
 * Returns false if the operation could not be completed for some reason.
 */
bool C_Table_remove(C_Table* t, const C_Userdata* key);

/**
 * Deletes the table and all memory it allocated.
 */
void C_Table_free(C_Table* *tptr);

/* -------------------- Iterator Functions ------------------------- */

void C_Table_new_iterator(C_Table* t, C_Table_Iterator* *iptr);

bool C_Table_Iterator_has_next(C_Table_Iterator* i);

void C_Table_Iterator_next(C_Table_Iterator* i);

bool C_Table_Iterator_current_key(C_Table_Iterator* i, C_Userdata *key);

bool C_Table_Iterator_current_pair(C_Table_Iterator* i, C_Userdata *key, C_Userdata *val);

bool C_Table_Iterator_free(C_Table_Iterator* *iptr);

/* -------------------- Userdata Functions ------------------------- */

bool C_Userdata_is_reference(const C_Userdata* ud);

/** Clears all values of the struct. */
void C_Userdata_clear(C_Userdata* ud, bool iscopy);

/** Sets provided values into struct. */
void C_Userdata_set(C_Userdata* ud, tag_t tag, size_t len, const void* ptr);

/** Sets the null-terminated string into the struct. */
void C_Userdata_set_string(C_Userdata* ud, const char* cstring);

/**
 * Sets the pointer *value* into the struct, without copying.
 */
void C_Userdata_set_pointer(C_Userdata* ud, const void* ref);

/** 
 * If len is nonzero, sets the contents of `ptr` into the struct.
 * Otherwise, sets the pointer *value* of `ptr`.
 */
void C_Userdata_set_value(C_Userdata* ud, const void* ptr, size_t len);

#endif // CTABLE_H_INCLUDED
