# djlc - Danny Jiang's LeetCode Library in C

A header-only C library for solving LeetCode problems.

## Features

Designed for competitive programming and LeetCode, djlc provides common data structures that are missing from the C standard library.

## Components

### Core Utilities
| Header | Description |
|--------|-------------|
| `compare.h` | Comparison functions for qsort |
| `utils.h` | Common utilities (swap, min, max, gcd, etc.) |
| `vector.h` | Dynamic array |

### Hash Maps
| Header | Description |
|--------|-------------|
| `hashmap_int.h` | Integer key-value hashmap |
| `hashmap_str.h` | String key hashmap |

### Linear Structures
| Header | Description |
|--------|-------------|
| `stack.h` | Stack (char and int versions) |
| `queue.h` | Queue for BFS |
| `deque.h` | Double-ended queue |
| `circular_buffer.h` | Circular buffer |
| `linked_list.h` | Singly linked list |
| `doubly_list.h` | Doubly linked list |

### Trees
| Header | Description |
|--------|-------------|
| `heap.h` | Binary heap / priority queue |
| `tree.h` | Binary tree utilities |
| `nary_tree.h` | N-ary tree utilities |
| `trie.h` | Prefix tree |
| `bit_trie.h` | Binary trie (XOR operations) |
| `bit.h` | Binary Indexed Tree / Fenwick Tree |

### Graph
| Header | Description |
|--------|-------------|
| `union_find.h` | Disjoint Set Union |
| `graph.h` | Adjacency list graph |

### Domain-Specific
| Header | Description |
|--------|-------------|
| `string_utils.h` | String utilities |
| `matrix.h` | Matrix operations |
| `mod_math.h` | Modular arithmetic |
| `interval.h` | Interval utilities |
| `random.h` | Random utilities |

## Usage

Include the main header to get all components:

```c
#include "djlc.h"
```

Or include individual headers as needed:

```c
#include "hashmap_int.h"
#include "heap.h"
```

## Build

```bash
make        # Build test
make test   # Build and run tests
make clean  # Clean build artifacts
```

## Example

```c
#include "djlc.h"

// Two Sum using hashmap
int* twoSum(int* nums, int n, int target, int* returnSize) {
    djlc_hashmap_int_t map;
    djlc_hm_int_init(&map, n * 2);
    
    for (int i = 0; i < n; i++) {
        int complement = target - nums[i];
        int j;
        if (djlc_hm_int_get(&map, complement, &j)) {
            int* result = malloc(2 * sizeof(int));
            result[0] = j;
            result[1] = i;
            *returnSize = 2;
            djlc_hm_int_free(&map);
            return result;
        }
        djlc_hm_int_put(&map, nums[i], i);
    }
    
    *returnSize = 0;
    djlc_hm_int_free(&map);
    return NULL;
}
```

## License

MIT License - see [../LICENSE](../LICENSE) for details.

