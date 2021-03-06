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

#include "internal.h"
#include "hashtable.h"

#define HT_DEFAULT_ALLOCATOR \
    {                        \
        .malloc = malloc,    \
        .free = free,        \
        .calloc = calloc,    \
        .realloc = realloc   \
    }


static const struct ht_memory_allocator ht_default_allocator =
    HT_DEFAULT_ALLOCATOR;

static struct ht_memory_allocator ht_allocator = HT_DEFAULT_ALLOCATOR;

struct ht_memory_allocator *ht_default_memory_allocator;

void
ht_set_memory_allocator(const struct ht_memory_allocator *allocator) {
    if (allocator) {
        ht_allocator = *allocator;
    } else {
        ht_allocator = ht_default_allocator;
    }
}

void *
ht_malloc(size_t sz) {
    return ht_allocator.malloc(sz);
}

void
ht_free(void *ptr) {
    ht_allocator.free(ptr);
}

void *
ht_calloc(size_t nb, size_t sz) {
    return ht_allocator.calloc(nb, sz);
}

void *
ht_realloc(void *ptr, size_t sz) {
    return ht_allocator.realloc(ptr, sz);
}
