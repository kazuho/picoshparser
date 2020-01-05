/*
* Copyright (c) 2020 Fastly, Kazuho Oku
*
* The software is licensed under either the MIT License (below) or the Perl
* license.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to
* deal in the Software without restriction, including without limitation the
* rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/
#ifndef picoshparser_h
#define picoshparser_h

#include <stddef.h>
#include <stdint.h>

typedef struct psr_parse_context {
    const char *_input, *_end;
    int _is_next_call;
} psr_parse_context_t;

static void psr_init_parse_context(psr_parse_context_t *ctx, const char *s, size_t len);
int psr_parse_dictionary(psr_parse_context_t *ctx, const char **key, size_t *key_len);
int psr_parse_int_member(psr_parse_context_t *ctx, int64_t *value);
int psr_parse_bool_member(psr_parse_context_t *ctx, int *value);
int psr_parse_parameter(psr_parse_context_t *ctx);

/* inline definitions */

inline void psr_init_parse_context(psr_parse_context_t *ctx, const char *s, size_t len)
{
    *ctx = (psr_parse_context_t){
        ._input = s,
        ._end   = s + len,
    };
}

#endif

