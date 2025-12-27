/**
 * djlc - Danny Jiang's LeetCode Library in C
 *
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025 Danny Jiang
 * https://github.com/djiangtw/djiang-oss-public
 *
 * A C library for solving LeetCode problems.
 * Provides common data structures and algorithms.
 *
 * Components:
 *   - compare.h         : Comparison functions for qsort
 *   - utils.h           : Common utilities (swap, min, max, abs, gcd, etc.)
 *   - vector.h          : Dynamic array (vector)
 *   - hashmap_int.h     : Integer key-value hashmap
 *   - hashmap_str.h     : String key hashmap (word counting)
 *   - string_utils.h    : String utilities (hash, sort, reverse, etc.)
 *   - stack.h           : Stack (char and int versions)
 *   - queue.h           : Queue for BFS (int and pointer versions)
 *   - deque.h           : Double-ended queue (monotonic queue, sliding window)
 *   - circular_buffer.h : Circular buffer (moving average, ring buffer)
 *   - heap.h            : Binary heap / priority queue
 *   - linked_list.h     : Singly linked list utilities
 *   - doubly_list.h     : Doubly linked list (LRU cache pattern)
 *   - tree.h            : Binary tree utilities (TreeNode, build, free)
 *   - nary_tree.h       : N-ary tree utilities
 *   - trie.h            : Trie (prefix tree) for string matching
 *   - bit_trie.h        : Binary Trie (XOR operations)
 *   - union_find.h      : Disjoint Set Union (Union-Find)
 *   - graph.h           : Adjacency list graph (topological sort)
 *   - bit.h             : Binary Indexed Tree / Fenwick Tree
 *   - interval.h        : Interval utilities (merge, overlap, scheduling)
 *   - matrix.h          : Matrix utilities (transpose, rotate, etc.)
 *   - mod_math.h        : Modular arithmetic (add, mul, pow, nCr)
 *   - random.h          : Random utilities (shuffle, reservoir sampling)
 */

#ifndef DJLC_H
#define DJLC_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Core utilities */
#include "compare.h"
#include "utils.h"
#include "vector.h"

/* Data structures */
#include "hashmap_int.h"
#include "hashmap_str.h"
#include "stack.h"
#include "queue.h"
#include "deque.h"
#include "circular_buffer.h"
#include "heap.h"
#include "linked_list.h"
#include "doubly_list.h"
#include "tree.h"
#include "nary_tree.h"
#include "trie.h"
#include "bit_trie.h"
#include "union_find.h"
#include "graph.h"
#include "bit.h"

/* Domain-specific utilities */
#include "string_utils.h"
#include "matrix.h"
#include "mod_math.h"
#include "random.h"
#include "interval.h"

#endif /* DJLC_H */

