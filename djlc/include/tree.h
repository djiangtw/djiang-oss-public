/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * tree.h - Binary Tree Utilities
 *
 * TreeNode definition and helper functions for building/freeing trees.
 * Compatible with LeetCode's TreeNode definition.
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_TREE_H
#define DJLC_TREE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <limits.h>

/*============================================================================
 * TREENODE DEFINITION
 * Note: LeetCode provides this, so only define for local testing
 *===========================================================================*/

#ifndef DJLC_TREENODE_DEFINED
#define DJLC_TREENODE_DEFINED

struct TreeNode {
    int val;
    struct TreeNode *left;
    struct TreeNode *right;
};

#endif /* DJLC_TREENODE_DEFINED */

/*============================================================================
 * NODE CREATION / DESTRUCTION
 *===========================================================================*/

static inline struct TreeNode* djlc_tree_node_create(int val) {
    struct TreeNode *node = (struct TreeNode *)malloc(sizeof(struct TreeNode));
    if (node) {
        node->val = val;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

static inline void djlc_tree_free(struct TreeNode *root) {
    if (!root) return;
    djlc_tree_free(root->left);
    djlc_tree_free(root->right);
    free(root);
}

/*============================================================================
 * BUILD TREE FROM ARRAY (LeetCode format)
 * Array format: level-order, NULL_VAL represents null nodes
 *===========================================================================*/

#define DJLC_TREE_NULL_VAL INT_MIN

/**
 * Build tree from level-order array.
 * @param arr Array of values (use DJLC_TREE_NULL_VAL for null)
 * @param size Size of array
 * @return Root of tree, or NULL if empty
 */
static inline struct TreeNode* djlc_tree_build(int *arr, int size) {
    if (size == 0 || arr[0] == DJLC_TREE_NULL_VAL) return NULL;
    
    struct TreeNode **nodes = (struct TreeNode **)malloc(size * sizeof(struct TreeNode *));
    if (!nodes) return NULL;
    
    /* Create all nodes first */
    for (int i = 0; i < size; i++) {
        if (arr[i] == DJLC_TREE_NULL_VAL) {
            nodes[i] = NULL;
        } else {
            nodes[i] = djlc_tree_node_create(arr[i]);
        }
    }
    
    /* Link parent-child relationships */
    for (int i = 0; i < size; i++) {
        if (nodes[i]) {
            int left_idx = 2 * i + 1;
            int right_idx = 2 * i + 2;
            if (left_idx < size) nodes[i]->left = nodes[left_idx];
            if (right_idx < size) nodes[i]->right = nodes[right_idx];
        }
    }
    
    struct TreeNode *root = nodes[0];
    free(nodes);
    return root;
}

/*============================================================================
 * TREE TRAVERSAL UTILITIES
 *===========================================================================*/

/* Count nodes in tree */
static inline int djlc_tree_count(struct TreeNode *root) {
    if (!root) return 0;
    return 1 + djlc_tree_count(root->left) + djlc_tree_count(root->right);
}

/* Get height of tree */
static inline int djlc_tree_height(struct TreeNode *root) {
    if (!root) return 0;
    int lh = djlc_tree_height(root->left);
    int rh = djlc_tree_height(root->right);
    return 1 + (lh > rh ? lh : rh);
}

/* Check if tree is balanced */
static inline int djlc_tree_check_balanced_helper(struct TreeNode *root) {
    if (!root) return 0;
    int lh = djlc_tree_check_balanced_helper(root->left);
    if (lh == -1) return -1;
    int rh = djlc_tree_check_balanced_helper(root->right);
    if (rh == -1) return -1;
    if (lh - rh > 1 || rh - lh > 1) return -1;
    return 1 + (lh > rh ? lh : rh);
}

static inline bool djlc_tree_is_balanced(struct TreeNode *root) {
    return djlc_tree_check_balanced_helper(root) != -1;
}

/*============================================================================
 * BST UTILITIES
 *===========================================================================*/

/* Validate BST */
static inline bool djlc_bst_validate_helper(struct TreeNode *root, long min, long max) {
    if (!root) return true;
    if (root->val <= min || root->val >= max) return false;
    return djlc_bst_validate_helper(root->left, min, root->val) &&
           djlc_bst_validate_helper(root->right, root->val, max);
}

static inline bool djlc_bst_is_valid(struct TreeNode *root) {
    return djlc_bst_validate_helper(root, LONG_MIN, LONG_MAX);
}

/* Search in BST */
static inline struct TreeNode* djlc_bst_search(struct TreeNode *root, int val) {
    while (root && root->val != val) {
        root = (val < root->val) ? root->left : root->right;
    }
    return root;
}

#endif /* DJLC_TREE_H */

