/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * hashmap_str.h - String key hashmap
 *
 * A hashmap with string keys and integer values.
 * Uses chaining for collision resolution.
 * Useful for word counting, frequency maps, etc.
 *
 * Author: Danny Jiang
 * Created: 2025-12-16
 */

#ifndef DJLC_HASHMAP_STR_H
#define DJLC_HASHMAP_STR_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define DJLC_HMS_DEFAULT_CAPACITY 1024

/* Hash table entry */
typedef struct djlc_hms_entry {
    char *key;
    int value;
    struct djlc_hms_entry *next;
} djlc_hms_entry_t;

typedef struct {
    djlc_hms_entry_t **buckets;
    size_t capacity;
    size_t size;
} djlc_hashmap_str_t;

/*============================================================================
 * Hash function (djb2)
 *===========================================================================*/

static inline unsigned int djlc_hms_hash(const char *str, size_t len) {
    unsigned int hash = 5381;
    for (size_t i = 0; i < len; i++) {
        hash = ((hash << 5) + hash) + (unsigned char)str[i];
    }
    return hash;
}

/*============================================================================
 * Lifecycle
 *===========================================================================*/

static inline bool djlc_hms_init(djlc_hashmap_str_t *hm, size_t capacity) {
    hm->buckets = (djlc_hms_entry_t **)calloc(capacity, sizeof(djlc_hms_entry_t *));
    if (!hm->buckets) return false;
    hm->capacity = capacity;
    hm->size = 0;
    return true;
}

static inline djlc_hashmap_str_t* djlc_hms_create(size_t capacity) {
    djlc_hashmap_str_t *hm = (djlc_hashmap_str_t *)malloc(sizeof(djlc_hashmap_str_t));
    if (!hm) return NULL;
    if (!djlc_hms_init(hm, capacity)) {
        free(hm);
        return NULL;
    }
    return hm;
}

static inline void djlc_hms_free(djlc_hashmap_str_t *hm) {
    if (!hm) return;
    for (size_t i = 0; i < hm->capacity; i++) {
        djlc_hms_entry_t *e = hm->buckets[i];
        while (e) {
            djlc_hms_entry_t *next = e->next;
            free(e->key);
            free(e);
            e = next;
        }
    }
    free(hm->buckets);
    hm->buckets = NULL;
    hm->capacity = 0;
    hm->size = 0;
}

static inline void djlc_hms_destroy(djlc_hashmap_str_t *hm) {
    djlc_hms_free(hm);
    free(hm);
}

/*============================================================================
 * Operations
 *===========================================================================*/

/* Put or increment: if key exists, add delta; otherwise set to delta */
static inline bool djlc_hms_add(djlc_hashmap_str_t *hm, const char *key, size_t len, int delta) {
    unsigned int idx = djlc_hms_hash(key, len) % hm->capacity;
    djlc_hms_entry_t *e = hm->buckets[idx];
    
    while (e) {
        if (strlen(e->key) == len && strncmp(e->key, key, len) == 0) {
            e->value += delta;
            return true;
        }
        e = e->next;
    }
    
    /* Create new entry */
    djlc_hms_entry_t *newEntry = (djlc_hms_entry_t *)malloc(sizeof(djlc_hms_entry_t));
    if (!newEntry) return false;
    newEntry->key = (char *)malloc(len + 1);
    if (!newEntry->key) { free(newEntry); return false; }
    strncpy(newEntry->key, key, len);
    newEntry->key[len] = '\0';
    newEntry->value = delta;
    newEntry->next = hm->buckets[idx];
    hm->buckets[idx] = newEntry;
    hm->size++;
    return true;
}

/* Put: set value (overwrites existing) */
static inline bool djlc_hms_put(djlc_hashmap_str_t *hm, const char *key, size_t len, int value) {
    unsigned int idx = djlc_hms_hash(key, len) % hm->capacity;
    djlc_hms_entry_t *e = hm->buckets[idx];
    
    while (e) {
        if (strlen(e->key) == len && strncmp(e->key, key, len) == 0) {
            e->value = value;
            return true;
        }
        e = e->next;
    }
    
    return djlc_hms_add(hm, key, len, value);
}

/* Get value, returns 0 if not found */
static inline int djlc_hms_get(djlc_hashmap_str_t *hm, const char *key, size_t len) {
    unsigned int idx = djlc_hms_hash(key, len) % hm->capacity;
    djlc_hms_entry_t *e = hm->buckets[idx];
    
    while (e) {
        if (strlen(e->key) == len && strncmp(e->key, key, len) == 0) {
            return e->value;
        }
        e = e->next;
    }
    return 0;
}

/* Check if key exists */
static inline bool djlc_hms_contains(djlc_hashmap_str_t *hm, const char *key, size_t len) {
    unsigned int idx = djlc_hms_hash(key, len) % hm->capacity;
    djlc_hms_entry_t *e = hm->buckets[idx];

    while (e) {
        if (strlen(e->key) == len && strncmp(e->key, key, len) == 0) {
            return true;
        }
        e = e->next;
    }
    return false;
}

/* Reset all values to 0 (keeps keys) */
static inline void djlc_hms_reset(djlc_hashmap_str_t *hm) {
    for (size_t i = 0; i < hm->capacity; i++) {
        djlc_hms_entry_t *e = hm->buckets[i];
        while (e) {
            e->value = 0;
            e = e->next;
        }
    }
}

/* Convenience: null-terminated string versions */
static inline bool djlc_hms_put_s(djlc_hashmap_str_t *hm, const char *key, int value) {
    return djlc_hms_put(hm, key, strlen(key), value);
}

static inline bool djlc_hms_add_s(djlc_hashmap_str_t *hm, const char *key, int delta) {
    return djlc_hms_add(hm, key, strlen(key), delta);
}

static inline int djlc_hms_get_s(djlc_hashmap_str_t *hm, const char *key) {
    return djlc_hms_get(hm, key, strlen(key));
}

static inline bool djlc_hms_contains_s(djlc_hashmap_str_t *hm, const char *key) {
    return djlc_hms_contains(hm, key, strlen(key));
}

#endif /* DJLC_HASHMAP_STR_H */

