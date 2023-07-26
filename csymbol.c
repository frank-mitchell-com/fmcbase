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

static volatile atomic_bool _c_symbol_init = false;

static C_Table* _symbols_by_name = NULL;
static C_Table* _symbols_by_ref  = NULL;

struct _C_Symbol {
    volatile atomic_int refcnt;

    /* not changed after creation */
    bool    tenured;
    size_t  strlen;
    uint8_t *strbuf;
};

static void C_Symbol_Lock_acquire() {
    // Assume re-entrant lock so multiple calls simply increment a count
}

static void C_Symbol_Lock_release() {
    // Assume re-entrant lock so multiple calls simply decrement a count
}

/*
 * Initialize global resources: symbols by name, symbols by reference,
 * mutexes, etc.
 */
static void C_Symbol_global_init() {
    if (!_c_symbol_init) {
        _c_symbol_init = true;

        C_Symbol_Lock_acquire();

        C_Table_new(&_symbols_by_name,  50);
        if (!_symbols_by_name) {
            _c_symbol_init = false;
            goto finally;
        }

        C_Table_new(&_symbols_by_ref,  100);
        if (!_symbols_by_ref) {
            _c_symbol_init = false;
            C_Table_free(&_symbols_by_name);
            goto finally;
        }

        _c_symbol_init = true;
finally:
        C_Symbol_Lock_release();
    }
}

/*
 * Delete a symbol from all tables and memory.
 */
static void C_Symbol_free(C_Symbol* sym) {
    if (!sym) return;

    C_Symbol_Lock_acquire();
    do {
        C_Userdata key;
        if (_symbols_by_ref) {
            C_Userdata_set_pointer(&key, sym);
            C_Table_remove(_symbols_by_ref, &key);
        }
        if (sym->strbuf && _symbols_by_name) {
            // In case of nulls ...
            C_Userdata_set_value(&key, sym->strbuf, sym->strlen);
            C_Table_remove(_symbols_by_name, &key);
        }
    } while (0);
    C_Symbol_Lock_release();

    if (sym->strbuf) {
        free(sym->strbuf);
    }
    free(sym);
}

/*
 * Allocate a new symbol and add it to all tables.
 */
static C_Symbol* C_Symbol_alloc(size_t len, const uint8_t* uptr) {
    C_Userdata key, value;

    C_Symbol_global_init();

    C_Symbol_Lock_acquire();

    C_Symbol* result = (C_Symbol*)malloc(sizeof(C_Symbol));
    if (!result) goto error;

    bzero(result, sizeof(C_Symbol));

    if (uptr) {
        uint8_t* buf = (uint8_t*)calloc(len+1, sizeof(uint8_t));
        if (!buf) goto error;

        // Can't use strdup() because of possible embedded nulls
        // TODO: Does this handle "" properly?
        memcpy(buf, uptr, len * sizeof(uint8_t));

        result->tenured = true;
        result->strlen = len;
        result->strbuf = buf;

        // C_Table will make copies anyway, so ...
        C_Userdata_set_value(&key, uptr, len);
        C_Userdata_set_pointer(&value, result);

        if (!C_Table_add(_symbols_by_name, &key, &value)) goto error;
    }
    C_Userdata_set_pointer(&key, result);
    C_Userdata_set_pointer(&value, result);
    if (!C_Table_put(_symbols_by_ref, &key, &value)) goto error;

    goto finally;
error:
    C_Symbol_free(result);
    goto finally;
finally:
    C_Symbol_Lock_release();
    return result;
}

/*
 * Look up and return the symbol for a byte sequence.
 */
static C_Symbol* C_Symbol_lookup(size_t len, const uint8_t* uptr) {
    C_Userdata key, result;

    if (!uptr) return NULL;

    if (!_symbols_by_name) return NULL;

    C_Symbol_Lock_acquire();
    do {
        C_Userdata_set(&key, 0, len, uptr);
        C_Userdata_clear(&result, false);

        C_Table_get(_symbols_by_ref, &key, &result);
    } while (0);
    C_Symbol_Lock_release();

    return result.ptr;
}

bool is_C_Symbol(void* p) {
    bool result = false;

    if (!_symbols_by_ref) return false;

    C_Symbol_Lock_acquire();
    do {
        C_Userdata key;
        C_Userdata_set_pointer(&key, p);
        result = C_Table_has(_symbols_by_ref, &key);
    } while (0);
    C_Symbol_Lock_release();

    return result;
}

void C_Symbol_new(C_Symbol* *symptr) {
    if (!symptr) return;
    C_Symbol_set(symptr, C_Symbol_alloc(0, NULL));
}

bool C_Symbol_for_cstring(C_Symbol* *symptr, const char* cstr) {
    return C_Symbol_for_utf8_string(symptr, strlen(cstr), (const uint8_t*) cstr);
}

bool C_Symbol_for_utf8_string(C_Symbol* *symptr, size_t len, const uint8_t* uptr) {
    C_Symbol* sym = NULL;
    bool result = false;

    if (!symptr) return false;

    C_Symbol_Lock_acquire();
    do {
        sym = C_Symbol_lookup(len, uptr);
        if (!result) {
            sym = C_Symbol_alloc(len, uptr);
            result = (sym != NULL);
        }
    } while (0);
    C_Symbol_Lock_release();

    C_Symbol_set(symptr, sym);
    return result;
}

C_Symbol* C_Symbol_retain(C_Symbol* sym) {
    if (!sym || !is_C_Symbol(sym)) return sym;

    if (!sym->tenured) {
        sym->refcnt++;
    }
    return sym;
}

void C_Symbol_release(C_Symbol* *symptr) {
    C_Symbol* sym;

    if (!symptr) return;
    sym = *symptr;

    if (!sym || !is_C_Symbol(sym)) return;

    if (!sym->tenured) {
        sym->refcnt--;
        if (sym->refcnt == 0) {
            C_Symbol_free(sym);
            (*symptr) = NULL;
        }
    }
}

C_Symbol* C_Symbol_set(C_Symbol* *lvalptr, C_Symbol* val) {
    C_Symbol* result = C_Symbol_retain(val);
    if (lvalptr) {
        (*lvalptr) = val;
    }
    C_Symbol_release(lvalptr);
    return result;
}

const uint8_t* C_Symbol_as_utf8_string(C_Symbol* sym, size_t *lenptr) {
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

ssize_t C_Symbol_as_cstring(C_Symbol* sym, size_t len, char* buf) {
    ssize_t result;

    if (!sym || !is_C_Symbol(sym)) return -1;

    // Can't use strcpy because we have to scan for nulls
    result = 0;
    for (int i = 0; i < sym->strlen && result < len; i++) {
        uint8_t c = sym->strbuf[i];
        if (c) {
            buf[result] = c;
            result++;
        } else {
            // TODO: Check that we're not at the end of buf
            buf[result] = 0xC0;
            buf[result+1] = 0x80;
            result += 2;
        }
    }
    return result;
}
