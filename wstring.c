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

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "thread.h"
#include "convert.h"
#include "refcount.h"
#include "wstring.h"

struct _U_String {
    // Does not change after creation
    size_t    wcs_len;
    wchar_t*  wcs_arr;
};

// TODO: Implement compressed strings, i.e. use arrays of utf16_t 
// or utf8_t where the characters fit entirely into UCS-2 (sans surrogates),
// Latin-1, or ASCII.  See Java 9 java.lang.String for inspiration.

static U_String* make_utf32_string(const char* charset, size_t insz, const octet_t* inbuf) {
    U_String* s;
    ssize_t read, written, offset;
    size_t ulen;
    wchar_t *ubuf;
    size_t bufsz = insz+2;
    wchar_t buffer[bufsz];

    written = C_Conv_transcode(charset, UTF_32, 
                                    insz, 
                                    (octet_t*)inbuf, 
                                    bufsz * sizeof(wchar_t), 
                                    (octet_t*)buffer, 
                                    &read);
    if (read != insz) goto error;
    
    ulen = written/sizeof(wchar_t);

    s = (U_String *)malloc(sizeof(U_String));
    if (s == NULL) goto error;

    ubuf = malloc((ulen+1) * sizeof(wchar_t));
    if (!ubuf) goto error;

    if (buffer[0] == L'\uFEFF') {
        offset = 1;
    } else {
        offset = 0;
    }

    bzero(ubuf, ulen+1);
    memmove(ubuf, buffer+offset, (ulen-offset) * sizeof(wchar_t));
    ubuf[ulen-offset] = 0;

    s->wcs_len = ulen-offset;
    s->wcs_arr = ubuf;
    return s;
error:
    if (ubuf) free(ubuf);
    if (s) free(s);
    return NULL;
}

static void free_string(void* p) {
    U_String* s = p;
    free(s->wcs_arr);
    free(s);
}

static bool make_string(U_String* *sp, const char* charset, size_t len, size_t csz, const void* buf) {
    if (sp == NULL || charset == NULL || buf == NULL) return false;

    *sp = NULL;

    *sp = make_utf32_string(charset, len * csz, buf);
    if (*sp != NULL) {
        C_Ref_Count_list(*sp);
        C_Ref_Count_on_zero(*sp, free_string);
        return true;
    }
    return false;
}

FMC_API bool U_String_new_ascii(U_String* *sp, size_t sz, const char* buf) {
    if (C_Conv_is_ascii(sz, buf)) {
        return make_string(sp, ASCII, sz, sizeof(octet_t), buf);
    } else {
        // TODO: Use locale instead
        return make_string(sp, UTF_8, sz, sizeof(octet_t), buf);
    }
}

FMC_API bool U_String_new_utf8(U_String* *sp, size_t sz, const utf8_t* buf) {
    return make_string(sp, UTF_8, sz, sizeof(utf8_t), buf);
}

FMC_API bool U_String_new_utf16(U_String* *sp, size_t sz, const utf16_t* buf) {
    return make_string(sp, UTF_16, sz, sizeof(utf16_t), buf);
}

FMC_API bool U_String_new_utf32(U_String* *sp, size_t sz, const wchar_t* buf) {
    return make_string(sp, UTF_32, sz, sizeof(wchar_t), buf);
}

FMC_API bool U_String_new_encoded(U_String* *sp, const char* charset, size_t sz, const octet_t* buf) {
    return make_string(sp, charset, sz, sizeof(octet_t), buf);
}

FMC_API bool U_String_new_from_cstring(U_String* *sp, const char* cstr) {
    size_t len = strlen(cstr);
    return U_String_new_ascii(sp, len, cstr);
}

FMC_API wchar_t U_String_char_at(U_String* s, size_t i) {
    if (i >= s->wcs_len) {
        return 0;
    } else {
        return s->wcs_arr[i];
    }
}

FMC_API size_t U_String_length(U_String* s) {
    return s->wcs_len;
}

FMC_API size_t U_String_to_utf8(U_String* s, size_t offset, size_t max, utf8_t* buf) {
    // Need to write the final null byte.
    return C_Conv_utf32_to_8(s->wcs_len+1, s->wcs_arr, max, buf+offset);
}

FMC_API size_t U_String_to_utf32(U_String* s, size_t offset, size_t max, utf32_t* buf) {
    // Need to write the final null byte.
    size_t safesz = (max < s->wcs_len+1) ? max : s->wcs_len+1;
    return (size_t)wmemcpy(buf+offset, s->wcs_arr, safesz);
}

FMC_API ssize_t U_String_to_charset(U_String* s, const char* charset, size_t offset, size_t max, octet_t* buf) {
    // Need to write the final null byte.
    return C_Conv_transcode(UTF_32, charset, 
                                    (s->wcs_len+1)*sizeof(wchar_t), 
                                    (octet_t*)s->wcs_arr, 
                                    max, 
                                    buf+offset, 
                                    NULL);
}

FMC_API size_t U_String_each(U_String* s, void* data, wchar_iterator f) {
    return 0;
}

FMC_API size_t U_String_each_after(U_String* s, size_t index, void* data, wchar_iterator f) {
    return 0;
}

FMC_API bool U_String_slice(U_String* *sp, U_String* s, ssize_t first, ssize_t last) {
    return false;
}

FMC_API bool U_String_slice_from(U_String* *sp, U_String* s, ssize_t first) {
    return U_String_slice(sp, s, first, -1);
}

FMC_API bool U_String_slice_to(U_String* *sp, U_String* s, ssize_t last) {
    return U_String_slice(sp, s, 0, last);
}

FMC_API bool U_String_join(U_String* *sp, U_String* head, U_String* tail) {
    return false;
}

FMC_API bool U_String_join_n(U_String* *sp, size_t n, ...) {
    return false;
}

FMC_API bool U_String_is_live(U_String* s) {
    return C_Ref_Count_is_listed(s);
}

FMC_API size_t U_String_references(U_String* s) {
    return C_Ref_Count_refcount(s);
}

FMC_API U_String* U_String_retain(U_String* s) {
    return (U_String*)C_Any_retain(s);
}

FMC_API bool U_String_release(U_String* *sp) {
    return C_Any_release((const void**)sp);
}

FMC_API void U_String_set(U_String* *lvalue, U_String* rvalue) {
    C_Any_set((const void**)lvalue, (const void*)rvalue);
}

