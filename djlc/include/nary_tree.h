/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * nary_tree.h - N-ary Tree
 *
 * N-ary tree structure with variable number of children.
 * Useful for:
 *   - Directory structures
 *   - Organizational hierarchies
 *   - General tree problems
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_NARY_TREE_H
#define DJLC_NARY_TREE_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/*============================================================================
 * N-ARY TREE NODE
 *===========================================================================*/

typedef struct djlc_nary_node {
    int val;
    int num_children;
    int children_cap;
    struct djlc_nary_node **children;
} djlc_nary_node_t;

/*============================================================================
 * NODE OPERATIONS
 *===========================================================================*/

static inline djlc_nary_node_t *djlc_nary_new_node(int val) {
    djlc_nary_node_t *node = (djlc_nary_node_t *)malloc(sizeof(djlc_nary_node_t));
    if (node) {
        node->val = val;
        node->num_children = 0;
        node->children_cap = 4;
        node->children = (djlc_nary_node_t **)malloc(4 * sizeof(djlc_nary_node_t *));
    }
    return node;
}

static inline void djlc_nary_free_node(djlc_nary_node_t *node) {
    if (!node) return;
    for (int i = 0; i < node->num_children; i++) {
        djlc_nary_free_node(node->children[i]);
    }
    free(node->children);
    free(node);
}

static inline bool djlc_nary_add_child(djlc_nary_node_t *parent, djlc_nary_node_t *child) {
    if (parent->num_children >= parent->children_cap) {
        parent->children_cap *= 2;
        parent->children = (djlc_nary_node_t **)realloc(
            parent->children, parent->children_cap * sizeof(djlc_nary_node_t *));
        if (!parent->children) return false;
    }
    parent->children[parent->num_children++] = child;
    return true;
}

/*============================================================================
 * TRAVERSAL - Level Order (BFS)
 *===========================================================================*/

typedef void (*djlc_nary_visitor_fn)(djlc_nary_node_t *node, int level, void *ctx);

static inline void djlc_nary_level_order(djlc_nary_node_t *root, 
                                          djlc_nary_visitor_fn visitor, void *ctx) {
    if (!root) return;
    djlc_nary_node_t **queue = (djlc_nary_node_t **)malloc(10000 * sizeof(djlc_nary_node_t *));
    int *levels = (int *)malloc(10000 * sizeof(int));
    int front = 0, rear = 0;
    queue[rear] = root;
    levels[rear++] = 0;
    
    while (front < rear) {
        djlc_nary_node_t *node = queue[front];
        int level = levels[front++];
        visitor(node, level, ctx);
        
        for (int i = 0; i < node->num_children; i++) {
            queue[rear] = node->children[i];
            levels[rear++] = level + 1;
        }
    }
    free(queue);
    free(levels);
}

/*============================================================================
 * TRAVERSAL - Pre-order (DFS)
 *===========================================================================*/

static inline void djlc_nary_preorder_helper(djlc_nary_node_t *node, int depth,
                                              djlc_nary_visitor_fn visitor, void *ctx) {
    if (!node) return;
    visitor(node, depth, ctx);
    for (int i = 0; i < node->num_children; i++) {
        djlc_nary_preorder_helper(node->children[i], depth + 1, visitor, ctx);
    }
}

static inline void djlc_nary_preorder(djlc_nary_node_t *root,
                                       djlc_nary_visitor_fn visitor, void *ctx) {
    djlc_nary_preorder_helper(root, 0, visitor, ctx);
}

/*============================================================================
 * TRAVERSAL - Post-order (DFS)
 *===========================================================================*/

static inline void djlc_nary_postorder_helper(djlc_nary_node_t *node, int depth,
                                               djlc_nary_visitor_fn visitor, void *ctx) {
    if (!node) return;
    for (int i = 0; i < node->num_children; i++) {
        djlc_nary_postorder_helper(node->children[i], depth + 1, visitor, ctx);
    }
    visitor(node, depth, ctx);
}

static inline void djlc_nary_postorder(djlc_nary_node_t *root,
                                        djlc_nary_visitor_fn visitor, void *ctx) {
    djlc_nary_postorder_helper(root, 0, visitor, ctx);
}

/*============================================================================
 * UTILITY FUNCTIONS
 *===========================================================================*/

/* Get max depth of N-ary tree */
static inline int djlc_nary_max_depth(djlc_nary_node_t *root) {
    if (!root) return 0;
    int max_child_depth = 0;
    for (int i = 0; i < root->num_children; i++) {
        int d = djlc_nary_max_depth(root->children[i]);
        if (d > max_child_depth) max_child_depth = d;
    }
    return 1 + max_child_depth;
}

/* Count total nodes */
static inline int djlc_nary_count_nodes(djlc_nary_node_t *root) {
    if (!root) return 0;
    int count = 1;
    for (int i = 0; i < root->num_children; i++) {
        count += djlc_nary_count_nodes(root->children[i]);
    }
    return count;
}

#endif /* DJLC_NARY_TREE_H */

