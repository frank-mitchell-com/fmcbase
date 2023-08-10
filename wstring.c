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

struct _C_Wstring {
    // Does not change after creation
    size_t    wcs_len;
    wchar_t*  wcs_arr;
};

// TODO: Implement compressed strings, i.e. use arrays of utf16_t 
// or utf8_t where the characters fit entirely into UCS-2 (sans surrogates),
// Latin-1, or ASCII.  See Java 9 java.lang.String for inspiration.

static C_Wstring* make_utf32_string(const char* charset, size_t insz, const octet_t* inbuf) {
    C_Wstring* s;
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

    s = (C_Wstring *)malloc(sizeof(C_Wstring));
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
    C_Wstring* s = p;
    free(s->wcs_arr);
    free(s);
}

static bool make_string(C_Wstring* *sp, const char* charset, size_t len, size_t csz, const void* buf) {
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

FMC_API bool C_Wstring_new_ascii(C_Wstring* *sp, size_t sz, const char* buf) {
    if (C_Conv_is_ascii(sz, buf)) {
        return make_string(sp, ASCII, sz, sizeof(octet_t), buf);
    } else {
        // TODO: Use locale instead
        return make_string(sp, UTF_8, sz, sizeof(octet_t), buf);
    }
}

FMC_API bool C_Wstring_new_utf8(C_Wstring* *sp, size_t sz, const utf8_t* buf) {
    return make_string(sp, UTF_8, sz, sizeof(utf8_t), buf);
}

FMC_API bool C_Wstring_new_utf16(C_Wstring* *sp, size_t sz, const utf16_t* buf) {
    return make_string(sp, UTF_16, sz, sizeof(utf16_t), buf);
}

FMC_API bool C_Wstring_new_utf32(C_Wstring* *sp, size_t sz, const wchar_t* buf) {
    return make_string(sp, UTF_32, sz, sizeof(wchar_t), buf);
}

FMC_API bool C_Wstring_new_encoded(C_Wstring* *sp, const char* charset, size_t sz, const octet_t* buf) {
    return make_string(sp, charset, sz, sizeof(octet_t), buf);
}

FMC_API bool C_Wstring_new_from_cstring(C_Wstring* *sp, const char* cstr) {
    size_t len = strlen(cstr);
    return C_Wstring_new_ascii(sp, len, cstr);
}

FMC_API wchar_t C_Wstring_char_at(C_Wstring* s, size_t i) {
    if (i >= s->wcs_len) {
        return 0;
    } else {
        return s->wcs_arr[i];
    }
}

FMC_API size_t C_Wstring_length(C_Wstring* s) {
    return s->wcs_len;
}

FMC_API size_t C_Wstring_to_utf8(C_Wstring* s, size_t offset, size_t max, utf8_t* buf) {
    // Need to write the final null byte.
    return C_Conv_utf32_to_8(s->wcs_len+1, s->wcs_arr, max, buf+offset);
}

FMC_API size_t C_Wstring_to_utf32(C_Wstring* s, size_t offset, size_t max, utf32_t* buf) {
    // Need to write the final null byte.
    size_t safesz = (max < s->wcs_len+1) ? max : s->wcs_len+1;
    return (size_t)wmemcpy(buf+offset, s->wcs_arr, safesz);
}

FMC_API ssize_t C_Wstring_to_charset(C_Wstring* s, const char* charset, size_t offset, size_t max, octet_t* buf) {
    // Need to write the final null byte.
    return C_Conv_transcode(UTF_32, charset, 
                                    (s->wcs_len+1)*sizeof(wchar_t), 
                                    (octet_t*)s->wcs_arr, 
                                    max, 
                                    buf+offset, 
                                    NULL);
}

FMC_API size_t C_Wstring_each(C_Wstring* s, void* data, wchar_iterator f) {
    return 0;
}

FMC_API size_t C_Wstring_each_after(C_Wstring* s, size_t index, void* data, wchar_iterator f) {
    return 0;
}

FMC_API bool C_Wstring_slice(C_Wstring* *sp, C_Wstring* s, ssize_t first, ssize_t last) {
    return false;
}

FMC_API bool C_Wstring_slice_from(C_Wstring* *sp, C_Wstring* s, ssize_t first) {
    return C_Wstring_slice(sp, s, first, -1);
}

FMC_API bool C_Wstring_slice_to(C_Wstring* *sp, C_Wstring* s, ssize_t last) {
    return C_Wstring_slice(sp, s, 0, last);
}

FMC_API bool C_Wstring_join(C_Wstring* *sp, C_Wstring* head, C_Wstring* tail) {
    return false;
}

FMC_API bool C_Wstring_join_n(C_Wstring* *sp, size_t n, ...) {
    return false;
}

FMC_API bool C_Wstring_is_live(C_Wstring* s) {
    return C_Ref_Count_is_listed(s);
}

FMC_API size_t C_Wstring_references(C_Wstring* s) {
    return C_Ref_Count_refcount(s);
}

FMC_API C_Wstring* C_Wstring_retain(C_Wstring* s) {
    return (C_Wstring*)C_Any_retain(s);
}

FMC_API bool C_Wstring_release(C_Wstring* *sp) {
    return C_Any_release((const void**)sp);
}

FMC_API void C_Wstring_set(C_Wstring* *lvalue, C_Wstring* rvalue) {
    C_Any_set((const void**)lvalue, (const void*)rvalue);
}

