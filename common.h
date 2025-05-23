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

#ifndef FMC_COMMON_INCLUDED
#define FMC_COMMON_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <wchar.h>

/**
 * Type for uncategorized byte data.
 */
typedef uint8_t  octet_t;

/**
 * Type for UTF-8 bytes.
 */
typedef uint8_t  char8_t;

/**
 * Type for UTF-16 2-byte characters.
 */
typedef uint16_t char16_t;

/**
 * Type for UTF-32 4-byte characters.
 */
typedef uint32_t char32_t;


/*
 * TODO: Adapt this for Microsoft DLL commands.
 */

#define FMC_API extern

#endif // FMC_COMMON_INCLUDED
