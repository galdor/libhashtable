/*
 * Copyright (c) 2013-2014 Nicolas Martyanoff
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "internal.h"
#include "hashtable.h"

#define HT_ERROR_BUFSZ 1024U

static __thread char ht_error_buf[HT_ERROR_BUFSZ];

const char *
ht_get_error(void) {
    return ht_error_buf;
}

void
ht_set_error(const char *fmt, ...) {
    char buf[HT_ERROR_BUFSZ];
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = vsnprintf(buf, HT_ERROR_BUFSZ, fmt, ap);
    va_end(ap);

    if ((size_t)ret >= HT_ERROR_BUFSZ)
        ret = HT_ERROR_BUFSZ - 1;

    memcpy(ht_error_buf, buf, (size_t)ret);
    ht_error_buf[ret] = '\0';
}
