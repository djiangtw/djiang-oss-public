/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * linked_list.h - Singly Linked List utilities
 *
 * Common operations for LeetCode ListNode structure.
 * Compatible with standard LeetCode definition.
 *
 * Author: Danny Jiang
 * Created: 2025-12-15
 */

#ifndef DJLC_LINKED_LIST_H
#define DJLC_LINKED_LIST_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/*============================================================================
 * Standard LeetCode ListNode definition
 *===========================================================================*/

#ifndef DJLC_LISTNODE_DEFINED
#define DJLC_LISTNODE_DEFINED

struct ListNode {
    int val;
    struct ListNode *next;
};

#endif /* DJLC_LISTNODE_DEFINED */

/*============================================================================
 * Creation utilities
 *===========================================================================*/

/**
 * Create a new ListNode with given value
 */
static inline struct ListNode* djlc_list_new_node(int val) {
    struct ListNode *node = (struct ListNode *)malloc(sizeof(struct ListNode));
    if (node) {
        node->val = val;
        node->next = NULL;
    }
    return node;
}

/**
 * Create a linked list from an array
 * Returns head of the list, or NULL if empty/failed
 */
static inline struct ListNode* djlc_list_create(int *arr, int size) {
    if (!arr || size <= 0) return NULL;
    
    struct ListNode *head = djlc_list_new_node(arr[0]);
    if (!head) return NULL;
    
    struct ListNode *curr = head;
    for (int i = 1; i < size; i++) {
        curr->next = djlc_list_new_node(arr[i]);
        if (!curr->next) {
            /* Allocation failed, clean up and return */
            while (head) {
                struct ListNode *tmp = head;
                head = head->next;
                free(tmp);
            }
            return NULL;
        }
        curr = curr->next;
    }
    
    return head;
}

/*============================================================================
 * Print utilities
 *===========================================================================*/

/**
 * Print list in format: [1,2,3,4,5]
 */
static inline void djlc_list_print(struct ListNode *head) {
    printf("[");
    while (head) {
        printf("%d", head->val);
        if (head->next) printf(",");
        head = head->next;
    }
    printf("]");
}

/**
 * Print list with newline
 */
static inline void djlc_list_println(struct ListNode *head) {
    djlc_list_print(head);
    printf("\n");
}

/*============================================================================
 * Memory management
 *===========================================================================*/

/**
 * Free entire list
 */
static inline void djlc_list_free(struct ListNode *head) {
    while (head) {
        struct ListNode *tmp = head;
        head = head->next;
        free(tmp);
    }
}

/*============================================================================
 * Query utilities
 *===========================================================================*/

/**
 * Get length of list
 */
static inline int djlc_list_length(struct ListNode *head) {
    int len = 0;
    while (head) {
        len++;
        head = head->next;
    }
    return len;
}

/**
 * Get nth node (0-indexed), NULL if out of bounds
 */
static inline struct ListNode* djlc_list_get(struct ListNode *head, int n) {
    while (head && n > 0) {
        head = head->next;
        n--;
    }
    return head;
}

/**
 * Get tail node, NULL if empty
 */
static inline struct ListNode* djlc_list_tail(struct ListNode *head) {
    if (!head) return NULL;
    while (head->next) {
        head = head->next;
    }
    return head;
}

/*============================================================================
 * Comparison utilities
 *===========================================================================*/

/**
 * Compare two lists for equality
 */
static inline int djlc_list_equal(struct ListNode *a, struct ListNode *b) {
    while (a && b) {
        if (a->val != b->val) return 0;
        a = a->next;
        b = b->next;
    }
    return a == NULL && b == NULL;
}

/*============================================================================
 * Merge utilities
 *===========================================================================*/

/**
 * Merge two sorted lists into one sorted list
 * Uses dummy head technique for clean code
 * Time: O(n + m), Space: O(1)
 */
static inline struct ListNode* djlc_list_merge_two(struct ListNode *l1, struct ListNode *l2) {
    struct ListNode dummy = {0, NULL};
    struct ListNode *tail = &dummy;

    while (l1 && l2) {
        if (l1->val <= l2->val) {
            tail->next = l1;
            l1 = l1->next;
        } else {
            tail->next = l2;
            l2 = l2->next;
        }
        tail = tail->next;
    }
    tail->next = l1 ? l1 : l2;

    return dummy.next;
}

/**
 * Merge k sorted lists using divide and conquer
 * Time: O(N log k), Space: O(log k) recursion
 */
static inline struct ListNode* djlc_list_merge_range(struct ListNode **lists, int left, int right) {
    if (left > right) return NULL;
    if (left == right) return lists[left];

    int mid = left + (right - left) / 2;
    struct ListNode *leftMerged = djlc_list_merge_range(lists, left, mid);
    struct ListNode *rightMerged = djlc_list_merge_range(lists, mid + 1, right);

    return djlc_list_merge_two(leftMerged, rightMerged);
}

/**
 * Merge k sorted lists
 */
static inline struct ListNode* djlc_list_merge_k(struct ListNode **lists, int k) {
    if (k == 0) return NULL;
    return djlc_list_merge_range(lists, 0, k - 1);
}

/*============================================================================
 * Reverse utilities
 *===========================================================================*/

/**
 * Reverse entire list
 */
static inline struct ListNode* djlc_list_reverse(struct ListNode *head) {
    struct ListNode *prev = NULL;
    while (head) {
        struct ListNode *next = head->next;
        head->next = prev;
        prev = head;
        head = next;
    }
    return prev;
}

/**
 * Reverse first n nodes, returns new head
 * Also sets *tail to point to the last node of reversed portion
 */
static inline struct ListNode* djlc_list_reverse_n(struct ListNode *head, int n,
                                                    struct ListNode **tail) {
    struct ListNode *prev = NULL;
    struct ListNode *curr = head;

    for (int i = 0; i < n && curr; i++) {
        struct ListNode *next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }

    if (tail) *tail = head;  /* Original head is now tail */
    if (head) head->next = curr;  /* Connect to remaining list */

    return prev;
}

#endif /* DJLC_LINKED_LIST_H */

