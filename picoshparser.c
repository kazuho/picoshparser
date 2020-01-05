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

static int advance_to_next_toplevel(psr_parse_context_t *ctx, int offending_ch)
{
    int ch = offending_ch;

    /* skip SP* */
    while (ch == ' ')
        get_ch(ctx);
    /* should be ',' or EOS (TODO handle ";", ")") */
    switch (ch) {
    case ',':
        ctx->state = PSR_STATE_TOPLEVEL;
        break;
    case -1:
        ctx->state = PSR_STATE_COMPLETE;
        return 1;
    default:
        return 0;
    }
    /* skip SP* */
    while ((ch = get_ch(ctx)) == ' ')
        ;
    if (ch == -1)
        return 0;
    unget_ch(ctx);

    return 1;
}

static int parse_key(psr_parse_context_t *ctx, const char **key, size_t *key_len)
{
    int ch;

    switch (ctx->state) {
    case PSR_STATE_TOPLEVEL:
        break;
    case PSR_STATE_DICT_VALUE:
        /* advance to next (FIXME) */
        while ((ch = get_ch(ctx)) != ',') {
            if (ch == -1)
                return 0;
        }
        if (!advance_to_next_toplevel(ctx, ch))
            return 0;
        break;
    default:
        return 0;
    }

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

const char *psr_complex__next_key(psr_parse_context_t *ctx, size_t *key_len)
{
    const char *key;

    /* parse key "=" */
    if (!parse_key(ctx, &key, key_len))
        goto Fail;
    if (get_ch(ctx) != '=')
        goto Fail;

    ctx->state = PSR_STATE_DICT_VALUE;
    return key;
Fail:
    return NULL;
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
    if (is_negative)
        *value = -*value;

    return advance_to_next_toplevel(ctx, ch);
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
    return advance_to_next_toplevel(ctx, get_ch(ctx));
}
