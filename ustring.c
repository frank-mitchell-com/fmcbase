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

#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "cconv.h"
#include "csymbol.h"
#include "ustring.h"

static void* default_alloc(void* unused, void* p, size_t nmem, size_t sz) {
    if (p != NULL) {
        if (nmem == 0 || sz == 0) {
            free(p);
            return NULL;
        } else {
            return realloc(p, nmem * sz);
        }
    }
    if (nmem > 0 && sz > 0) {
        return malloc(nmem * sz);
    }
    return NULL;
}

// TODO: Protect these with a mutex?

static volatile u_string_alloc _alloc_func = default_alloc;
static volatile void*          _alloc_data = NULL;

struct _U_String {
    volatile atomic_int refcnt;

    // Does not change after creation
    C_Symbol* encoding;
    size_t    nbytes;
    uint8_t*  bytes;
};

static bool make_string(U_String* *sp, C_Symbol* enc, size_t len, size_t csz, const void* buf) {
    octet_t* sb;
    U_String* s;

    if (sp == NULL || enc == NULL || buf == NULL) return false;

    *sp = NULL;

    s = (U_String *)_alloc_func((void *)_alloc_data, NULL, 1, sizeof(U_String));
    if (s == NULL) return false;

    sb = _alloc_func((void *)_alloc_data, NULL, len + 1, csz);
    if (s == NULL) {
        free(s);
        return false;
    }
    // TODO: Watch out for overflow from len * csz. It *should* be fine, but ...
    memcpy(sb, buf, len * csz);

    s->refcnt   = 1;
    s->encoding = enc;
    s->nbytes   = len * csz;
    s->bytes    = sb;

    *sp = s;
    return true;
}

static void free_string(U_String* s) {
    free(s->bytes);
    free(s);
}

USTR_API void U_String_set_allocator(u_string_alloc func, void *data, bool local) {
    // TODO: Make thread local on request
    // TODO: Make thread-safe
    _alloc_data = data;
    _alloc_func = func;
}

static C_Symbol* lookup_symbol(const char* str) {
    C_Symbol* result = NULL;
    C_Symbol_for_utf8_string(&result, strlen(str), (const uint8_t*)str);
    return result;
}

USTR_API bool U_String_new_ascii(U_String* *sp, size_t sz, const char* buf) {
    C_Symbol* enc;
    if (C_Conv_is_ascii(sz, buf)) {
        enc = lookup_symbol("US-ASCII");
    } else {
        enc = lookup_symbol("UTF-8");
    }
    return make_string(sp, enc, sz, sizeof(octet_t*), buf);
}

USTR_API bool U_String_new_utf8(U_String* *sp, size_t sz, const utf8_t* buf) {
    return U_String_new_ascii(sp, sz, (const char*)buf);
}

USTR_API bool U_String_new_utf16(U_String* *sp, size_t sz, const utf16_t* buf) {
    C_Symbol* enc = lookup_symbol("UTF-16");
    return make_string(sp, enc, sz, sizeof(utf16_t), buf);
}

USTR_API bool U_String_new_utf32(U_String* *sp, size_t sz, const wchar_t* buf) {
    C_Symbol* enc = lookup_symbol("UTF-32");
    return make_string(sp, enc, sz, sizeof(wchar_t), buf);
}

USTR_API bool U_String_new_encoded(U_String* *sp, C_Symbol* encoding, size_t sz, const octet_t* buf) {
    return make_string(sp, encoding, sz, sizeof(octet_t), buf);
}

USTR_API bool U_String_new_from_cstring(U_String* *sp, const char* cstr) {
    size_t len = strlen(cstr);
    return U_String_new_ascii(sp, len, cstr);
}

USTR_API wchar_t U_String_char_at(U_String* s, size_t i) {
    return 0;
}

USTR_API C_Symbol* U_String_encoding(U_String* s) {
    return s->encoding;
}

USTR_API const char* U_String_encoding_name(U_String* s) {
    return (const char*)C_Symbol_as_utf8_string(s->encoding, NULL);
}

USTR_API size_t U_String_length(U_String* s) {
    return 0;
}

USTR_API size_t U_String_to_byte(U_String* s, size_t offset, size_t max, octet_t* buf) {
    return (size_t)memcpy(&(buf[max]), s->bytes, (max < s->nbytes) ? max : s->nbytes);
}

USTR_API size_t U_String_to_utf8(U_String* s, size_t offset, size_t max, utf8_t* buf) {
    const char* encname = U_String_encoding_name(s);
    return C_Conv_transcode(encname, "UTF-8", 
            s->nbytes, (char*)s->bytes, 
            max, (char*)(&(buf[offset])), NULL);
}

USTR_API size_t U_String_to_utf32(U_String* s, size_t offset, size_t max, wchar_t* buf) {
    const char* encname = U_String_encoding_name(s);
    return C_Conv_transcode(encname, "UTF-32", 
            s->nbytes, (char*)s->bytes, 
            max * sizeof(wchar_t), (char*)(&(buf[offset])), NULL);
}

USTR_API size_t U_String_each(U_String* s, void* data, u_iterator f) {
    return 0;
}

USTR_API size_t U_String_each_after(U_String* s, size_t index, void* data, u_iterator f) {
    return 0;
}

USTR_API bool U_String_slice(U_String* *sp, U_String* s, int first, int last) {
    return false;
}

USTR_API bool U_String_slice_from(U_String* *sp, U_String* s, int first) {
    return false;
}

USTR_API bool U_String_slice_to(U_String* *sp, U_String* s, int last) {
    return false;
}

USTR_API bool U_String_join(U_String* *sp, U_String* head, U_String* tail) {
    return false;
}

USTR_API bool U_String_join_n(U_String* *sp, size_t n, ...) {
    return false;
}

USTR_API bool U_String_is_live(U_String* s) {
    return true;
}

USTR_API size_t U_String_references(U_String* s) {
    if (s == NULL || !U_String_is_live(s)) return 0;

    return s->refcnt;
}

USTR_API bool U_String_retain(U_String* s) {
    if (s == NULL || !U_String_is_live(s)) return false;
    s->refcnt++;
    return true;
}

USTR_API bool U_String_release(U_String* *sp) {
    U_String* s;

    if (sp == NULL) return 0;

    s = *sp;
    if (s == NULL || !U_String_is_live(s)) return false;
    if (s->refcnt > 1) {
        s->refcnt--;
    } else {
        free_string(s);
        (*sp) = NULL;
    }
    return true;
}

USTR_API bool U_String_set(U_String* *lvalue, U_String* rvalue) {
    U_String* oldvalue;

    if (!lvalue) return false;

    if (!U_String_retain(rvalue)) return false;

    oldvalue = (*lvalue);
    *lvalue = rvalue;
    return U_String_release(&oldvalue);
}

