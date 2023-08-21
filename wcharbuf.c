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
#include <wchar.h>
#include "refcount.h"
#include "wcharbuf.h"

#define DEFAULT_BUF_SIZ    11

struct _C_Wchar_Buffer {
    wchar_t* buffer;
    size_t   capacity;
    size_t   length;
};

static void free_buffer(void* p) {
    C_Wchar_Buffer* buf = (C_Wchar_Buffer*)p;
    free(buf->buffer);
    free(buf);
}

static void alloc_init(C_Wchar_Buffer* *newref, size_t n) {
    size_t cap = n > DEFAULT_BUF_SIZ ? n : DEFAULT_BUF_SIZ;
    C_Wchar_Buffer* wcb = NULL;
    wchar_t* b = NULL;

    if (!newref) return;
    *newref = NULL;

    wcb = malloc(sizeof(C_Wchar_Buffer));
    if (!wcb) goto error;

    b = calloc(cap+1, sizeof(wchar_t));
    if (!b) goto error;

    wcb->buffer = b;
    wcb->capacity = cap;
    wcb->length = 0;

    C_Ref_Count_list(wcb);
    C_Ref_Count_on_zero(wcb, free_buffer);

    *newref = wcb;
    return;
error:
    if (b) free(b);
    if (wcb) free(wcb);
    return;
}

static bool ensure_capacity(C_Wchar_Buffer* self, size_t n) {
    wchar_t* newb;

    if (n <= self->capacity) {
        return true;
    }
    newb = reallocarray(self->buffer, n+1, sizeof(wchar_t));
    if (newb == 0) {
        return false;
    }
    self->buffer = newb;
    self->capacity = n;
    return true;
}

FMC_API void C_Wchar_Buffer_new(C_Wchar_Buffer* *newref) {
    alloc_init(newref, 0);
}

FMC_API void C_Wchar_Buffer_new_size(C_Wchar_Buffer* *newref, size_t capacity) {
    alloc_init(newref, capacity);
}

FMC_API void C_Wchar_Buffer_new_from_wcs(C_Wchar_Buffer* *newref, const wchar_t* wcs) {
    size_t len = wcslen(wcs);
    alloc_init(newref, len);
    if (*newref == NULL) return;
    wmemcpy((*newref)->buffer, wcs, len);
    (*newref)->length = len;
}

FMC_API void C_Wchar_Buffer_new_from_string(C_Wchar_Buffer* *newref, const C_Wstring* str) {
    size_t len = C_Wstring_length(str);
    alloc_init(newref, len);
    if (*newref == NULL) return;
    C_Wstring_to_utf32(str, 0, len, (*newref)->buffer);
    (*newref)->length = len;
}

FMC_API size_t C_Wchar_Buffer_length(C_Wchar_Buffer* self) {
    return self->length;
}

static size_t abs_index(C_Wchar_Buffer* self, ssize_t index) {
    size_t  len = self->length;
    ssize_t i = (index < 0) ? len + index : index;
    if (i < 0) {
        return 0;
    } else if (i > len) {
        return len;
    } else {
        return i;
    }
}

FMC_API wchar_t C_Wchar_Buffer_char_at(C_Wchar_Buffer* self, ssize_t index) {
    ssize_t i = abs_index(self, index);

    if (i == self->length) {
        return 0;
    }
    return self->buffer[i];
}

FMC_API bool C_Wchar_Buffer_slice(C_Wchar_Buffer* self, ssize_t first, ssize_t last, const C_Wstring* *sp) {
    ssize_t i = abs_index(self, first);
    ssize_t j = abs_index(self, last);

    if (i > j) {
        i = j;
    }
    C_Wstring_new_utf32(sp, j - i, self->buffer + i);
    return *sp != NULL;
}

FMC_API void C_Wchar_Buffer_append_char(C_Wchar_Buffer* self, wchar_t c) {
    if (!ensure_capacity(self, self->length+1)) return;
    // TODO
}

FMC_API void C_Wchar_Buffer_append_string(C_Wchar_Buffer* self, const C_Wstring* s) {
    if (!ensure_capacity(self, self->length + C_Wstring_length(s))) return;
    // TODO
}

FMC_API void C_Wchar_Buffer_insert_char(C_Wchar_Buffer* self, ssize_t index, wchar_t c) {
    if (!ensure_capacity(self, self->length+1)) return;
    // TODO
}

FMC_API void C_Wchar_Buffer_insert_string(C_Wchar_Buffer* self, ssize_t index, const C_Wstring* s) {
    if (!ensure_capacity(self, self->length + C_Wstring_length(s))) return;
    // TODO
}

FMC_API void C_Wchar_Buffer_set_char(C_Wchar_Buffer* self, ssize_t index, wchar_t c) {
    // TODO
}

FMC_API void C_Wchar_Buffer_set_slice(C_Wchar_Buffer* self, ssize_t first, ssize_t last, const C_Wstring* s) {
    if (!ensure_capacity(self, self->length + C_Wstring_length(s))) return;
    // TODO
}

FMC_API void C_Wchar_Buffer_set_string(C_Wchar_Buffer* self, ssize_t index, const C_Wstring* s) {
    if (!ensure_capacity(self, self->length + C_Wstring_length(s))) return;
    // TODO
}

FMC_API bool C_Wchar_Buffer_to_string(C_Wchar_Buffer* self, const C_Wstring* *sp) {
    if (!self || !sp) return false;

    C_Wstring_new_utf32(sp, self->length, self->buffer);
    return *sp != NULL;
}

FMC_API bool C_Wchar_Buffer_is_live(const C_Wchar_Buffer* b) {
    return C_Ref_Count_is_listed(b);
}

FMC_API C_Wchar_Buffer* C_Wchar_Buffer_retain(const C_Wchar_Buffer* b) {
    return (C_Wchar_Buffer*)C_Any_retain(b);
}

FMC_API bool C_Wchar_Buffer_release(C_Wchar_Buffer* *bptr) {
    return C_Any_release((const void**)bptr);
}

FMC_API void C_Wchar_Buffer_set(const C_Wchar_Buffer* *lvalue, const C_Wchar_Buffer* rvalue) {
    C_Any_set((const void**)lvalue, (const void*)rvalue);
}

