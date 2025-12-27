/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * doubly_list.h - Doubly Linked List
 *
 * Doubly linked list with sentinel nodes for O(1) operations.
 * Ideal for LRU Cache, LFU Cache patterns.
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_DOUBLY_LIST_H
#define DJLC_DOUBLY_LIST_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/*============================================================================
 * DOUBLY LINKED LIST NODE
 *===========================================================================*/

typedef struct djlc_dlist_node {
    int key;
    int val;
    struct djlc_dlist_node *prev;
    struct djlc_dlist_node *next;
} djlc_dlist_node_t;

/*============================================================================
 * DOUBLY LINKED LIST WITH SENTINEL NODES
 * head <-> node1 <-> node2 <-> ... <-> tail
 *===========================================================================*/

typedef struct {
    djlc_dlist_node_t *head;  /* Sentinel head (dummy) */
    djlc_dlist_node_t *tail;  /* Sentinel tail (dummy) */
    int size;
} djlc_dlist_t;

static inline bool djlc_dlist_init(djlc_dlist_t *list) {
    list->head = (djlc_dlist_node_t *)calloc(1, sizeof(djlc_dlist_node_t));
    list->tail = (djlc_dlist_node_t *)calloc(1, sizeof(djlc_dlist_node_t));
    if (!list->head || !list->tail) {
        free(list->head); free(list->tail);
        return false;
    }
    list->head->next = list->tail;
    list->tail->prev = list->head;
    list->size = 0;
    return true;
}

static inline void djlc_dlist_free(djlc_dlist_t *list) {
    djlc_dlist_node_t *cur = list->head;
    while (cur) {
        djlc_dlist_node_t *next = cur->next;
        free(cur);
        cur = next;
    }
    list->head = list->tail = NULL;
    list->size = 0;
}

static inline bool djlc_dlist_empty(djlc_dlist_t *list) {
    return list->size == 0;
}

static inline int djlc_dlist_size(djlc_dlist_t *list) {
    return list->size;
}

/*============================================================================
 * NODE OPERATIONS - O(1)
 *===========================================================================*/

/* Create a new node */
static inline djlc_dlist_node_t *djlc_dlist_new_node(int key, int val) {
    djlc_dlist_node_t *node = (djlc_dlist_node_t *)malloc(sizeof(djlc_dlist_node_t));
    if (node) {
        node->key = key;
        node->val = val;
        node->prev = node->next = NULL;
    }
    return node;
}

/* Remove a node from list (does not free) */
static inline void djlc_dlist_remove(djlc_dlist_t *list, djlc_dlist_node_t *node) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
    list->size--;
}

/* Insert node after target */
static inline void djlc_dlist_insert_after(djlc_dlist_t *list, djlc_dlist_node_t *target, 
                                           djlc_dlist_node_t *node) {
    node->next = target->next;
    node->prev = target;
    target->next->prev = node;
    target->next = node;
    list->size++;
}

/* Add to front (after head sentinel) */
static inline void djlc_dlist_push_front(djlc_dlist_t *list, djlc_dlist_node_t *node) {
    djlc_dlist_insert_after(list, list->head, node);
}

/* Add to back (before tail sentinel) */
static inline void djlc_dlist_push_back(djlc_dlist_t *list, djlc_dlist_node_t *node) {
    djlc_dlist_insert_after(list, list->tail->prev, node);
}

/* Get front node (not sentinel) */
static inline djlc_dlist_node_t *djlc_dlist_front(djlc_dlist_t *list) {
    return list->size > 0 ? list->head->next : NULL;
}

/* Get back node (not sentinel) */
static inline djlc_dlist_node_t *djlc_dlist_back(djlc_dlist_t *list) {
    return list->size > 0 ? list->tail->prev : NULL;
}

/* Pop front and return node (does not free) */
static inline djlc_dlist_node_t *djlc_dlist_pop_front(djlc_dlist_t *list) {
    if (list->size == 0) return NULL;
    djlc_dlist_node_t *node = list->head->next;
    djlc_dlist_remove(list, node);
    return node;
}

/* Pop back and return node (does not free) */
static inline djlc_dlist_node_t *djlc_dlist_pop_back(djlc_dlist_t *list) {
    if (list->size == 0) return NULL;
    djlc_dlist_node_t *node = list->tail->prev;
    djlc_dlist_remove(list, node);
    return node;
}

/* Move existing node to front - common LRU operation */
static inline void djlc_dlist_move_to_front(djlc_dlist_t *list, djlc_dlist_node_t *node) {
    djlc_dlist_remove(list, node);
    djlc_dlist_push_front(list, node);
}

#endif /* DJLC_DOUBLY_LIST_H */

