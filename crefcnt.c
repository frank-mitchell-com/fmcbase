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

#include <pthread.h>
#include <string.h>
#include <strings.h>
#include <stdatomic.h>
#include <stdlib.h>
#include "cthread.h"
#include "ctable.h"
#include "crefset.h"
#include "crefcnt.h"

/* -------------------------- PRIVATE FUNCTIONS -------------------------- */

static LOCK_DECL(_table_lock);

static C_Table*   _reftable = NULL;
static C_Ref_Set* _reflist  = NULL;
static C_Ref_Set* _zeroset  = NULL;

typedef struct C_Ref_Record {
    LOCK_TYPE(lock);
    const void* key;
    uint32_t    refcnt;
} C_Ref_Record;


static C_Table* reftable() {
    if (!_reftable) {
        C_Table_new(&_reftable, 10);
    }
    return _reftable;
}

static C_Ref_Set* reflist() {
    if (!_reflist) {
        C_Ref_Set_new(&_reflist, 10);
    }
    return _reflist;
}

static C_Ref_Set* zeroset() {
    if (!_zeroset) {
        C_Ref_Set_new(&_zeroset, 10);
    }
    return _zeroset;
}

static C_Ref_Record* record(const void* obj) {
    C_Userdata key, value;

    C_Userdata_set_pointer(&key, obj);
    C_Userdata_clear(&value, false);

    C_Table_get((reftable()), &key, &value);

    return (C_Ref_Record*)value.ptr;
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

    C_Userdata key, value;
    C_Userdata_set_pointer(&key, obj);
    C_Userdata_set_pointer(&value, rec);
    if (!C_Table_add(reftable(), &key, &value)) {
        LOCK_FREE(rec->lock);
        free(rec);
        rec = NULL;
    }
    return rec;
}

static void remove_record(C_Ref_Record* rec) {
    if (rec) {
        C_Userdata key;
        C_Userdata_set_pointer(&key, rec->key);
        C_Table_remove(reftable(), &key);
    }
}

/* ---------------------------- API FUNCTIONS ---------------------------- */

extern uint32_t C_Ref_Count_refcount(const void* obj) {
    uint32_t result;
    C_Ref_Record* rec;

    LOCK_ACQUIRE(_table_lock);

    if (C_Ref_Set_has(zeroset(), obj)) {
        result = 0;
    } else {
        rec = record(obj);
        if (!rec) {
            result = 1;
        } else {
            LOCK_ACQUIRE(rec->lock);

            result = rec->refcnt;

            LOCK_RELEASE(rec->lock);
        }
    }

    LOCK_RELEASE(_table_lock);

    return result;
}

extern uint32_t C_Ref_Count_decrement(const void* obj) {
    uint32_t result;
    C_Ref_Record* rec;

    LOCK_ACQUIRE(_table_lock);

    rec = record(obj);
    if (!rec) {
        // Reference count is 1 or less; add to "zero set"
        C_Ref_Set_add(zeroset(), obj);
        result = 0;
    } else {
        bool freerec = false;

        LOCK_ACQUIRE(rec->lock);

        rec->refcnt--;
        result = rec->refcnt;

        if (rec->refcnt <= 1) {
            // Remove the record from the table to signify a count of 1
            // and to optimize memory usage.
            // Most objects stay at 1 for their entire lifetime,
            // so using the lack of a record to signify 1
            // saves a lot of space in the table.
            remove_record(rec);
            freerec = true;
        }

        LOCK_RELEASE(rec->lock);

        if (freerec) {
            LOCK_FREE(rec->lock);
            free(rec);
            rec = NULL;
        }
    }

    LOCK_RELEASE(_table_lock);

    return result;
}

extern uint32_t C_Ref_Count_increment(const void* obj) {
    uint32_t result = 0;
    C_Ref_Record* rec;

    LOCK_ACQUIRE(_table_lock);

    if (C_Ref_Set_has(zeroset(), obj)) {
        // Reference count was zero, somehow.
        // Bump it back to 1 by removing the object from the Zero Set.
        C_Ref_Set_add(zeroset(), obj);
    } else {
        rec = record(obj);
        if (!rec) {
            // Reference count is effectively 1.
            // Create a new record for this object, with reference
            // count set to 1
            rec = add_record(obj);
        }
        LOCK_ACQUIRE(rec->lock);

        // TODO: Check if less than maxint?
        rec->refcnt++;
        result = rec->refcnt;

        LOCK_RELEASE(rec->lock);
    }

    LOCK_RELEASE(_table_lock);

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
    C_Ref_Record* rec;

    LOCK_ACQUIRE(_table_lock);

    C_Ref_Set_remove(reflist(), obj);
    C_Ref_Set_remove(zeroset(), obj);

    rec = record(obj);
    if (rec) {
        LOCK_ACQUIRE(rec->lock);

        remove_record(rec);

        LOCK_RELEASE(rec->lock);
        LOCK_FREE(rec->lock);

        free(rec);
    }

    LOCK_RELEASE(_table_lock);
}
