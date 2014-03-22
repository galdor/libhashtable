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

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "hashtable.h"

static void die(const char *, ...)
    __attribute__((format(printf, 1, 2)));
static void usage(const char *, int);

#define TEST_DEFINE(name_) \
    int test_case_##name_()

#define TEST_SUCCEED() \
    return 1

#define TEST_FAIL(fmt_, ...)                  \
    do {                                      \
        fprintf(stderr, fmt_"\n", ##__VA_ARGS__); \
        return 0;                             \
    } while (0)

#define TEST_ERROR(fmt_, ...)                 \
    do {                                      \
        fprintf(stderr, fmt_"\n", ##__VA_ARGS__); \
        return -1;                            \
    } while (0)

#define TEST_ASSERT(cond_, fmt_, ...)                                     \
    if (!(cond_)) {                                                       \
        TEST_FAIL("assertion '" #cond_ "' failed: " fmt_, ##__VA_ARGS__); \
    }


TEST_DEFINE(insert) {
    struct ht_table *table;
    const char *str;

    table = ht_table_new(ht_hash_string, ht_equal_string);

    TEST_ASSERT(ht_table_is_empty(table),
                "a newly created table should be empty");

    ht_table_insert(table, "a", "abc");

    TEST_ASSERT(ht_table_get_nb_entries(table) == 1,
                "invalid number of entries after 1 insertion");

    ht_table_insert(table, "d", "def");
    ht_table_insert(table, "g", "ghi");

    TEST_ASSERT(ht_table_get_nb_entries(table) == 3,
                "invalid number of entries after 3 insertions");

    TEST_ASSERT(ht_table_get(table, "a", (void **)&str) == 1,
                "cannot fetch entry 'a'");
    TEST_ASSERT(strcmp(str, "abc") == 0,
                "invalid value for entry 'a'");

    TEST_ASSERT(ht_table_get(table, "g", (void **)&str) == 1,
                "cannot fetch element");
    TEST_ASSERT(strcmp(str, "ghi") == 0,
                "invalid value for entry 'g'");

    ht_table_insert(table, "g", "foo");

    TEST_ASSERT(ht_table_get(table, "g", (void **)&str) == 1,
                "cannot fetch element");
    TEST_ASSERT(strcmp(str, "foo") == 0,
                "invalid value for entry 'g' after update");

    TEST_ASSERT(ht_table_get_nb_entries(table) == 3,
                "invalid number of entries after entry update");

    TEST_ASSERT(ht_table_get(table, "k", (void **)&str) == 0,
                "invalid return value when fetching an entry which does "
                "not exist");

    ht_table_delete(table);

    TEST_SUCCEED();
}

TEST_DEFINE(insert2) {
    struct ht_table *table;
    const char *str, *key, *value;

    table = ht_table_new(ht_hash_string, ht_equal_string);

    ht_table_insert2(table, "a", "abc", (void **)&key, (void **)&value);
    TEST_ASSERT(key == NULL,
                "invalid old key for entry 'a'");
    TEST_ASSERT(value == NULL,
                "invalid old value for entry 'a'");

    TEST_ASSERT(ht_table_get(table, "a", (void **)&str) == 1,
                "cannot fetch entry 'a'");
    TEST_ASSERT(strcmp(str, "abc") == 0,
                "invalid value for entry 'a'");

    ht_table_insert2(table, "a", "def", (void **)&key, (void **)&value);

    TEST_ASSERT(ht_table_get(table, "a", (void **)&str) == 1,
                "cannot fetch entry 'a'");
    TEST_ASSERT(strcmp(str, "def") == 0,
                "invalid value for entry 'a'");
    TEST_ASSERT(strcmp(key, "a") == 0,
                "invalid old key for entry 'a'");
    TEST_ASSERT(strcmp(value, "abc") == 0,
                "invalid old value for entry 'a'");

    ht_table_delete(table);

    TEST_SUCCEED();
}

TEST_DEFINE(remove) {
    struct ht_table *table;

    table = ht_table_new(ht_hash_string, ht_equal_string);
    if (!table)
        TEST_ERROR("cannot create table: %s", ht_get_error());

    TEST_ASSERT(ht_table_remove(table, "a") == 0,
                "invalid return value when removing an entry which does "
                "not exist");

    ht_table_insert(table, "a", "abc");
    ht_table_remove(table, "a");

    TEST_ASSERT(ht_table_is_empty(table),
                "a table should be empty after the removal of its only entry");

    ht_table_insert(table, "a", "abc");
    ht_table_insert(table, "d", "def");
    ht_table_insert(table, "g", "ghi");

    TEST_ASSERT(ht_table_remove(table, "d") == 1,
                "invalid return value when removing an existing entry");
    TEST_ASSERT(ht_table_remove(table, "j") == 0,
                "invalid return value when removing an entry that does"
                "not exist");

    ht_table_delete(table);

    TEST_SUCCEED();
}

TEST_DEFINE(remove2) {
    struct ht_table *table;
    const char *key, *value;

    table = ht_table_new(ht_hash_string, ht_equal_string);
    if (!table)
        TEST_ERROR("cannot create table: %s", ht_get_error());

    ht_table_insert(table, "a", "abc");
    TEST_ASSERT(ht_table_remove2(table, "a",
                                 (void **)&key, (void **)&value) == 1,
                "invalid return value when removing an existing entry");
    TEST_ASSERT(strcmp(key, "a") == 0,
                "invalid old key for entry 'a'");
    TEST_ASSERT(strcmp(value, "abc") == 0,
                "invalid old value for entry 'a'");

    ht_table_delete(table);

    TEST_SUCCEED();
}

TEST_DEFINE(clear) {
    struct ht_table *table;

    table = ht_table_new(ht_hash_string, ht_equal_string);

    ht_table_insert(table, "a", "abc");

    ht_table_clear(table);

    TEST_ASSERT(ht_table_is_empty(table),
                "a cleared table should be empty");

    TEST_ASSERT(ht_table_contains(table, "a") == false,
                "a cleared table should not contains any entry");

    ht_table_delete(table);

    TEST_SUCCEED();
}

TEST_DEFINE(resize) {
    struct ht_table *table;

    size_t nb_entries = 100;
    size_t nb_removed = 90;

    table = ht_table_new(ht_hash_int32, ht_equal_int32);

    for (size_t i = 0; i < nb_entries; i++) {
        ht_table_insert(table, HT_INT32_TO_POINTER(i),
                        HT_INT32_TO_POINTER(1));
    }

    for (size_t i = 0; i < nb_entries; i++) {
        TEST_ASSERT(ht_table_contains(table, HT_INT32_TO_POINTER(i)),
                    "entry %zu not found", i);
    }

    for (size_t i = 0; i < nb_removed; i++) {
        ht_table_remove(table, HT_INT32_TO_POINTER(i));
    }

    TEST_ASSERT(ht_table_get_nb_entries(table) == nb_entries - nb_removed,
                "invalid number of entries after resize");

    for (size_t i = 0; i < nb_entries; i++) {
        if (i < nb_removed) {
            TEST_ASSERT(!ht_table_contains(table, HT_INT32_TO_POINTER(i)),
                        "removed entry %zu found", i);
        } else {
            TEST_ASSERT(ht_table_contains(table, HT_INT32_TO_POINTER(i)),
                        "entry %zu not found", i);
        }
    }

    ht_table_delete(table);

    TEST_SUCCEED();
}

TEST_DEFINE(iterate) {
    struct ht_table *table;
    struct ht_table_iterator *it;
    void *key, *value;

    size_t nb_values;
    struct {
        int key;
        char *value;
        bool found;
    } values[] = {
        {0, "abc", false},
        {1, "def", false},
        {2, "ghi", false},
        {3, "jkl", false},
        {4, "mno", false},
        {5, "pqr", false},
        {6, "stu", false},
        {7, "vwx", false},
        {8, "yz",  false}
    };

    nb_values = sizeof(values) / sizeof(values[0]);

    table = ht_table_new(ht_hash_int32, ht_equal_int32);

    it = ht_table_iterate(table);
    TEST_ASSERT(ht_table_iterator_get_next(it, &key, &value) == 0,
                "next entry found in empty table");
    ht_table_iterator_delete(it);

    for (size_t i = 0; i < nb_values; i++)
        ht_table_insert(table, HT_INT32_TO_POINTER(values[i].key),
                        values[i].value);

    it = ht_table_iterate(table);

    for (size_t i = 0; i < nb_values; i++) {
        bool found;

        TEST_ASSERT(ht_table_iterator_get_next(it, &key, &value) == 1,
                    "next entry not found by iterator");

        found = false;
        for (size_t i = 0; i < nb_values; i++) {
            if (HT_POINTER_TO_INT32(key) == values[i].key) {
                found = true;
                TEST_ASSERT(!values[i].found,
                            "iterator returned the same entry two times");
                TEST_ASSERT(strcmp(value, values[i].value) == 0,
                            "invalid value returned by iterator");
                values[i].found = true;
                break;
            }
        }

        TEST_ASSERT(found, "unknown key returned by iterator");
    }

    for (size_t i = 0; i < nb_values; i++) {
        TEST_ASSERT(values[i].found,
                    "entry was not encountered during iteration");
    }

    TEST_ASSERT(ht_table_iterator_get_next(it, &key, &value) == 0,
                "next entry found by iterator");

    ht_table_iterator_delete(it);

    ht_table_delete(table);

    TEST_SUCCEED();
}

TEST_DEFINE(iterate_operations) {
    struct ht_table *table;
    struct ht_table_iterator *it;
    void *key, *value;

    table = ht_table_new(ht_hash_string, ht_equal_string);

    ht_table_insert(table, "a", "abc");
    ht_table_insert(table, "d", "def");
    ht_table_insert(table, "g", "ghi");

    it = ht_table_iterate(table);

    while (ht_table_iterator_get_next(it, &key, &value)) {
        if (strcmp(key, "d") == 0) {
            ht_table_iterator_remove(it);
        } else if (strcmp(key, "g") == 0) {
            ht_table_iterator_set_value(it, "foo");
        }
    }

    ht_table_iterator_delete(it);

    TEST_ASSERT(ht_table_contains(table, "a"),
                "entry not found");
    TEST_ASSERT(!ht_table_contains(table, "d"),
                "entry removed by iterator found");
    TEST_ASSERT(ht_table_get(table, "g", &value) == 1,
                "entry not found");
    TEST_ASSERT(strcmp(value, "foo") == 0,
                "invalid value after modification by iterator");

    ht_table_delete(table);

    TEST_SUCCEED();
}


#define TEST_CASE(name_) {.name = #name_, .test_func = test_case_##name_}

static struct {
    const char *name;
    int (*test_func)();
} test_cases[] = {
    TEST_CASE(insert),
    TEST_CASE(insert2),
    TEST_CASE(remove),
    TEST_CASE(remove2),
    TEST_CASE(clear),
    TEST_CASE(resize),
    TEST_CASE(iterate),
    TEST_CASE(iterate_operations),
};

#undef TEST_CASE

int
main(int argc, char **argv) {
    size_t nb_tests, nb_passed;
    const char *color_esc;
    int opt;

    opterr = 0;
    while ((opt = getopt(argc, argv, "hn:")) != -1) {
        switch (opt) {
            case 'h':
                usage(argv[0], 0);
                break;

            case '?':
                usage(argv[0], 1);
        }
    }

    nb_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    nb_passed = 0;

    for (size_t i = 0; i < nb_tests; i++) {
        int ret;

        printf("\e[34m---- %-30s ----\e[0m\n", test_cases[i].name);

        ret = test_cases[i].test_func();

        if (ret == -1) {
            printf("\e[31;1merror\e[0m\n");
        } else if (ret == 0) {
            printf("\e[31mfailure\e[0m\n");
        } else {
            printf("\e[32mok\e[0m\n");
            nb_passed++;
        }
    }

    printf("\e[34m----------------------------------------\e[0m\n");

    if (nb_passed == 0) {
        color_esc = "\e[31m";
    } else if (nb_passed == nb_tests) {
        color_esc = "\e[32m";
    } else {
        color_esc = "\e[33m";
    }

    printf("%s%zu/%zu tests passed (%.0f%%)\e[0m\n",
           color_esc, nb_passed, nb_tests,
           (double)nb_passed * 100.0 / nb_tests);

    return (nb_passed == nb_tests) ? 0 : 1;
}

static void
usage(const char *argv0, int exit_code) {
    printf("Usage: %s [-hn]\n"
            "\n"
            "Options:\n"
            "  -h         display help\n",
            argv0);
    exit(exit_code);
}

static void
die(const char *fmt, ...) {
    va_list ap;

    fprintf(stderr, "fatal error: ");

    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);

    putc('\n', stderr);
    exit(1);
}
