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

#ifndef LIBHASHTABLE_HASHTABLE_H
#define LIBHASHTABLE_HASHTABLE_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define HT_INT32_TO_POINTER(i_) ((void *)(intptr_t)(int32_t)(i_))
#define HT_POINTER_TO_INT32(p_) ((int32_t)(intptr_t)(p_))

struct ht_memory_allocator {
   void *(*malloc)(size_t);
   void (*free)(void *);
   void *(*calloc)(size_t, size_t);
   void *(*realloc)(void *, size_t);
};

extern struct ht_memory_allocator *ht_default_memory_allocator;

typedef uint32_t (*ht_hash_func)(const void *);
typedef bool (*ht_equal_func)(const void *, const void *);

const char *ht_version(void);
const char *ht_build_id(void);

const char *ht_get_error(void);

void ht_set_memory_allocator(const struct ht_memory_allocator *);

struct ht_table *ht_table_new(ht_hash_func, ht_equal_func);
void ht_table_delete(struct ht_table *);
size_t ht_table_nb_entries(const struct ht_table *);
bool ht_table_is_empty(const struct ht_table *);
void ht_table_clear(struct ht_table *);
int ht_table_insert(struct ht_table *, void *, void *);
int ht_table_insert2(struct ht_table *, void *, void *, void **, void **);
int ht_table_remove(struct ht_table *, const void *);
int ht_table_remove2(struct ht_table *, const void *, void **, void **);
int ht_table_get(struct ht_table *, const void *, void **);
bool ht_table_contains(struct ht_table *, const void *);
void ht_table_print(struct ht_table *, FILE *);

struct ht_table_iterator *ht_table_iterate(struct ht_table *);
void ht_table_iterator_delete(struct ht_table_iterator *);
int ht_table_iterator_next(struct ht_table_iterator *, void **, void **);
void ht_table_iterator_remove(struct ht_table_iterator *);
void ht_table_iterator_set_value(struct ht_table_iterator *, void *);

uint32_t ht_hash_int32(const void *);
bool ht_equal_int32(const void *, const void *);

uint32_t ht_hash_string(const void *);
bool ht_equal_string(const void *, const void *);

#endif
