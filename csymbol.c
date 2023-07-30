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
#include <stdatomic.h>
#include <stdlib.h>
#include "ctable.h"

#include "csymbol.h"

/*
 * Both tables have to be thread-safe.  For simplicity we'll use a 
 * single mutex, since functions that create symbols with strings will end
 * up searching the string table first, and functions that look up strings
 * will need exclusive access to the string table to prevent data corruption.
 * For adjusting the reference count of each symbol we'll trust atomic
 * operations.
 */

static C_Table* _symbols_by_name = NULL;
static C_Table* _symbols_by_ref  = NULL;

struct C_Symbol {
    volatile atomic_int refcnt;

    /* not changed after creation */
    bool    tenured;
    size_t  strlen;
    uint8_t *strbuf;
};

static void lock_acquire() {
    // Assume re-entrant lock so multiple calls simply increment a count
}

static void lock_release() {
    // Assume re-entrant lock so multiple calls simply decrement a count
}

static C_Table* symbols_by_name() {
    if (!_symbols_by_name) {
        C_Table_new(&_symbols_by_name, 5);
    }
    return _symbols_by_name;
}

static C_Table* symbols_by_ref() {
    if (!_symbols_by_ref) {
        C_Table_new(&_symbols_by_ref, 10);
    }
    return _symbols_by_ref;
}

/*
 * Delete a symbol from all tables and memory.
 * ASSUMES the CALLER has the LOCK.
 */
static void free_symbol(C_Symbol* sym) {
    C_Userdata key;
    if (!sym) return;

    if (symbols_by_ref()) {
        C_Userdata_set_pointer(&key, sym);
        C_Table_remove(symbols_by_ref(), &key);
    }
    if (sym->strbuf && symbols_by_name()) {
        // In case of nulls ...
        C_Userdata_set_value(&key, sym->strbuf, sym->strlen);
        C_Table_remove(symbols_by_name(), &key);
    }

    if (sym->strbuf) {
        free(sym->strbuf);
    }
    free(sym);
}

/*
 * Allocate a new symbol and add it to all tables.
 * ASSUMES the CALLER has the LOCK.
 */
static C_Symbol* symbol_alloc_init(size_t len, const uint8_t* uptr) {
    C_Userdata key, value;

    C_Symbol* result = (C_Symbol*)malloc(sizeof(C_Symbol));
    if (!result) goto error;

    bzero(result, sizeof(C_Symbol));

    if (uptr) {
        uint8_t* buf = (uint8_t*)calloc(len+1, sizeof(uint8_t));
        if (!buf) goto error;

        // Can't use strdup() because of possible embedded nulls
        // TODO: Does this handle "" properly?
        memcpy(buf, uptr, (len + 1) * sizeof(uint8_t));

        result->tenured = true;
        result->strlen = len;
        result->strbuf = buf;

        C_Userdata_set_value(&key, buf, len);
        C_Userdata_set_pointer(&value, result);

        if (!C_Table_add(symbols_by_name(), &key, &value)) goto error;
    }
    C_Userdata_set_pointer(&key, result);
    C_Userdata_set_pointer(&value, result);
    if (!C_Table_put(symbols_by_ref(), &key, &value)) goto error;

    goto finally;
error:
    free_symbol(result);
    goto finally;
finally:
    return result;
}

/*
 * Look up and return the symbol for a byte sequence.
 * If none exists, it creates a new symbol.
 * ASSUMES the CALLER has the LOCK.
 */
static C_Symbol* find_symbol(size_t len, const uint8_t* uptr, bool* isnew) {
    C_Symbol* result;
    C_Userdata key, value;

    if (!uptr) return NULL;

    if (!symbols_by_name()) return NULL;

    C_Userdata_set_value(&key, uptr, len);
    C_Userdata_clear(&value, false);
    C_Table_get(symbols_by_name(), &key, &value);

    if (value.ptr) {
        result = value.ptr;
        (*isnew) = false;
    } else {
        result = symbol_alloc_init(len, uptr);
        (*isnew) = (result != NULL);
    }
    return result;
}

extern bool is_C_Symbol(void* p) {
    C_Userdata key;
    bool result = false;

    if (!symbols_by_ref()) return false;

    lock_acquire();

    C_Userdata_set_pointer(&key, p);
    result = C_Table_has(symbols_by_ref(), &key);

    lock_release();

    return result;
}

extern void C_Symbol_new(C_Symbol* *symptr) {
    C_Symbol* sym;

    if (!symptr) return;

    lock_acquire();

    sym = symbol_alloc_init(0, NULL);

    lock_release();

    (*symptr) = C_Symbol_retain(sym);
}

extern bool C_Symbol_for_cstring(C_Symbol* *symptr, const char* cstr) {
    return C_Symbol_for_utf8_string(symptr, strlen(cstr), (const uint8_t*) cstr);
}

extern bool C_Symbol_for_utf8_string(C_Symbol* *symptr, size_t len, const uint8_t* uptr) {
    C_Symbol *sym;
    bool result = false;

    if (!symptr) return false;

    lock_acquire();

    sym = find_symbol(len, uptr, &result);

    lock_release();

    (*symptr) = C_Symbol_retain(sym);

    return result;
}

extern int C_Symbol_references(C_Symbol* sym) {
    int refcnt;

    if (!sym || !is_C_Symbol(sym)) return -255;

    refcnt = sym->refcnt;

    if (sym->tenured) {
        return (refcnt > 0) ? refcnt + 1 : 1;
    }
    return sym->refcnt;
}

extern C_Symbol* C_Symbol_retain(C_Symbol* sym) {
    if (!sym || !is_C_Symbol(sym)) return sym;

    sym->refcnt++;

    return sym;
}

extern void C_Symbol_release(C_Symbol* *symptr) {
    C_Symbol* sym;

    if (!symptr) return;
    sym = *symptr;

    if (!sym || !is_C_Symbol(sym)) return;

    sym->refcnt--;

    if (!sym->tenured && sym->refcnt <= 0) {
        lock_acquire();

        free_symbol(sym);

        lock_release();
    }

    (*symptr) = NULL;
}

extern C_Symbol* C_Symbol_set(C_Symbol* *lvalptr, C_Symbol* val) {
    if (lvalptr) {
        C_Symbol* oldval = (*lvalptr);

        (*lvalptr) = C_Symbol_retain(val);
        C_Symbol_release(&oldval);
    }
    return val;
}

extern const uint8_t* C_Symbol_as_utf8_string(C_Symbol* sym, size_t *lenptr) {
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

extern ssize_t C_Symbol_as_cstring(C_Symbol* sym, size_t len, char* buf) {
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
