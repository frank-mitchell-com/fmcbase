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

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include "refcount.h"
#include "refset.h"
#include "strtable.h"
#include "cthread.h"

#include "symbol.h"

/*
 * Both tables have to be thread-safe.  For simplicity we'll use a 
 * single mutex, since functions that create symbols with strings will end
 * up searching the string table first, and functions that look up strings
 * will need exclusive access to the string table to prevent data corruption.
 * For adjusting the reference count of each symbol we'll trust atomic
 * operations.
 */

static C_String_Table* _symbols_by_name = NULL;
static C_Ref_Set*      _symbol_ref_set  = NULL;

static LOCK_DECL(_lock);

struct C_Symbol {
    /* not changed after creation */
    bool    tenured;
    size_t  strlen;
    uint8_t *strbuf;
};

static C_String_Table* symbols_by_name() {
    if (!_symbols_by_name) {
        C_String_Table_new(&_symbols_by_name, 5);
    }
    return _symbols_by_name;
}

static C_Ref_Set* symbol_ref_set() {
    if (!_symbol_ref_set) {
        C_Ref_Set_new(&_symbol_ref_set, 10);
    }
    return _symbol_ref_set;
}

/*
 * Delete a symbol from all tables and memory.
 * ASSUMES the CALLER has the LOCK.
 */
static void free_symbol(C_Symbol* sym) {
    if (!sym) return;

    C_Ref_Set_remove(symbol_ref_set(), sym);
    if (sym->strbuf && symbols_by_name()) {
        // In case of nulls ...
        C_String_Table_remove(symbols_by_name(), sym->strlen, sym->strbuf, NULL);
    }

    if (sym->strbuf) {
        free(sym->strbuf);
    }
    free(sym);
}

/*
 * External version of `free_symbol()` which acquires the lock.
 */
static void free_symbol_ext(void* p) {
    if (!p || !is_C_Symbol(p)) return;

    LOCK_ACQUIRE(_lock);

    free_symbol((C_Symbol*)p);

    LOCK_RELEASE(_lock);
}

/*
 * Allocate a new symbol and add it to all tables.
 * ASSUMES the CALLER has the LOCK.
 */
static C_Symbol* symbol_alloc_init(size_t len, const uint8_t* uptr) {
    C_Symbol* result = (C_Symbol*)malloc(sizeof(C_Symbol));
    if (!result) goto error;

    memset(result, 0, sizeof(C_Symbol));

    if (uptr) {
        uint8_t* buf = (uint8_t*)calloc(len+1, sizeof(uint8_t));
        if (!buf) goto error;

        // Can't use strdup() because of possible embedded nulls
        // TODO: Does this handle "" properly?
        memcpy(buf, uptr, (len + 1) * sizeof(uint8_t));

        result->tenured = true;
        result->strlen = len;
        result->strbuf = buf;

        if (!C_String_Table_add(symbols_by_name(), len, buf, result)) {
            free_symbol(result);
            return (C_Symbol*)C_String_Table_get(symbols_by_name(), len, buf);
        }
    }
    C_Ref_Set_add(symbol_ref_set(), result);
    C_Ref_Count_list(result);
    if (!result->tenured) {
        C_Ref_Count_on_free(result, free_symbol_ext);
    }

    return result;
error:
    free_symbol(result);
    return NULL;
}

/*
 * Look up and return the symbol for a byte sequence.
 * If none exists, it creates a new symbol.
 * ASSUMES the CALLER has the LOCK.
 */
static C_Symbol* find_symbol(size_t len, const uint8_t* uptr, bool* isnew) {
    C_Symbol* result;

    if (!uptr) return NULL;

    if (!symbols_by_name()) return NULL;

    result = (C_Symbol*)C_String_Table_get(symbols_by_name(), len, uptr);

    if (result != NULL) {
        (*isnew) = false;
    } else {
        result = symbol_alloc_init(len, uptr);
        (*isnew) = (result != NULL);
    }
    return result;
}

FMC_API bool is_C_Symbol(const void* p) {
    bool result;

    LOCK_ACQUIRE(_lock);

    result = C_Ref_Set_has(symbol_ref_set(), p);

    LOCK_RELEASE(_lock);

    return result;
}

FMC_API void C_Symbol_new(const C_Symbol* *symptr) {
    C_Symbol* sym;

    if (!symptr) return;

    LOCK_ACQUIRE(_lock);

    sym = symbol_alloc_init(0, NULL);

    LOCK_RELEASE(_lock);

    (*symptr) = sym;
}

FMC_API bool C_Symbol_for_cstring(const C_Symbol* *symptr, const char* cstr) {
    return C_Symbol_for_utf8_string(symptr, strlen(cstr), (const uint8_t*) cstr);
}

FMC_API bool C_Symbol_for_utf8_string(const C_Symbol* *symptr, size_t len, const uint8_t* uptr) {
    C_Symbol *sym;
    bool result = false;

    if (!symptr) return false;

    LOCK_ACQUIRE(_lock);

    sym = find_symbol(len, uptr, &result);

    LOCK_RELEASE(_lock);

    (*symptr) = sym;

    return result;
}

FMC_API int C_Symbol_references(const C_Symbol* sym) {
    return (int)C_Ref_Count_refcount(sym);
}

FMC_API const C_Symbol* C_Symbol_retain(const C_Symbol* sym) {
    return (C_Symbol*) C_Any_retain(sym);
}

FMC_API bool C_Symbol_release(const C_Symbol* *symptr) {
    return C_Any_release((const void**)symptr);
}

FMC_API void C_Symbol_set(const C_Symbol* *lvalptr, const C_Symbol* val) {
    C_Any_set((const void**)lvalptr, (const void*)val);
}

FMC_API const uint8_t* C_Symbol_as_utf8_string(const C_Symbol* sym, size_t *lenptr) {
    if (!sym || !is_C_Symbol(sym)) return NULL;
    if (lenptr) {
        (*lenptr) = sym->strlen;
    }
    if (sym->strbuf) {
        return sym->strbuf;
    } else {
        // Valid symbol, but no string value
        return (const uint8_t*) "";
    }
}

FMC_API ssize_t C_Symbol_as_cstring(const C_Symbol* sym, size_t len, char* buf) {
    ssize_t result;

    if (!sym || !is_C_Symbol(sym)) return -1;

    // Can't use strcpy because we have to scan for nulls
    result = 0;
    for (int i = 0; i < sym->strlen && result < len; i++) {
        uint8_t c = sym->strbuf[i];
        if (c != '\0') {
            buf[result] = c;
            result++;
        } else {
            // Take *THIS*, Never-Nesters!
            if (result >= len-2) {
                buf[result] = '\0';
                result++;
            } else {
                buf[result] = 0xC0;
                buf[result+1] = 0x80;
                result += 2;
            }
        }
    }
    return result;
}
