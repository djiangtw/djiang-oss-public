/**
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public

 * graph.h - Graph with Adjacency List
 *
 * Directed graph using adjacency list representation.
 * Supports topological sort, cycle detection, BFS/DFS.
 *
 * Author: Danny Jiang
 * Created: 2025-12-24
 */

#ifndef DJLC_GRAPH_H
#define DJLC_GRAPH_H

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

/*============================================================================
 * ADJACENCY LIST GRAPH
 *===========================================================================*/

typedef struct {
    int **adj;        /* adj[i] = array of neighbors of node i */
    int *adj_size;    /* Number of neighbors for each node */
    int *adj_cap;     /* Capacity of each adjacency list */
    int *indegree;    /* In-degree of each node (for topological sort) */
    int n;            /* Number of nodes */
} djlc_graph_t;

static inline bool djlc_graph_init(djlc_graph_t *g, int n) {
    g->n = n;
    g->adj = (int **)malloc(n * sizeof(int *));
    g->adj_size = (int *)calloc(n, sizeof(int));
    g->adj_cap = (int *)malloc(n * sizeof(int));
    g->indegree = (int *)calloc(n, sizeof(int));
    if (!g->adj || !g->adj_size || !g->adj_cap || !g->indegree) {
        free(g->adj); free(g->adj_size); free(g->adj_cap); free(g->indegree);
        return false;
    }
    for (int i = 0; i < n; i++) {
        g->adj_cap[i] = 4;
        g->adj[i] = (int *)malloc(4 * sizeof(int));
    }
    return true;
}

static inline void djlc_graph_free(djlc_graph_t *g) {
    for (int i = 0; i < g->n; i++) {
        free(g->adj[i]);
    }
    free(g->adj);
    free(g->adj_size);
    free(g->adj_cap);
    free(g->indegree);
    g->n = 0;
}

/* Add directed edge from u to v */
static inline bool djlc_graph_add_edge(djlc_graph_t *g, int u, int v) {
    if (g->adj_size[u] >= g->adj_cap[u]) {
        g->adj_cap[u] *= 2;
        g->adj[u] = (int *)realloc(g->adj[u], g->adj_cap[u] * sizeof(int));
        if (!g->adj[u]) return false;
    }
    g->adj[u][g->adj_size[u]++] = v;
    g->indegree[v]++;
    return true;
}

/* Reset indegree (call before topological sort if graph was modified) */
static inline void djlc_graph_reset_indegree(djlc_graph_t *g) {
    memset(g->indegree, 0, g->n * sizeof(int));
    for (int u = 0; u < g->n; u++) {
        for (int i = 0; i < g->adj_size[u]; i++) {
            g->indegree[g->adj[u][i]]++;
        }
    }
}

/*============================================================================
 * TOPOLOGICAL SORT (Kahn's Algorithm)
 * Returns: number of nodes in topological order
 *          If return value < n, there's a cycle
 *===========================================================================*/

static inline int djlc_graph_topo_sort(djlc_graph_t *g, int *result) {
    int *indeg = (int *)malloc(g->n * sizeof(int));
    memcpy(indeg, g->indegree, g->n * sizeof(int));
    
    int *queue = (int *)malloc(g->n * sizeof(int));
    int front = 0, rear = 0;
    
    /* Enqueue all nodes with indegree 0 */
    for (int i = 0; i < g->n; i++) {
        if (indeg[i] == 0) queue[rear++] = i;
    }
    
    int count = 0;
    while (front < rear) {
        int u = queue[front++];
        if (result) result[count] = u;
        count++;
        
        for (int i = 0; i < g->adj_size[u]; i++) {
            int v = g->adj[u][i];
            if (--indeg[v] == 0) queue[rear++] = v;
        }
    }
    
    free(indeg);
    free(queue);
    return count;  /* count < g->n means cycle exists */
}

/* Check if graph has cycle (useful for course schedule) */
static inline bool djlc_graph_has_cycle(djlc_graph_t *g) {
    return djlc_graph_topo_sort(g, NULL) < g->n;
}

/* Check if all nodes can be visited (for course schedule) */
static inline bool djlc_graph_can_finish(djlc_graph_t *g) {
    return djlc_graph_topo_sort(g, NULL) == g->n;
}

/*============================================================================
 * BFS on Graph
 *===========================================================================*/

static inline int djlc_graph_bfs(djlc_graph_t *g, int start, int *dist) {
    int *queue = (int *)malloc(g->n * sizeof(int));
    int front = 0, rear = 0;
    
    for (int i = 0; i < g->n; i++) dist[i] = -1;
    dist[start] = 0;
    queue[rear++] = start;
    
    int max_dist = 0;
    while (front < rear) {
        int u = queue[front++];
        for (int i = 0; i < g->adj_size[u]; i++) {
            int v = g->adj[u][i];
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                if (dist[v] > max_dist) max_dist = dist[v];
                queue[rear++] = v;
            }
        }
    }
    free(queue);
    return max_dist;
}

#endif /* DJLC_GRAPH_H */

