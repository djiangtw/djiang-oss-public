/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * string_utils.h - String Utility Functions
 *
 * Common string operations for LeetCode problems.
 * Includes: hash, sort, duplicate, etc.
 *
 * Author: Danny Jiang
 * Created: 2025-12-17
 */

#ifndef DJLC_STRING_UTILS_H
#define DJLC_STRING_UTILS_H

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ========== Hash Functions ========== */

/* DJB2 hash - good general purpose string hash */
static inline unsigned int djlc_hash_djb2(const char* s) {
    unsigned int hash = 5381;
    int c;
    while ((c = *s++)) {
        hash = ((hash << 5) + hash) + c;  /* hash * 33 + c */
    }
    return hash;
}

/* Simple hash with multiplier 31 */
static inline unsigned int djlc_hash_str(const char* s) {
    unsigned int h = 0;
    while (*s) {
        h = h * 31 + *s++;
    }
    return h;
}

/* Hash with modulo (for hash table) */
static inline unsigned int djlc_hash_str_mod(const char* s, unsigned int mod) {
    return djlc_hash_str(s) % mod;
}

/* ========== String Sorting ========== */

/* Compare function for qsort on chars */
static inline int djlc_cmp_char_for_qsort(const void* a, const void* b) {
    return *(const char*)a - *(const char*)b;
}

/* Sort string in-place */
static inline void djlc_sort_str_inplace(char* s) {
    qsort(s, strlen(s), sizeof(char), djlc_cmp_char_for_qsort);
}

/* Return a sorted copy of string (caller must free) */
static inline char* djlc_sort_str(const char* s) {
    int len = strlen(s);
    char* sorted = (char*)malloc(len + 1);
    if (!sorted) return NULL;
    strcpy(sorted, s);
    qsort(sorted, len, sizeof(char), djlc_cmp_char_for_qsort);
    return sorted;
}

/* ========== String Duplication ========== */

/* Duplicate string (portable strdup) */
static inline char* djlc_strdup(const char* s) {
    size_t len = strlen(s) + 1;
    char* copy = (char*)malloc(len);
    if (copy) memcpy(copy, s, len);
    return copy;
}

/* Duplicate first n characters */
static inline char* djlc_strndup(const char* s, size_t n) {
    size_t len = strlen(s);
    if (n > len) n = len;
    char* copy = (char*)malloc(n + 1);
    if (copy) {
        memcpy(copy, s, n);
        copy[n] = '\0';
    }
    return copy;
}

/* ========== String Reverse ========== */

/* Reverse string in-place */
static inline void djlc_str_reverse(char* s) {
    int len = strlen(s);
    for (int i = 0, j = len - 1; i < j; i++, j--) {
        char t = s[i];
        s[i] = s[j];
        s[j] = t;
    }
}

/* Reverse portion of string [start, end) */
static inline void djlc_str_reverse_range(char* s, int start, int end) {
    for (int i = start, j = end - 1; i < j; i++, j--) {
        char t = s[i];
        s[i] = s[j];
        s[j] = t;
    }
}

/* ========== Character Counting ========== */

/* Count occurrences of character c in string s */
static inline int djlc_str_count_char(const char* s, char c) {
    int count = 0;
    while (*s) {
        if (*s++ == c) count++;
    }
    return count;
}

/* Check if string contains only digits */
static inline int djlc_str_is_digit(const char* s) {
    if (!*s) return 0;
    while (*s) {
        if (!isdigit(*s++)) return 0;
    }
    return 1;
}

/* Check if string contains only alphabetic characters */
static inline int djlc_str_is_alpha(const char* s) {
    if (!*s) return 0;
    while (*s) {
        if (!isalpha(*s++)) return 0;
    }
    return 1;
}

/* ========== Character Frequency ========== */

/* Fill frequency array for lowercase letters (size 26) */
static inline void djlc_str_freq_lower(const char* s, int freq[26]) {
    memset(freq, 0, 26 * sizeof(int));
    while (*s) {
        if (*s >= 'a' && *s <= 'z') {
            freq[*s - 'a']++;
        }
        s++;
    }
}

/* Fill frequency array for ASCII (size 128) */
static inline void djlc_str_freq_ascii(const char* s, int freq[128]) {
    memset(freq, 0, 128 * sizeof(int));
    while (*s) {
        freq[(unsigned char)*s]++;
        s++;
    }
}

#endif /* DJLC_STRING_UTILS_H */

