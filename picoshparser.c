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
#include <assert.h>
#include "picoshparser.h"

#if __GNUC__ >= 3
#define PSR_LIKELY(x) __builtin_expect(!!(x), 1)
#define PSR_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define PSR_LIKELY(x) (x)
#define PSR_UNLIKELY(x) (x)
#endif

static inline int is_end(psr_parse_context_t *ctx)
{
    return ctx->_input == ctx->_end;
}

static inline int peek_ch(psr_parse_context_t *ctx)
{
    if (is_end(ctx))
        return -1;
    return *ctx->_input;
}

static inline int get_ch(psr_parse_context_t *ctx)
{
    if (is_end(ctx))
        return -1;
    return *ctx->_input++;
}

static inline void unget_ch(psr_parse_context_t *ctx)
{
    --ctx->_input;
}

static inline int skip_midspace(psr_parse_context_t *ctx)
{
    int ch;
    while ((ch = get_ch(ctx)) == ' ')
        ;
    if (ch == -1)
        return 0;
    unget_ch(ctx);
    return 1;
}

static int skip_rest_of_member(psr_parse_context_t *ctx)
{
    /* FIXME */
    while (1) {
        switch (get_ch(ctx)) {
        case ',':
            unget_ch(ctx);
            return 1;
        case -1:
            return 1;
        default:
            break;
        }
    }
}

static int parse_key(psr_parse_context_t *ctx, const char **key, size_t *key_len)
{
    int ch;

    /* first char is lcalpha */
    *key = ctx->_input;
    ch = get_ch(ctx);
    if (PSR_UNLIKELY(!('a' <= ch && ch <= 'z')))
        return 0;
    /* following chars are lcalpha / DIGIT / "_" / "-" / "*" */
    do {
        ch = get_ch(ctx);
    } while (('a' <= ch && ch <= 'z') || ('0' <= ch && ch <= '9') || ch == '_' || ch == '-' || ch == '*');
    if (ch == -1)
        return 0;
    unget_ch(ctx);
    *key_len = ctx->_input - *key;
    return 1;
}

static inline void do_parse_dictionary(psr_parse_context_t *ctx, int first_call)
{
    if (!first_call) {
        /* skip to the end of member */
        if (!skip_rest_of_member(ctx))
            goto Fail;
        /* finish processing when the input terminates */
        if (is_end(ctx))
            goto End;
        /* *SP "," *SP */
        if (!skip_midspace(ctx))
            goto Fail;
        if (get_ch(ctx) != ',')
            goto Fail;
        if (!skip_midspace(ctx))
            goto Fail;
    }

    /* parse key "=" */
    if (!parse_key(ctx, &ctx->key, &ctx->key_len))
        goto Fail;
    if (get_ch(ctx) != '=')
        goto Fail;

    return;
Fail:
    ctx->key = NULL;
    ctx->key_len = 0;
    return;
End:
    ctx->done = 1;
    return;
}

void psr_parse_dictionary_first(psr_parse_context_t *ctx, const char *field_value, size_t field_len)
{
    *ctx = (psr_parse_context_t){
        ._input = field_value,
        ._end   = field_value + field_len,
    };
    do_parse_dictionary(ctx, 1);
}

void psr_parse_dictionary_next(psr_parse_context_t *ctx)
{
    do_parse_dictionary(ctx, 0);
}

int psr_parse_int_part(psr_parse_context_t *ctx, int64_t *value)
{
    int ch, is_negative = 0;

    if (PSR_UNLIKELY(peek_ch(ctx) == '-')) {
        get_ch(ctx);
        is_negative = 1;
    }

    ch = get_ch(ctx);
    if (PSR_UNLIKELY(!('0' <= ch && ch <= '9')))
        return 0;
    *value = ch - '0';
    while (1) {
        ch = get_ch(ctx);
        if (!('0' <= ch && ch <= '9'))
            break;
        *value = *value * 10 + ch - '0';
    }
    if (ch == -1)
        return 0;
    unget_ch(ctx);

    if (is_negative)
        *value = -*value;
    return 1;
}

int psr_parse_bool_part(psr_parse_context_t *ctx, int *value)
{
    if (PSR_UNLIKELY(get_ch(ctx) != '?'))
        return 0;
    switch (get_ch(ctx)) {
    case '0':
        *value = 0;
        break;
    case '1':
        *value = 1;
        break;
    default:
        return 0;
    }
    return 1;
}
