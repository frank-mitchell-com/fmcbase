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

#define UTF_8   "UTF-8"
#define UTF_32  "UTF-32"

typedef uint8_t  octet_t;
typedef uint8_t  utf8_t;
typedef uint16_t utf16_t;
typedef wchar_t  utf32_t;


/*
 * TODO: Adapt this for Microsoft DLL commands.
 */

#define FMC_API extern

#endif // FMC_COMMON_INCLUDED