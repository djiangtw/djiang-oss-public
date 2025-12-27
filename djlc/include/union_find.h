/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * union_find.h - Disjoint Set Union (Union-Find)
 *
 * Efficient Union-Find with path compression and union by rank.
 * Time complexity: O(alpha(n)) per operation (nearly constant).
 *
 * == Common Algorithm Patterns ==
 *
 * 1. Connected Components (LC 0200, 0547):
 *    djlc_uf_init(&uf, n);
 *    for each edge (u, v): djlc_uf_union(&uf, u, v);
 *    answer = djlc_uf_count(&uf);
 *
 * 2. Prime Factorization Grouping (LC 0952):
 *    for each num: for f=2 to sqrt(num): if num%f==0:
 *        djlc_uf_union(&uf, num, f);
 *        djlc_uf_union(&uf, num, num/f);
 *
 * 3. Cycle Detection in Undirected Graph (LC 0684):
 *    for each edge (u, v):
 *        if (djlc_uf_connected(&uf, u, v)) return edge;  // cycle found
 *        djlc_uf_union(&uf, u, v);
 *
 * 4. Weighted Union-Find for Ratios (LC 0399):
 *    Use djlc_uf_weighted_* functions to track value ratios
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_UNION_FIND_H
#define DJLC_UNION_FIND_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    int *parent;
    int *rank;
    int size;      /* number of elements */
    int components; /* number of disjoint sets */
} djlc_uf_t;

/* Initialize Union-Find with n elements (0 to n-1) */
static inline bool djlc_uf_init(djlc_uf_t *uf, int n) {
    uf->parent = (int *)malloc(n * sizeof(int));
    uf->rank = (int *)malloc(n * sizeof(int));
    if (!uf->parent || !uf->rank) {
        free(uf->parent);
        free(uf->rank);
        return false;
    }
    for (int i = 0; i < n; i++) {
        uf->parent[i] = i;
        uf->rank[i] = 0;
    }
    uf->size = n;
    uf->components = n;
    return true;
}

static inline void djlc_uf_free(djlc_uf_t *uf) {
    if (uf->parent) { free(uf->parent); uf->parent = NULL; }
    if (uf->rank) { free(uf->rank); uf->rank = NULL; }
    uf->size = 0;
    uf->components = 0;
}

/* Find with path compression */
static inline int djlc_uf_find(djlc_uf_t *uf, int x) {
    if (uf->parent[x] != x) {
        uf->parent[x] = djlc_uf_find(uf, uf->parent[x]);
    }
    return uf->parent[x];
}

/* Union by rank, returns true if merge happened (were different sets) */
static inline bool djlc_uf_union(djlc_uf_t *uf, int x, int y) {
    int px = djlc_uf_find(uf, x);
    int py = djlc_uf_find(uf, y);
    if (px == py) return false;
    
    /* Union by rank */
    if (uf->rank[px] < uf->rank[py]) {
        uf->parent[px] = py;
    } else if (uf->rank[px] > uf->rank[py]) {
        uf->parent[py] = px;
    } else {
        uf->parent[py] = px;
        uf->rank[px]++;
    }
    uf->components--;
    return true;
}

/* Check if x and y are in the same set */
static inline bool djlc_uf_connected(djlc_uf_t *uf, int x, int y) {
    return djlc_uf_find(uf, x) == djlc_uf_find(uf, y);
}

/* Get number of disjoint sets */
static inline int djlc_uf_count(djlc_uf_t *uf) {
    return uf->components;
}

/*============================================================================
 * WEIGHTED UNION-FIND - For problems like "Evaluate Division"
 * Each edge has a weight representing the ratio/relationship
 *===========================================================================*/

typedef struct {
    int *parent;
    double *weight;  /* weight[i] = value of i relative to parent[i] */
    int size;
} djlc_uf_weighted_t;

static inline bool djlc_uf_weighted_init(djlc_uf_weighted_t *uf, int n) {
    uf->parent = (int *)malloc(n * sizeof(int));
    uf->weight = (double *)malloc(n * sizeof(double));
    if (!uf->parent || !uf->weight) {
        free(uf->parent);
        free(uf->weight);
        return false;
    }
    for (int i = 0; i < n; i++) {
        uf->parent[i] = i;
        uf->weight[i] = 1.0;
    }
    uf->size = n;
    return true;
}

static inline void djlc_uf_weighted_free(djlc_uf_weighted_t *uf) {
    if (uf->parent) { free(uf->parent); uf->parent = NULL; }
    if (uf->weight) { free(uf->weight); uf->weight = NULL; }
    uf->size = 0;
}

/* Find with path compression, also updates weights */
static inline int djlc_uf_weighted_find(djlc_uf_weighted_t *uf, int x) {
    if (uf->parent[x] != x) {
        int root = djlc_uf_weighted_find(uf, uf->parent[x]);
        uf->weight[x] *= uf->weight[uf->parent[x]];
        uf->parent[x] = root;
    }
    return uf->parent[x];
}

/* Union: x / y = val */
static inline void djlc_uf_weighted_union(djlc_uf_weighted_t *uf, int x, int y, double val) {
    int px = djlc_uf_weighted_find(uf, x);
    int py = djlc_uf_weighted_find(uf, y);
    if (px == py) return;
    /* weight[px] should be such that x/y = val */
    /* x = weight[x] * px, y = weight[y] * py */
    /* val = x/y = (weight[x] * px) / (weight[y] * py) */
    /* So px = val * weight[y] / weight[x] * py */
    uf->parent[px] = py;
    uf->weight[px] = val * uf->weight[y] / uf->weight[x];
}

/* Query: returns x / y if connected, -1.0 otherwise */
static inline double djlc_uf_weighted_query(djlc_uf_weighted_t *uf, int x, int y) {
    int px = djlc_uf_weighted_find(uf, x);
    int py = djlc_uf_weighted_find(uf, y);
    if (px != py) return -1.0;
    return uf->weight[x] / uf->weight[y];
}

#endif /* DJLC_UNION_FIND_H */

