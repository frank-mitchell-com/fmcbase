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

#ifndef FMC_THREAD_H_INCLUDED
#define FMC_THREAD_H_INCLUDED

/** @file
 * Macros for thread portability.
 */

#include <pthread.h>

#define LOCK_DECL(x)        pthread_mutex_t (x) = PTHREAD_MUTEX_INITIALIZER
#define LOCK_TYPE(x)        pthread_mutex_t (x)
#define LOCK_INIT(x)        pthread_mutex_init(&(x), NULL)
#define LOCK_ACQUIRE(x)     pthread_mutex_lock(&(x))
#define LOCK_RELEASE(x)     pthread_mutex_unlock(&(x))
#define LOCK_FREE(x)        pthread_mutex_destroy(&(x))

#define COND_DECL(x)        pthread_cond_t(x) = PTHREAD_COND_INITIALIZER
#define COND_TYPE(x)        pthread_cond_t(x)
#define COND_INIT(x)        pthread_cond_init(&(x), NULL)
#define COND_WAIT(x, m)     pthread_cond_wait(&(x), &(m))
#define COND_SIGNAL(x)      pthread_cond_signal(&(x))
#define COND_SIGNAL_ALL(x)  pthread_cond_broadcast(&(x));
#define COND_FREE(x)        pthread_cond_destroy(&(x))

#define RWLOCK_DECL(x)      pthread_rwlock_t (x) = PTHREAD_RWLOCK_INITIALIZER
#define RWLOCK_TYPE(x)      pthread_rwlock_t (x)
#define RWLOCK_INIT(x)      pthread_rwlock_init(&(x), NULL)
#define RWLOCK_ACQ_READ(x)  pthread_rwlock_rdlock(&(x))
#define RWLOCK_ACQ_WRITE(x) pthread_rwlock_wrlock(&(x))
#define RWLOCK_RELEASE(x)   pthread_rwlock_unlock(&(x))
#define RWLOCK_FREE(x)      pthread_rwlock_destroy(&(x))

#endif // FMC_THREAD_H_INCLUDED

