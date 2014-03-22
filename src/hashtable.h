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
   void *(*malloc)(size_t sz);
   void (*free)(void *ptr);
   void *(*calloc)(size_t nb, size_t sz);
   void *(*realloc)(void *ptr, size_t sz);
};

extern struct ht_memory_allocator *ht_default_memory_allocator;

typedef uint32_t (*ht_hash_func)(const void *key);
typedef bool (*ht_equal_func)(const void *k1, const void *k2);


const char *ht_get_error(void);

void ht_set_memory_allocator(const struct ht_memory_allocator *allocator);

struct ht_table *ht_table_new(ht_hash_func hash_func,
                              ht_equal_func equal_func);
void ht_table_delete(struct ht_table *table);
size_t ht_table_get_nb_entries(const struct ht_table *table);
bool ht_table_is_empty(const struct ht_table *table);
void ht_table_clear(struct ht_table *table);
int ht_table_insert(struct ht_table *table, void *key, void *value);
int ht_table_insert2(struct ht_table *table, void *key, void *value,
                     void **old_key, void **old_value);
int ht_table_remove(struct ht_table *table, const void *key);
int ht_table_remove2(struct ht_table *table, const void *key,
                     void **old_key, void **old_value);
int ht_table_get(struct ht_table *table, const void *key, void **value);
bool ht_table_contains(struct ht_table *table, const void *key);
void ht_table_print(struct ht_table *table, FILE *file);

struct ht_table_iterator *ht_table_iterate(struct ht_table *table);
void ht_table_iterator_delete(struct ht_table_iterator *it);
int ht_table_iterator_get_next(struct ht_table_iterator *it,
                               void **key, void **value);
void ht_table_iterator_remove(struct ht_table_iterator *it);
void ht_table_iterator_set_value(struct ht_table_iterator *it,
                                 void *value);

uint32_t ht_hash_int32(const void *key);
bool ht_equal_int32(const void *k1, const void *k2);

uint32_t ht_hash_string(const void *key);
bool ht_equal_string(const void *k1, const void *k2);

#endif
