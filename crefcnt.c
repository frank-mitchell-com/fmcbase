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
#include "cthread.h"
#include "ctable.h"
#include "crefset.h"
#include "creftbl.h"
#include "crefcnt.h"

/* -------------------------- PRIVATE FUNCTIONS -------------------------- */

static LOCK_DECL(_table_lock);

static C_Ref_Table* _reftable = NULL;
static C_Ref_Set*   _reflist  = NULL;
static C_Ref_Set*   _zeroset  = NULL;
static C_Ref_Table* _onzero   = NULL;

typedef struct C_Ref_Record {
    LOCK_TYPE(lock);
    const void* key;
    uint32_t    refcnt;
    bool        freed;
} C_Ref_Record;


static C_Ref_Table* reftable() {
    if (!_reftable) {
        C_Ref_Table_new(&_reftable, 11);
    }
    return _reftable;
}

static C_Ref_Set* reflist() {
    if (!_reflist) {
        C_Ref_Set_new(&_reflist, 11);
    }
    return _reflist;
}

static C_Ref_Set* zeroset() {
    if (!_zeroset) {
        C_Ref_Set_new(&_zeroset, 11);
    }
    return _zeroset;
}

static C_Ref_Table* onzerotbl() {
    if (!_onzero) {
        C_Ref_Table_new(&_onzero, 11);
    }
    return _onzero;
}

static C_Ref_Record* record(const void* obj) {
    return (C_Ref_Record*)C_Ref_Table_get(reftable(), obj);
}

static C_Ref_Record* add_record(const void* obj) {
    C_Ref_Record* rec = record(obj);

    if (rec != NULL) {
        return rec;
    }

    rec = malloc(sizeof(C_Ref_Record));
    if (!rec) {
        return NULL;
    }

    LOCK_INIT(rec->lock);
    rec->key    = obj;
    rec->refcnt = 1;
    rec->freed  = false;

    if (!C_Ref_Table_put(reftable(), obj, rec, NULL)) {
        LOCK_FREE(rec->lock);
        free(rec);
        rec = NULL;
    }
    return rec;
}

static void remove_record(C_Ref_Record* rec) {
    if (rec) {
        C_Ref_Table_remove(reftable(), rec->key, NULL);
    }
}

/* ---------------------------- API FUNCTIONS ---------------------------- */

extern uint32_t C_Ref_Count_refcount(const void* obj) {
    uint32_t result = 1;
    C_Ref_Record* rec = NULL;

    LOCK_ACQUIRE(_table_lock);

    if (C_Ref_Set_has(zeroset(), obj)) {
        result = 0;
    } else {
        rec = record(obj);
        if (!rec) {
            result = 1;
        }
    }

    LOCK_RELEASE(_table_lock);

    if (rec != NULL) {
        LOCK_ACQUIRE(rec->lock);

        result = rec->refcnt;

        LOCK_RELEASE(rec->lock);
    }


    return result;
}

extern uint32_t C_Ref_Count_decrement(const void* obj) {
    uint32_t result;
    C_Ref_Record* rec = NULL;
    C_On_Zero_Fcn onzero = NULL;

    LOCK_ACQUIRE(_table_lock);

    rec = record(obj);
    if (!rec) {
        // Reference count is 1 or less; add to "zero set"
        C_Ref_Set_add(zeroset(), obj);
        result = 0;

        onzero = (C_On_Zero_Fcn)C_Ref_Table_get(onzerotbl(), obj);
    }

    LOCK_RELEASE(_table_lock);

    if (onzero) {
        C_Ref_Count_delist(obj);
        onzero((void *)obj);
    }

    if (rec != NULL) {
        LOCK_ACQUIRE(rec->lock);

        if (rec->refcnt > 0) {
            rec->refcnt--;
        }
        result = rec->refcnt;

        if (rec->refcnt <= 1) {
            // Remove the record from the table to signify a count of 1
            // and to optimize memory usage.
            // Most objects stay at 1 for their entire lifetime,
            // so using the lack of a record to signify 1
            // saves a lot of space in the table.
            remove_record(rec);
            rec->freed = true;
        }

        if (rec->freed) {
            // Must free "immediately" after release, according to spec.
            LOCK_RELEASE(rec->lock);
            LOCK_FREE(rec->lock);
            free(rec);
            rec = NULL;
        } else {
            LOCK_RELEASE(rec->lock);
        }
    }

    return result;
}

extern uint32_t C_Ref_Count_increment(const void* obj) {
    uint32_t result = 0;
    C_Ref_Record* rec = NULL;

    LOCK_ACQUIRE(_table_lock);

    if (C_Ref_Set_has(zeroset(), obj)) {
        // Reference count was zero, somehow.
        // Bump it back to 1 by removing the object from the Zero Set.
        C_Ref_Set_add(zeroset(), obj);
        result = 1;
    } else {
        rec = record(obj);
        if (!rec) {
            // Reference count is effectively 1.
            // Create a new record for this object, with reference
            // count set to 1
            rec = add_record(obj);
        }
    }

    LOCK_RELEASE(_table_lock);

    if (rec != NULL) {
        LOCK_ACQUIRE(rec->lock);

        // TODO: Check if less than maxint?
        rec->refcnt++;
        result = rec->refcnt;

        LOCK_RELEASE(rec->lock);
    }

    return result;
}

extern bool C_Ref_Count_is_listed(const void* obj) {
    bool result = false;

    LOCK_ACQUIRE(_table_lock);

    result = C_Ref_Set_has(reflist(), obj);

    LOCK_RELEASE(_table_lock);

    return result;
}

extern void C_Ref_Count_list(const void* obj) {
    LOCK_ACQUIRE(_table_lock);

    C_Ref_Set_add(reflist(), obj);

    LOCK_RELEASE(_table_lock);
}

extern void C_Ref_Count_delist(const void* obj) {
    C_Ref_Record* rec = NULL;

    LOCK_ACQUIRE(_table_lock);

    C_Ref_Set_remove(reflist(), obj);
    C_Ref_Set_remove(zeroset(), obj);
    C_Ref_Table_remove(onzerotbl(), obj, NULL);

    rec = record(obj);

    LOCK_RELEASE(_table_lock);

    if (rec != NULL) {
        LOCK_ACQUIRE(rec->lock);

        remove_record(rec);

        rec->freed = true;

        LOCK_RELEASE(rec->lock);

        LOCK_FREE(rec->lock);

        free(rec);
    }
}

extern void C_Ref_Count_on_zero(const void* p, C_On_Zero_Fcn onzero) {
    LOCK_ACQUIRE(_table_lock);
    C_Ref_Table_put(onzerotbl(), p, onzero, NULL);
    LOCK_RELEASE(_table_lock);
}

/* ---------------------------- HELPER FUNCTIONS ---------------------------- */

const void* C_Any_retain(const void* p) {
    if (p == NULL || !C_Ref_Count_is_listed(p)) {
        return NULL;
    }
    C_Ref_Count_increment(p);
    return p;
}

bool C_Any_release(const void* *pptr) {
    if (!pptr || !(*pptr) || !C_Ref_Count_is_listed(*pptr)) {
        return false;
    }

    C_Ref_Count_decrement(*pptr);
    *pptr = NULL;
    return true;
}

void C_Any_set(const void* *lvalue, const void* value) {
    const void* oldvalue;
    if (!lvalue) {
        return;
    }
    oldvalue = *lvalue;
    *lvalue = value;
    C_Any_retain(value);
    C_Any_release(&oldvalue);
}

