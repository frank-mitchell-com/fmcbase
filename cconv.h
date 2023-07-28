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

#ifndef CCONV_H_INCLUDED
#define CCONV_H_INCLUDED

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

bool C_Conv_is_ascii(size_t sz, uint8_t* buf);

size_t C_Conv_utf8_to_16_length(size_t sz, uint8_t* buf, size_t *csz);

size_t C_Conv_utf8_to_32_length(size_t sz, uint8_t* buf, size_t *csz);

size_t C_Conv_utf16_to_8_length(size_t sz, uint16_t* buf, size_t *csz);

size_t C_Conv_utf32_to_8_length(size_t sz, uint32_t* buf, size_t *csz);

size_t C_Conv_utf8_to_16(size_t insz, uint8_t* inbuf, 
                        size_t outsz, uint16_t* outbuf);

size_t C_Conv_utf8_to_32(size_t insz, uint8_t* inbuf, 
                        size_t outsz, uint32_t* outbuf);

size_t C_Conv_utf16_to_8(size_t insz, uint16_t* inbuf, 
                        size_t outsz, uint8_t* outbuf);

size_t C_Conv_utf32_to_8(size_t insz, uint32_t* inbuf,
                        size_t outsz, uint8_t* outbuf);

ssize_t C_Conv_decode_utf8(const char* encoding,
                            size_t insz, uint8_t* inbuf,
                            size_t outsz, uint8_t* outbuf);

ssize_t C_Conv_decode_utf32(const char* encoding,
                            size_t insz, uint8_t* inbuf,
                            size_t outsz, uint32_t* outbuf);

ssize_t C_Conv_encode_utf8(const char* encoding,
                            size_t insz, uint8_t* inbuf,
                            size_t outsz, uint8_t* outbuf);

ssize_t C_Conv_encode_utf32(const char* encoding,
                            size_t insz, uint32_t* inbuf,
                            size_t outsz, uint8_t* outbuf);

ssize_t C_Conv_transcode(const char* incode, 
                        const char* outcode,
                        size_t insz, uint8_t* inbuf,
                        size_t outsz, uint8_t* outbuf);

#endif // CCONV_H_INCLUDED
