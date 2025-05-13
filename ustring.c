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
#include "ustring.h"

typedef enum String_Type {
    STRING_EMPTY = 0,
    STRING_LATIN_1 = 1,
    STRING_UCS_2 = 2,
    STRING_UCS_4 = 3,
} String_Type;


struct _C_Ustring {
    // Does not change after creation
    String_Type type;
    uint64_t    hash;
    union _string {
        struct _string0 {
            size_t    len;
        } e;
        struct _string8 {
            size_t    len;
            octet_t   arr[];
        } c;
        struct _string16 {
            size_t    len;
            char16_t   arr[];
        } u;
        struct _string32 {
            size_t    len;
            char32_t   arr[];
        } w;
    } s;
};

static uint64_t hashcode(char32_t* buf, size_t len) {
    // Stolen from 
    // https://stackoverflow.com/questions/8317508/hash-function-for-a-string
    uint64_t result = 37;
    const int A = 54059;
    const int B = 76973;
    for (int i = 0; i < len; i++) {
        result = (result * A) ^ (buf[i] * B);
    }
    return result;
}

// TODO: Implement compressed strings, i.e. use arrays of char16_t 
// or char8_t where the characters fit entirely into UCS-2 (sans surrogates),
// Latin-1, or ASCII.  See Java 9 java.lang.String for inspiration.

static const C_Ustring* make_utf32_string(const char* charset, size_t insz, const octet_t* inbuf) {
    C_Ustring* s;
    ssize_t read, written, offset;
    size_t ulen, usiz;
    size_t bufsz = insz+2;
    char32_t buffer[bufsz];

    written = C_Conv_transcode(charset, UTF_32, 
                                    insz, 
                                    (octet_t*)inbuf, 
                                    bufsz * sizeof(char32_t), 
                                    (octet_t*)buffer, 
                                    &read);
    if (read != insz) goto error;
    
    if (buffer[0] == L'\uFEFF') {
        offset = 1;
    } else {
        offset = 0;
    }

    ulen = (written-offset)/sizeof(char32_t);
    usiz = C_Conv_min_bytes(ulen, buffer+offset);

    s = (C_Ustring *)malloc(sizeof(C_Ustring) + (ulen+1) * usiz);
    if (s == NULL) goto error;

    s->hash = hashcode(buffer + offset, ulen);

    switch (usiz) {
        case sizeof(octet_t):
            s->type = STRING_LATIN_1;
            for (int i = 0; i < ulen; i++) {
                s->s.c.arr[i] = (octet_t)buffer[offset+i];
            }
            s->s.c.arr[ulen] = 0;
            s->s.c.len = ulen;
            break;
        case sizeof(char16_t):
            s->type = STRING_UCS_2;
            for (int i = 0; i < ulen; i++) {
                s->s.u.arr[i] = (char16_t)buffer[offset+i];
            }
            s->s.u.arr[ulen] = 0;
            s->s.u.len = ulen;
            break;
        case sizeof(char32_t):
            s->type = STRING_UCS_4;
            memmove(s->s.w.arr, buffer + offset, ulen * usiz);
            s->s.w.arr[ulen] = 0;
            s->s.w.len = ulen;
            break;
        default:
            s->type = STRING_EMPTY;
            s->s.e.len = 0;
            break;
    }

    return s;
error:
    if (s) free(s);
    return NULL;
}

static void free_string(void* p) {
    free(p);
}

static bool make_string(const C_Ustring* *sp, const char* charset, size_t len, size_t csz, const void* buf) {
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

FMC_API bool C_Ustring_new_ascii(const C_Ustring* *sp, size_t sz, const char* buf) {
    if (C_Conv_is_ascii(sz, buf)) {
        return make_string(sp, ASCII, sz, sizeof(octet_t), buf);
    } else {
        // TODO: Use locale instead
        return make_string(sp, UTF_8, sz, sizeof(octet_t), buf);
    }
}

FMC_API bool C_Ustring_new_utf8(const C_Ustring* *sp, size_t sz, const char8_t* buf) {
    return make_string(sp, UTF_8, sz, sizeof(char8_t), buf);
}

FMC_API bool C_Ustring_new_utf16(const C_Ustring* *sp, size_t sz, const char16_t* buf) {
    return make_string(sp, UTF_16, sz, sizeof(char16_t), buf);
}

FMC_API bool C_Ustring_new_utf32(const C_Ustring* *sp, size_t sz, const char32_t* buf) {
    return make_string(sp, UTF_32, sz, sizeof(char32_t), buf);
}

FMC_API bool C_Ustring_new_encoded(const C_Ustring* *sp, const char* charset, size_t sz, const octet_t* buf) {
    return make_string(sp, charset, sz, sizeof(octet_t), buf);
}

FMC_API bool C_Ustring_new_from_cstring(const C_Ustring* *sp, const char* cstr) {
    size_t len = strlen(cstr);
    return C_Ustring_new_ascii(sp, len, cstr);
}

FMC_API int C_Ustring_compare(const C_Ustring* a, const C_Ustring* b) {
    if (a == NULL || b == NULL) {
        if (a == b) {
            return 0;
        } else if (a == NULL) {
            return -1;
        } else {
            return 1;
        }
    }
    if (C_Ustring_length(a) != C_Ustring_length(b)) {
        return C_Ustring_length(a) - C_Ustring_length(b);
    }
    for (size_t i = 0; i < C_Ustring_length(a); i++) {
        char32_t ac = C_Ustring_char_at(a, i);
        char32_t bc = C_Ustring_char_at(b, i);
        if (ac != bc) {
            return ac - bc;
        }
    }
    return 0;
}

FMC_API bool C_Ustring_equals(const C_Ustring* a, const C_Ustring* b) {
    return C_Ustring_compare(a, b) == 0;
}

FMC_API uint64_t C_Ustring_hashcode(const C_Ustring* s) {
    if (s == NULL) return 0;
    return s->hash;
}

FMC_API char32_t C_Ustring_char_at(const C_Ustring* s, size_t i) {
    if (i >= C_Ustring_length(s)) {
        return L'\0';
    }
    switch (s->type) {
        case STRING_LATIN_1:
            return (char32_t)s->s.c.arr[i];
        case STRING_UCS_2:
            return (char32_t)s->s.u.arr[i];
        case STRING_UCS_4:
            return s->s.w.arr[i];
        default:
            return L'\0';
    }
}

FMC_API size_t C_Ustring_length(const C_Ustring* s) {
    switch (s->type) {
        case STRING_LATIN_1:
            return s->s.c.len;
        case STRING_UCS_2:
            return s->s.u.len;
        case STRING_UCS_4:
            return s->s.w.len;
        default:
            return 0;
    }
}

static size_t latin1_to_8(size_t insz, const octet_t* inbuf, size_t outsz, char8_t* outbuf) {
    size_t j = 0;
    for (size_t i = 0; i < insz && j < outsz; i++) {
        char32_t cp = inbuf[i];

        if (cp <= 0x7F) {
            outbuf[j] = cp;
            j += 1;
        } else if (j+2 < outsz) {
            outbuf[j + 0] = (uint8_t) (0xC0 | (0x1F & (cp >> 6)));
            outbuf[j + 1] = (uint8_t) (0x80 | (0x3F & cp));
            j += 2;
        } else {
            outbuf[j] = 0;
            j+=2;
        }
    }
    return j;
}

FMC_API size_t C_Ustring_to_utf8(const C_Ustring* s, size_t offset, size_t max, char8_t* buf) {
     switch (s->type) {
        case STRING_EMPTY:
            buf[offset] = L'\0';
            return 1;
        case STRING_LATIN_1:
            return latin1_to_8(s->s.c.len+1, s->s.c.arr, max, buf+offset);
        case STRING_UCS_2:
            return C_Conv_char16_to_8(s->s.u.len+1, s->s.u.arr, max, buf+offset);
        case STRING_UCS_4:
            return C_Conv_char32_to_8(s->s.w.len+1, s->s.w.arr, max, buf+offset);
    }
    return 0;
}

FMC_API size_t C_Ustring_to_utf32(const C_Ustring* s, size_t offset, size_t max, char32_t* buf) {
    // Need to write the final null byte.
    if (s->type == STRING_UCS_4) {
        size_t safesz = (max < s->s.w.len+1) ? max : s->s.w.len+1;
        memcpy(buf+offset, s->s.w.arr, safesz * sizeof(char32_t));
        return safesz;
    } else {
        size_t len = C_Ustring_length(s);
        size_t i;
        for (i = 0; i < max && i < len+1; i++) {
            buf[i] = C_Ustring_char_at(s, i);
        }
        return i;
    }
}

FMC_API ssize_t C_Ustring_to_charset(const C_Ustring* s, const char* charset, size_t offset, size_t max, octet_t* buf) {
    size_t insz;
    octet_t* inbuf;
    const char* incs;
    char32_t emptybuf[2] = U"\0\0";

    // Need to write the final null byte.
    switch (s->type) {
        case STRING_LATIN_1:
            incs = "ISO-8859-1";
            insz = (s->s.c.len + 1) * sizeof(octet_t);
            inbuf = (octet_t*)s->s.c.arr;
            break;
        case STRING_UCS_2:
            incs = UTF_16;
            insz = (s->s.u.len + 1) * sizeof(char16_t);
            inbuf = (octet_t*)s->s.u.arr;
            break;
        case STRING_UCS_4:
            incs = UTF_32;
            insz = (s->s.w.len + 1) * sizeof(char32_t);
            inbuf = (octet_t*)s->s.w.arr;
            break;
        default:
            incs = UTF_32;
            insz = sizeof(char32_t);
            inbuf = (octet_t*)emptybuf;
            break;
    }

    return C_Conv_transcode(incs, charset, insz, inbuf, max, buf+offset, NULL);
}

FMC_API bool C_Ustring_slice(const C_Ustring* *sp, const C_Ustring* s, ssize_t first, ssize_t last) {
    return false;
}

FMC_API bool C_Ustring_slice_from(const C_Ustring* *sp, const C_Ustring* s, ssize_t first) {
    return C_Ustring_slice(sp, s, first, -1);
}

FMC_API bool C_Ustring_slice_to(const C_Ustring* *sp, const C_Ustring* s, ssize_t last) {
    return C_Ustring_slice(sp, s, 0, last);
}

FMC_API bool C_Ustring_join(const C_Ustring* *sp, const C_Ustring* head, const C_Ustring* tail) {
    return false;
}

FMC_API bool C_Ustring_join_n(const C_Ustring* *sp, size_t n, ...) {
    return false;
}

FMC_API bool C_Ustring_is_live(const C_Ustring* s) {
    return C_Ref_Count_is_listed(s);
}

FMC_API size_t C_Ustring_references(const C_Ustring* s) {
    return C_Ref_Count_refcount(s);
}

FMC_API const C_Ustring* C_Ustring_retain(const C_Ustring* s) {
    return (const C_Ustring*)C_Any_retain(s);
}

FMC_API bool C_Ustring_release(const C_Ustring* *sp) {
    return C_Any_release((const void**)sp);
}

FMC_API void C_Ustring_set(const C_Ustring* *lvalue, const C_Ustring* rvalue) {
    C_Any_set((const void**)lvalue, (const void*)rvalue);
}

