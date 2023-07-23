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

#include <math.h>     /* defines float_t, double_t */
#include <stddef.h>   /* defines size_t */
#include <stdbool.h>  /* defines bool */
#include <stdint.h>   /* defines {,u}intN_t */
#include <wchar.h>    /* defines wchar_t */

#ifndef C_Table_Number
#define C_Table_Number double
#endif

typedef uint8_t  utf8_t;
typedef uint16_t utf16_t;
typedef wchar_t  utf32_t;

/**
typedef unsigned char  utf8_t;  // if no <stdint.h>
typedef unsigned short utf16_t; // if no <stdint.h> && sizeof(short) == 2
typedef unsigned int   utf32_t; // if no <wchar.h>  && sizeof(int)   == 4
**/

typedef struct _C_Table          C_Table;
typedef struct _C_Table_Iterator C_Table_Iterator;

typedef unsigned int  tag_t;

typedef struct _ctbl_userdata {
    tag_t   tag;    /* identifies type */
    size_t  len;    /* size of (*ptr) */
    void*   ptr;    /* pointer to data */
} C_Userdata;

typedef enum _C_Table_Any_Type {
    Type_NOT_FOUND  = 0,    /* Indicates a key not found */

    Type_VOIDPTR    = 1,    /* (void *) *not* managed by the table */
    Type_CHARPTR    = 2,    /* (const char*) *not* managed by the table */
    Type_NULL       = 3,    /* indicates a null pointer stored */

    Type_BOOLEAN    = 5,    /* boolean */
    Type_INTEGER    = 6,    /* integer, default size */
    Type_NUMBER     = 7,    /* number, default size, usually floating point */

    Type_STRING_8   = 0xA,  /* 8-bit string with a length */
    Type_STRING_16  = 0xB,  /* 16-bit string with a length */
    Type_STRING_32  = 0xC,  /* 32-bit string with a length */

    /* types from <stdint.h> */
    Type_INT_8      = 0x10,
    Type_UINT_8     = 0x11,
    Type_INT_16     = 0x12,
    Type_UINT_16    = 0x13,
    Type_INT_32     = 0x14,
    Type_UINT_32    = 0x15,
    Type_INT_64     = 0x16,
    Type_UINT_64    = 0x17,
    Type_INT_MAX    = 0x18,
    Type_UINT_MAX   = 0x19,

    /* types from <math.h> */
    Type_FLOAT      = 0x21,
    Type_DOUBLE     = 0x22,

    Type_USERDATA = 0xFF    /* tagged user data managed by the table */

} C_Table_Any_Type;

struct _C_Table_Any {
    C_Table_Any_Type type;
    union {
        /* Type_VOIDPTR */
        const void* vptr;

        /* Type_CHARPTR */
        const char* cptr;

        /* Type_USERDATA */
        C_Userdata ud;
 
        /* Type_BOOLEAN */
        bool b;

        /* Type_INTEGER */
        int i;

        /* Type_NUMBER */
        C_Table_Number num;

        /* Type_STRING_8 */
        struct _string_8 {
            size_t        l;
            const utf8_t* p;
        } s8;

        /* Type_STRING_16 */
        struct _string_16 {
            size_t         l;
            const utf16_t* p;
        } s16;

        /* Type_STRING_32 */
        struct _string_32 {
            size_t         l;
            const utf32_t* p;
        } s32;

        /* Type_{,U}INT_{8,16,32,64,MAX} */
        int8_t      i8;
        uint8_t     u8;
        int16_t     i16;
        uint16_t    u16;
        int32_t     i32;
        uint32_t    u32;
        int64_t     i64;
        uint64_t    u64;
        intmax_t    imax;
        uintmax_t   umax;

        /* Type_FLOAT, Type_DOUBLE */
        float_t     flt;
        double_t    dbl;
    } value;
};

typedef struct _C_Table_Any C_Table_Any;

typedef bool (C_Userdata_Eq)(const C_Userdata* a, const C_Userdata* b);

void C_Table_new(C_Table* *tptr, size_t minsz);

void C_Table_define_userdata_eq(C_Table* t, C_Userdata_Eq);

bool C_Table_get(C_Table* t, const C_Table_Any* key, C_Table_Any* (*valptr));

bool C_Table_get_cptr(C_Table* t, const void* key, void* (*valptr));

bool C_Table_get_vptr(C_Table* t, void* key, void* (*valptr));

void C_Table_set(C_Table* t, const C_Table_Any* key, const C_Table_Any *value);

void C_Table_set_cptr(C_Table* t, const char* key, const void* value);

void C_Table_set_vptr(C_Table* t, const void* key, const void* value);

void C_Table_del(C_Table* *tptr);

void C_Table_new_iterator(C_Table* t, C_Table_Iterator* *iptr);

bool C_Table_Iterator_at_end(C_Table_Iterator* i);

void C_Table_Iterator_next(C_Table_Iterator* i);

bool C_Table_Iterator_current_key(C_Table_Iterator* i, C_Table_Any* *key);

bool C_Table_Iterator_current_pair(C_Table_Iterator* i, 
                                    const C_Table_Any* *keyptr, 
                                    C_Table_Any* *valptr);

bool C_Table_Iterator_remove_current(C_Table_Iterator* i);

bool C_Table_Iterator_del(C_Table_Iterator* *iptr);


/** Allocates new, empty struct */
void C_Table_Any_new(C_Table_Any* *aptr);

/** Initializes existing struct */
void C_Table_Any_init(C_Table_Any* a);

/** Clears contents of a struct, deallocates managed memory */
void C_Table_Any_clear(C_Table_Any* a);

/** Deallocates a struct */
void C_Table_Any_del(C_Table_Any* *aptr);

/* Sets to Type_CHARPTR */
void C_Table_Any_cptr(C_Table_Any* a, const char* cptr);

/* 
 * Sets to Type_BOOLEAN, Type_INTEGER, Type_NUMBER,
 * Type_{,U}INT{8,16,32,64,MAX}, Type_FLOAT, Type_DOUBLE.
 */
bool C_Table_Any_scalar(C_Table_Any* a, 
                        C_Table_Any_Type type,
                        const void* val);

/* Sets to Type_STRING_8 */
void C_Table_Any_string_8(C_Table_Any* a,
                            size_t len,
                            const utf8_t* value);

/* Sets to Type_STRING_16 */
void C_Table_Any_string_16(C_Table_Any* a,
                            size_t len,
                            const utf16_t* value);

/* Sets to Type_STRING_32 */
void C_Table_Any_string_32(C_Table_Any* a,
                            size_t len,
                            const utf32_t* value);

/* Sets to Type_USERDATA */
void C_Table_Any_userdata(C_Table_Any* a, const C_Userdata* userdata);
 
/* Sets to Type_VOIDPTR */
void C_Table_Any_vptr(C_Table_Any* a, const void* vptr);


/** Allocates new struct */
void C_Userdata_new(C_Userdata* *udptr);

/** Allocates and copies new userdata contents into existing struct. */
void C_Userdata_init(C_Userdata* ud, tag_t tag, size_t len, const void* ptr);

/** Frees contents. */
void C_Userdata_clear(C_Userdata* ud);

/** Frees struct. */
void C_Userdata_del(C_Userdata* *udptr);

#endif // CTABLE_H_INCLUDED