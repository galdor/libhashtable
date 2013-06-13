/*
 * Copyright (c) 2013 Nicolas Martyanoff
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

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>

#ifdef HT_PLATFORM_LINUX
#   include <sched.h>
#endif

#include "glib.h"

#include "hashtable.h"

static void die(const char *, ...)
    __attribute__((format(printf, 1, 2)));
static void usage(const char *, int);

static void bench_start();
static void bench_report(const char *, size_t);

static void bench_read_file(const char *, char ***, size_t *);

static uint32_t bench_hash_ht(const void *);
static bool bench_equal_ht(const void *, const void *);
static void bench_ht(char **, size_t);

static guint bench_hash_glib(gconstpointer);
static gboolean bench_equal_glib(gconstpointer, gconstpointer);
static void bench_glib(char **, size_t);

static struct timespec bench_time_1;


int
main(int argc, char **argv) {
    const char *path;
    char **words;
    size_t nb_words;
    int opt;

    opterr = 0;
    while ((opt = getopt(argc, argv, "h")) != -1) {
        switch (opt) {
            case 'h':
                usage(argv[0], 0);
                break;

            case '?':
                usage(argv[0], 1);
        }
    }

    if (optind >= argc)
        usage(argv[0], 1);

    path = argv[optind];

    bench_read_file(path, &words, &nb_words);
    printf("%zu words read from %s\n", nb_words, path);

#ifdef HT_PLATFORM_LINUX
    {
        cpu_set_t set;

        CPU_ZERO(&set);
        CPU_SET(0, &set);

        if (sched_setaffinity(0, sizeof(cpu_set_t), &set) == -1)
            die("cannot set process affinity: %m");
    }
#endif

    bench_ht(words, nb_words);
    bench_glib(words, nb_words);

    for (size_t i = 0; i < nb_words; i++)
        free(words[i]);
    free(words);

    return 0;
}

static void
usage(const char *argv0, int exit_code) {
    printf("Usage: %s [-h] <path>\n"
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

static void
bench_start() {
    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &bench_time_1) == -1)
        die("cannot get clock value: %m");
}

static void
bench_report(const char *label, size_t nb_words) {
    struct timespec bench_time_2;
    double time_1, time_2, time_diff;
    size_t words_per_second;

    if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &bench_time_2) == -1)
        die("cannot get clock value: %m");

    time_1  = bench_time_1.tv_sec * 1000.0;
    time_1 += bench_time_1.tv_nsec / 1.0e6;

    time_2  = bench_time_2.tv_sec * 1000.0;
    time_2 += bench_time_2.tv_nsec / 1.0e6;

    time_diff = time_2 - time_1;
    words_per_second = (size_t)((nb_words * 1000.0) / time_diff);

    printf("%-20s  %.2fms (%zu words/s)\n",
           label, time_diff, words_per_second);
}

static void
bench_read_file(const char *path, char ***pwords, size_t *p_nb_words) {
    char **words;
    size_t sz, nb_words;
    struct stat st;
    void *map;
    size_t mapsz;
    int fd;
    const char *ptr;
    size_t len;

    fd = open(path, O_RDONLY);
    if (fd == -1)
        die("cannot open %s: %m", path);

    if (fstat(fd, &st) == -1)
        die("cannot get stat on %s: %m", path);

    mapsz = (size_t)st.st_size;

    map = mmap(NULL, mapsz, PROT_READ, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
        die("cannot map %s: %m", path);

    close(fd);

    sz = 8;
    words = malloc(sz * sizeof(char *));
    if (!words)
        die("cannot allocate word array: %m");

    nb_words = 0;

    ptr = map;
    len = mapsz;

    while (len > 0) {
        const char *start;
        char *word;
        size_t word_len;

        while (len > 0) {
            if (isalnum((unsigned char)*ptr)) {
                break;
            } else {
                ptr++;
                len--;
            }
        }
        if (len == 0)
            break;

        start = ptr;

        while (len > 0) {
            if (isalnum((unsigned char)*ptr)) {
                ptr++;
                len--;
            } else {
                break;
            }
        }

        word_len = (size_t)(ptr - start);
        word = strndup(start, word_len);
        if (!word)
            die("cannot allocate word: %m");

        if (nb_words + 1 >= sz) {
            sz *= 2;
            words = realloc(words, sz * sizeof(char *));
            if (!words)
                die("cannot reallocate word array: %m");
        }

        words[nb_words++] = word;
    }

    *pwords = words;
    *p_nb_words = nb_words;

    munmap(map, mapsz);
}


static uint32_t
bench_hash_ht(const void *key) {
    const unsigned char *str;
    uint32_t hash;

    hash = 5381;
    for (str = key; *str; str++)
        hash = ((hash << 5) + hash) ^ *str;

    return hash;
}

static bool
bench_equal_ht(const void *k1, const void *k2) {
    const char *s1, *s2;

    s1 = k1;
    s2 = k2;

    return strcmp(s1, s2) == 0;
}

static void
bench_ht(char **words, size_t nb_words) {
    struct ht_table *table;

    table = ht_table_new(bench_hash_ht, bench_equal_ht);
    if (!table)
        die("cannot create hash table: %s", ht_get_error());

    bench_start();

    for (size_t i = 0; i < nb_words; i++) {
        char *word;
        intptr_t count;
        void *pvalue;

        word = words[i];

        if (ht_table_get(table, word, &pvalue) == 1) {
            count = (intptr_t)pvalue;
        } else {
            count = 1;
        }

        if (ht_table_insert(table, word, (void *)count) == -1)
            die("cannot insert entry: %s", ht_get_error());
    }

    bench_report("libhashtable", nb_words);

    ht_table_delete(table);
}

static guint
bench_hash_glib(gconstpointer key) {
    const unsigned char *str;
    uint32_t hash;

    hash = 5381;
    for (str = key; *str; str++)
        hash = ((hash << 5) + hash) ^ *str;

    return hash;
}

static gboolean
bench_equal_glib(gconstpointer k1, gconstpointer k2) {
    const char *s1, *s2;

    s1 = k1;
    s2 = k2;

    return strcmp(s1, s2) == 0;
}

static void
bench_glib(char **words, size_t nb_words) {
    GHashTable *table;

    table = g_hash_table_new(bench_hash_glib, bench_equal_glib);

    bench_start();

    for (size_t i = 0; i < nb_words; i++) {
        char *word;
        intptr_t count;
        gpointer pvalue;

        word = words[i];

        if (g_hash_table_lookup_extended(table, word, NULL, &pvalue) == TRUE) {
            count = GPOINTER_TO_INT(pvalue) + 1;
        } else {
            count = 1;
        }

        g_hash_table_insert(table, word, GINT_TO_POINTER(count));
    }

    bench_report("glib", nb_words);

    g_hash_table_destroy(table);
}
