/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * trie.h - Trie (Prefix Tree) data structure
 *
 * Efficient string matching and prefix queries.
 * Supports lowercase English letters [a-z] by default.
 *
 * Author: Danny Jiang
 * Created: 2025-12-16
 */

#ifndef DJLC_TRIE_H
#define DJLC_TRIE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define DJLC_TRIE_ALPHABET_SIZE 26

typedef struct djlc_trie_node {
    struct djlc_trie_node *children[DJLC_TRIE_ALPHABET_SIZE];
    int count;          /* Number of words ending at this node */
    int prefix_count;   /* Number of words with this prefix */
    int id;             /* Optional: word ID for mapping */
} djlc_trie_node_t;

typedef struct {
    djlc_trie_node_t *root;
    int word_count;     /* Total unique words */
    int node_count;     /* Total nodes allocated */
} djlc_trie_t;

/*============================================================================
 * Node operations
 *===========================================================================*/

static inline djlc_trie_node_t* djlc_trie_new_node(void) {
    djlc_trie_node_t *node = (djlc_trie_node_t *)calloc(1, sizeof(djlc_trie_node_t));
    return node;
}

static inline void djlc_trie_free_node(djlc_trie_node_t *node) {
    if (!node) return;
    for (int i = 0; i < DJLC_TRIE_ALPHABET_SIZE; i++) {
        djlc_trie_free_node(node->children[i]);
    }
    free(node);
}

/*============================================================================
 * Trie lifecycle
 *===========================================================================*/

static inline bool djlc_trie_init(djlc_trie_t *trie) {
    trie->root = djlc_trie_new_node();
    if (!trie->root) return false;
    trie->word_count = 0;
    trie->node_count = 1;
    return true;
}

static inline djlc_trie_t* djlc_trie_create(void) {
    djlc_trie_t *trie = (djlc_trie_t *)malloc(sizeof(djlc_trie_t));
    if (!trie) return NULL;
    if (!djlc_trie_init(trie)) {
        free(trie);
        return NULL;
    }
    return trie;
}

static inline void djlc_trie_free(djlc_trie_t *trie) {
    if (!trie) return;
    djlc_trie_free_node(trie->root);
    trie->root = NULL;
    trie->word_count = 0;
    trie->node_count = 0;
}

static inline void djlc_trie_destroy(djlc_trie_t *trie) {
    djlc_trie_free(trie);
    free(trie);
}

/*============================================================================
 * Core operations
 *===========================================================================*/

/**
 * Insert a word into the trie
 * Returns the node where the word ends
 */
static inline djlc_trie_node_t* djlc_trie_insert(djlc_trie_t *trie, const char *word, size_t len) {
    djlc_trie_node_t *node = trie->root;
    
    for (size_t i = 0; i < len; i++) {
        int idx = word[i] - 'a';
        if (idx < 0 || idx >= DJLC_TRIE_ALPHABET_SIZE) return NULL;
        
        if (!node->children[idx]) {
            node->children[idx] = djlc_trie_new_node();
            if (!node->children[idx]) return NULL;
            trie->node_count++;
        }
        node = node->children[idx];
        node->prefix_count++;
    }
    
    if (node->count == 0) {
        trie->word_count++;
        node->id = trie->word_count;  /* Assign unique ID */
    }
    node->count++;
    return node;
}

/**
 * Insert null-terminated string
 */
static inline djlc_trie_node_t* djlc_trie_insert_s(djlc_trie_t *trie, const char *word) {
    return djlc_trie_insert(trie, word, strlen(word));
}

/**
 * Search for exact word match
 * Returns the end node if found, NULL otherwise
 */
static inline djlc_trie_node_t* djlc_trie_search(djlc_trie_t *trie, const char *word, size_t len) {
    djlc_trie_node_t *node = trie->root;
    
    for (size_t i = 0; i < len; i++) {
        int idx = word[i] - 'a';
        if (idx < 0 || idx >= DJLC_TRIE_ALPHABET_SIZE) return NULL;
        if (!node->children[idx]) return NULL;
        node = node->children[idx];
    }
    
    return (node->count > 0) ? node : NULL;
}

static inline djlc_trie_node_t* djlc_trie_search_s(djlc_trie_t *trie, const char *word) {
    return djlc_trie_search(trie, word, strlen(word));
}

/**
 * Check if word exists
 */
static inline bool djlc_trie_contains(djlc_trie_t *trie, const char *word, size_t len) {
    return djlc_trie_search(trie, word, len) != NULL;
}

static inline bool djlc_trie_contains_s(djlc_trie_t *trie, const char *word) {
    return djlc_trie_search_s(trie, word) != NULL;
}

/**
 * Check if any word starts with prefix
 */
static inline djlc_trie_node_t* djlc_trie_starts_with(djlc_trie_t *trie, const char *prefix, size_t len) {
    djlc_trie_node_t *node = trie->root;

    for (size_t i = 0; i < len; i++) {
        int idx = prefix[i] - 'a';
        if (idx < 0 || idx >= DJLC_TRIE_ALPHABET_SIZE) return NULL;
        if (!node->children[idx]) return NULL;
        node = node->children[idx];
    }

    return node;
}

static inline djlc_trie_node_t* djlc_trie_starts_with_s(djlc_trie_t *trie, const char *prefix) {
    return djlc_trie_starts_with(trie, prefix, strlen(prefix));
}

/**
 * Get word count for a specific word
 */
static inline int djlc_trie_get_count(djlc_trie_t *trie, const char *word, size_t len) {
    djlc_trie_node_t *node = djlc_trie_search(trie, word, len);
    return node ? node->count : 0;
}

static inline int djlc_trie_get_count_s(djlc_trie_t *trie, const char *word) {
    return djlc_trie_get_count(trie, word, strlen(word));
}

/**
 * Match word in string at given position
 * Returns end node if match found, NULL otherwise
 */
static inline djlc_trie_node_t* djlc_trie_match_at(djlc_trie_t *trie, const char *s,
                                                    size_t pos, size_t word_len) {
    djlc_trie_node_t *node = trie->root;

    for (size_t i = 0; i < word_len; i++) {
        int idx = s[pos + i] - 'a';
        if (idx < 0 || idx >= DJLC_TRIE_ALPHABET_SIZE) return NULL;
        if (!node->children[idx]) return NULL;
        node = node->children[idx];
    }

    return (node->count > 0) ? node : NULL;
}

#endif /* DJLC_TRIE_H */

