/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * bit_trie.h - Binary Trie (Bitwise Trie)
 *
 * Trie where each node has only 2 children (0 and 1).
 * Useful for:
 *   - Maximum XOR of two numbers
 *   - XOR queries on arrays
 *   - Bitwise prefix matching
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_BIT_TRIE_H
#define DJLC_BIT_TRIE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/*============================================================================
 * BINARY TRIE NODE
 *===========================================================================*/

typedef struct djlc_bit_trie_node {
    struct djlc_bit_trie_node *children[2];
    int count;  /* Number of values passing through this node */
} djlc_bit_trie_node_t;

typedef struct {
    djlc_bit_trie_node_t *root;
    int max_bits;  /* Number of bits to consider (default 32) */
} djlc_bit_trie_t;

/*============================================================================
 * BASIC OPERATIONS
 *===========================================================================*/

static inline djlc_bit_trie_node_t *djlc_bit_trie_new_node(void) {
    djlc_bit_trie_node_t *node = (djlc_bit_trie_node_t *)calloc(1, sizeof(djlc_bit_trie_node_t));
    return node;
}

static inline bool djlc_bit_trie_init(djlc_bit_trie_t *trie, int max_bits) {
    trie->root = djlc_bit_trie_new_node();
    trie->max_bits = max_bits > 0 ? max_bits : 32;
    return trie->root != NULL;
}

static inline void djlc_bit_trie_free_node(djlc_bit_trie_node_t *node) {
    if (!node) return;
    djlc_bit_trie_free_node(node->children[0]);
    djlc_bit_trie_free_node(node->children[1]);
    free(node);
}

static inline void djlc_bit_trie_free(djlc_bit_trie_t *trie) {
    djlc_bit_trie_free_node(trie->root);
    trie->root = NULL;
}

/* Insert a number into the trie */
static inline void djlc_bit_trie_insert(djlc_bit_trie_t *trie, int num) {
    djlc_bit_trie_node_t *node = trie->root;
    for (int i = trie->max_bits - 1; i >= 0; i--) {
        int bit = (num >> i) & 1;
        if (!node->children[bit]) {
            node->children[bit] = djlc_bit_trie_new_node();
        }
        node = node->children[bit];
        node->count++;
    }
}

/* Remove a number from the trie (decrement count) */
static inline void djlc_bit_trie_remove(djlc_bit_trie_t *trie, int num) {
    djlc_bit_trie_node_t *node = trie->root;
    for (int i = trie->max_bits - 1; i >= 0; i--) {
        int bit = (num >> i) & 1;
        if (!node->children[bit]) return;
        node = node->children[bit];
        node->count--;
    }
}

/* Check if a number exists in the trie */
static inline bool djlc_bit_trie_contains(djlc_bit_trie_t *trie, int num) {
    djlc_bit_trie_node_t *node = trie->root;
    for (int i = trie->max_bits - 1; i >= 0; i--) {
        int bit = (num >> i) & 1;
        if (!node->children[bit] || node->children[bit]->count == 0) {
            return false;
        }
        node = node->children[bit];
    }
    return true;
}

/*============================================================================
 * XOR OPERATIONS
 *===========================================================================*/

/* Find maximum XOR with given number */
static inline int djlc_bit_trie_max_xor(djlc_bit_trie_t *trie, int num) {
    djlc_bit_trie_node_t *node = trie->root;
    int result = 0;
    for (int i = trie->max_bits - 1; i >= 0; i--) {
        int bit = (num >> i) & 1;
        int want = 1 - bit;  /* We want opposite bit for max XOR */
        if (node->children[want] && node->children[want]->count > 0) {
            result |= (1 << i);
            node = node->children[want];
        } else if (node->children[bit]) {
            node = node->children[bit];
        } else {
            break;
        }
    }
    return result;
}

/* Find minimum XOR with given number */
static inline int djlc_bit_trie_min_xor(djlc_bit_trie_t *trie, int num) {
    djlc_bit_trie_node_t *node = trie->root;
    int result = 0;
    for (int i = trie->max_bits - 1; i >= 0; i--) {
        int bit = (num >> i) & 1;
        /* We want same bit for min XOR */
        if (node->children[bit] && node->children[bit]->count > 0) {
            node = node->children[bit];
        } else if (node->children[1 - bit]) {
            result |= (1 << i);
            node = node->children[1 - bit];
        } else {
            break;
        }
    }
    return result;
}

/* Find maximum XOR between any two numbers in array */
static inline int djlc_bit_trie_max_xor_pair(int *nums, int n) {
    djlc_bit_trie_t trie;
    djlc_bit_trie_init(&trie, 32);
    int max_xor = 0;
    for (int i = 0; i < n; i++) {
        djlc_bit_trie_insert(&trie, nums[i]);
        int xor_val = djlc_bit_trie_max_xor(&trie, nums[i]);
        if (xor_val > max_xor) max_xor = xor_val;
    }
    djlc_bit_trie_free(&trie);
    return max_xor;
}

#endif /* DJLC_BIT_TRIE_H */

