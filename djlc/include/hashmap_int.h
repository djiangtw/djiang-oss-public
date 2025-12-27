/**
 * hashmap_int.h - Integer key hashmap
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public
 *
 * A simple hashmap with integer keys and integer values.
 * Uses open addressing with linear probing.
 */

#ifndef DJLC_HASHMAP_INT_H
#define DJLC_HASHMAP_INT_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/* Hash table entry states */
#define DJLC_HM_EMPTY    0
#define DJLC_HM_OCCUPIED 1
#define DJLC_HM_DELETED  2

/* Default capacity */
#define DJLC_HM_DEFAULT_CAPACITY 1024

typedef struct {
    int key;
    int value;
    uint8_t state;
} djlc_hm_entry_t;

typedef struct {
    djlc_hm_entry_t *entries;
    size_t capacity;
    size_t size;
} djlc_hashmap_int_t;

/* Initialize hashmap with given capacity */
static inline bool djlc_hm_init(djlc_hashmap_int_t *hm, size_t capacity) {
    hm->entries = (djlc_hm_entry_t *)calloc(capacity, sizeof(djlc_hm_entry_t));
    if (!hm->entries) return false;
    hm->capacity = capacity;
    hm->size = 0;
    return true;
}

/* Free hashmap */
static inline void djlc_hm_free(djlc_hashmap_int_t *hm) {
    if (hm->entries) {
        free(hm->entries);
        hm->entries = NULL;
    }
    hm->capacity = 0;
    hm->size = 0;
}

/* Hash function for integers */
static inline size_t djlc_hm_hash(int key, size_t capacity) {
    /* Handle negative keys */
    uint32_t k = (uint32_t)key;
    k = ((k >> 16) ^ k) * 0x45d9f3b;
    k = ((k >> 16) ^ k) * 0x45d9f3b;
    k = (k >> 16) ^ k;
    return k % capacity;
}

/* Put key-value pair into hashmap */
static inline bool djlc_hm_put(djlc_hashmap_int_t *hm, int key, int value) {
    if (hm->size >= hm->capacity * 3 / 4) {
        /* Load factor > 0.75, should resize (not implemented for simplicity) */
        return false;
    }

    size_t idx = djlc_hm_hash(key, hm->capacity);
    size_t start = idx;

    do {
        if (hm->entries[idx].state != DJLC_HM_OCCUPIED) {
            hm->entries[idx].key = key;
            hm->entries[idx].value = value;
            hm->entries[idx].state = DJLC_HM_OCCUPIED;
            hm->size++;
            return true;
        }
        if (hm->entries[idx].key == key) {
            /* Update existing key */
            hm->entries[idx].value = value;
            return true;
        }
        idx = (idx + 1) % hm->capacity;
    } while (idx != start);

    return false;
}

/* Get value by key, returns true if found */
static inline bool djlc_hm_get(djlc_hashmap_int_t *hm, int key, int *value) {
    size_t idx = djlc_hm_hash(key, hm->capacity);
    size_t start = idx;

    do {
        if (hm->entries[idx].state == DJLC_HM_EMPTY) {
            return false;
        }
        if (hm->entries[idx].state == DJLC_HM_OCCUPIED &&
            hm->entries[idx].key == key) {
            *value = hm->entries[idx].value;
            return true;
        }
        idx = (idx + 1) % hm->capacity;
    } while (idx != start);

    return false;
}

/* Check if key exists */
static inline bool djlc_hm_contains(djlc_hashmap_int_t *hm, int key) {
    int dummy;
    return djlc_hm_get(hm, key, &dummy);
}

#endif /* DJLC_HASHMAP_INT_H */

