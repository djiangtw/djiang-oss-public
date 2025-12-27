/**
 * test_djlc.c - Test djlc library components
 *
 * Build: make test
 */

#include <stdio.h>
#include <assert.h>
#include "djlc.h"

void test_stack_int(void) {
    printf("Testing stack_int... ");
    
    djlc_stack_int_t s;
    assert(djlc_stack_int_init(&s, 4));
    
    assert(djlc_stack_int_empty(&s));
    
    djlc_stack_int_push(&s, 10);
    djlc_stack_int_push(&s, 20);
    djlc_stack_int_push(&s, 30);
    
    assert(djlc_stack_int_size(&s) == 3);
    
    int val;
    djlc_stack_int_peek(&s, &val);
    assert(val == 30);
    
    djlc_stack_int_pop(&s, &val);
    assert(val == 30);
    
    djlc_stack_int_pop(&s, &val);
    assert(val == 20);
    
    /* Test grow */
    djlc_stack_int_push(&s, 1);
    djlc_stack_int_push(&s, 2);
    djlc_stack_int_push(&s, 3);
    djlc_stack_int_push(&s, 4);
    djlc_stack_int_push(&s, 5);  /* Should trigger grow */
    assert(djlc_stack_int_size(&s) == 6);
    
    djlc_stack_int_free(&s);
    printf("OK\n");
}

void test_stack_char(void) {
    printf("Testing stack_char... ");
    
    djlc_stack_char_t s;
    assert(djlc_stack_char_init(&s, 8));
    
    djlc_stack_char_push(&s, '(');
    djlc_stack_char_push(&s, '[');
    djlc_stack_char_push(&s, '{');
    
    char c;
    djlc_stack_char_pop(&s, &c);
    assert(c == '{');
    
    djlc_stack_char_pop(&s, &c);
    assert(c == '[');
    
    djlc_stack_char_pop(&s, &c);
    assert(c == '(');
    
    assert(djlc_stack_char_empty(&s));
    
    djlc_stack_char_free(&s);
    printf("OK\n");
}

void test_stack_char_static(void) {
    printf("Testing stack_char static macros... ");
    
    DJLC_STACK_CHAR_STATIC_DEFINE(mystack, 100);
    
    assert(DJLC_STACK_CHAR_EMPTY(mystack));
    
    DJLC_STACK_CHAR_PUSH(mystack, 'a');
    DJLC_STACK_CHAR_PUSH(mystack, 'b');
    DJLC_STACK_CHAR_PUSH(mystack, 'c');
    
    assert(DJLC_STACK_CHAR_SIZE(mystack) == 3);
    assert(DJLC_STACK_CHAR_PEEK(mystack) == 'c');
    assert(DJLC_STACK_CHAR_POP(mystack) == 'c');
    assert(DJLC_STACK_CHAR_POP(mystack) == 'b');
    assert(DJLC_STACK_CHAR_POP(mystack) == 'a');
    assert(DJLC_STACK_CHAR_EMPTY(mystack));
    
    printf("OK\n");
}

void test_linked_list(void) {
    printf("Testing linked_list... ");
    
    /* Create list from array */
    int arr[] = {1, 2, 3, 4, 5};
    struct ListNode *head = djlc_list_create(arr, 5);
    assert(head != NULL);
    
    /* Test length */
    assert(djlc_list_length(head) == 5);
    
    /* Test get */
    assert(djlc_list_get(head, 0)->val == 1);
    assert(djlc_list_get(head, 2)->val == 3);
    assert(djlc_list_get(head, 4)->val == 5);
    assert(djlc_list_get(head, 5) == NULL);
    
    /* Test tail */
    assert(djlc_list_tail(head)->val == 5);
    
    /* Test equal */
    int arr2[] = {1, 2, 3, 4, 5};
    struct ListNode *head2 = djlc_list_create(arr2, 5);
    assert(djlc_list_equal(head, head2));
    
    int arr3[] = {1, 2, 3};
    struct ListNode *head3 = djlc_list_create(arr3, 3);
    assert(!djlc_list_equal(head, head3));
    
    /* Print test */
    printf("\n  List: ");
    djlc_list_println(head);
    
    /* Free all */
    djlc_list_free(head);
    djlc_list_free(head2);
    djlc_list_free(head3);
    
    printf("  OK\n");
}

void test_hashmap_int(void) {
    printf("Testing hashmap_int... ");

    djlc_hashmap_int_t hm;
    assert(djlc_hm_init(&hm, 64));

    djlc_hm_put(&hm, 42, 100);
    djlc_hm_put(&hm, -5, 200);
    djlc_hm_put(&hm, 0, 300);

    int val;
    assert(djlc_hm_get(&hm, 42, &val) && val == 100);
    assert(djlc_hm_get(&hm, -5, &val) && val == 200);
    assert(djlc_hm_get(&hm, 0, &val) && val == 300);
    assert(!djlc_hm_get(&hm, 999, &val));

    assert(djlc_hm_contains(&hm, 42));
    assert(!djlc_hm_contains(&hm, 999));

    djlc_hm_free(&hm);
    printf("OK\n");
}

void test_hashmap_str(void) {
    printf("Testing hashmap_str... ");

    djlc_hashmap_str_t *hm = djlc_hms_create(64);
    assert(hm != NULL);

    /* Test put and get */
    djlc_hms_put_s(hm, "foo", 1);
    djlc_hms_put_s(hm, "bar", 2);
    djlc_hms_put_s(hm, "baz", 3);

    assert(djlc_hms_get_s(hm, "foo") == 1);
    assert(djlc_hms_get_s(hm, "bar") == 2);
    assert(djlc_hms_get_s(hm, "baz") == 3);
    assert(djlc_hms_get_s(hm, "qux") == 0);

    /* Test add (increment) */
    djlc_hms_add_s(hm, "foo", 5);
    assert(djlc_hms_get_s(hm, "foo") == 6);

    /* Test contains */
    assert(djlc_hms_contains_s(hm, "bar"));
    assert(!djlc_hms_contains_s(hm, "xyz"));

    /* Test with length */
    djlc_hms_put(hm, "hello", 3, 99);  /* Only "hel" */
    assert(djlc_hms_get(hm, "hel", 3) == 99);

    djlc_hms_destroy(hm);
    printf("OK\n");
}

void test_trie(void) {
    printf("Testing trie... ");

    djlc_trie_t *trie = djlc_trie_create();
    assert(trie != NULL);

    /* Insert words */
    djlc_trie_insert_s(trie, "apple");
    djlc_trie_insert_s(trie, "app");
    djlc_trie_insert_s(trie, "banana");
    djlc_trie_insert_s(trie, "app");  /* Duplicate */

    assert(trie->word_count == 3);

    /* Search */
    assert(djlc_trie_contains_s(trie, "apple"));
    assert(djlc_trie_contains_s(trie, "app"));
    assert(djlc_trie_contains_s(trie, "banana"));
    assert(!djlc_trie_contains_s(trie, "appl"));
    assert(!djlc_trie_contains_s(trie, "ban"));

    /* Count */
    assert(djlc_trie_get_count_s(trie, "app") == 2);
    assert(djlc_trie_get_count_s(trie, "apple") == 1);

    /* Prefix */
    assert(djlc_trie_starts_with_s(trie, "app") != NULL);
    assert(djlc_trie_starts_with_s(trie, "ban") != NULL);
    assert(djlc_trie_starts_with_s(trie, "xyz") == NULL);

    djlc_trie_destroy(trie);
    printf("OK\n");
}

void test_list_merge(void) {
    printf("Testing list merge... ");

    /* Create two sorted lists */
    int arr1[] = {1, 3, 5};
    int arr2[] = {2, 4, 6};
    struct ListNode *l1 = djlc_list_create(arr1, 3);
    struct ListNode *l2 = djlc_list_create(arr2, 3);

    /* Merge */
    struct ListNode *merged = djlc_list_merge_two(l1, l2);

    /* Verify */
    int expected[] = {1, 2, 3, 4, 5, 6};
    struct ListNode *exp = djlc_list_create(expected, 6);
    assert(djlc_list_equal(merged, exp));

    djlc_list_free(merged);
    djlc_list_free(exp);
    printf("OK\n");
}

void test_list_reverse(void) {
    printf("Testing list reverse... ");

    int arr[] = {1, 2, 3, 4, 5};
    struct ListNode *head = djlc_list_create(arr, 5);

    head = djlc_list_reverse(head);

    int expected[] = {5, 4, 3, 2, 1};
    struct ListNode *exp = djlc_list_create(expected, 5);
    assert(djlc_list_equal(head, exp));

    djlc_list_free(head);
    djlc_list_free(exp);
    printf("OK\n");
}

void test_compare(void) {
    printf("Testing compare... ");

    int arr[] = {5, 2, 8, 1, 9, 3};
    qsort(arr, 6, sizeof(int), djlc_cmp_int_asc);
    assert(arr[0] == 1 && arr[5] == 9);

    qsort(arr, 6, sizeof(int), djlc_cmp_int_desc);
    assert(arr[0] == 9 && arr[5] == 1);

    char str[] = "dcba";
    qsort(str, 4, sizeof(char), djlc_cmp_char_asc);
    assert(strcmp(str, "abcd") == 0);

    printf("OK\n");
}

void test_utils(void) {
    printf("Testing utils... ");

    /* min/max */
    assert(djlc_min(3, 7) == 3);
    assert(djlc_max(3, 7) == 7);
    assert(djlc_min3(5, 2, 8) == 2);
    assert(djlc_max3(5, 2, 8) == 8);

    /* abs */
    assert(djlc_abs(-5) == 5);
    assert(djlc_abs(5) == 5);

    /* swap */
    int a = 10, b = 20;
    djlc_swap_int(&a, &b);
    assert(a == 20 && b == 10);

    /* clamp */
    assert(djlc_clamp(5, 0, 10) == 5);
    assert(djlc_clamp(-5, 0, 10) == 0);
    assert(djlc_clamp(15, 0, 10) == 10);

    /* gcd/lcm */
    assert(djlc_gcd(12, 8) == 4);
    assert(djlc_lcm(4, 6) == 12);

    /* pow_mod */
    assert(djlc_pow_mod(2, 10, 1000) == 24);  /* 1024 % 1000 */

    /* reverse */
    int arr[] = {1, 2, 3, 4, 5};
    djlc_reverse_int(arr, 5);
    assert(arr[0] == 5 && arr[4] == 1);

    printf("OK\n");
}

void test_vector(void) {
    printf("Testing vector... ");

    djlc_vector_t vec;
    assert(djlc_vec_init(&vec, 4));

    int vals[] = {10, 20, 30, 40, 50, 60};
    for (int i = 0; i < 6; i++) {
        djlc_vec_push(&vec, &vals[i]);
    }
    assert(djlc_vec_size(&vec) == 6);
    assert(*(int*)djlc_vec_get(&vec, 0) == 10);
    assert(*(int*)djlc_vec_get(&vec, 5) == 60);

    int* popped = (int*)djlc_vec_pop(&vec);
    assert(*popped == 60);
    assert(djlc_vec_size(&vec) == 5);

    djlc_vec_free(&vec);

    /* Test int vector */
    djlc_vec_int_t vi;
    djlc_vec_int_init(&vi, 4);
    for (int i = 0; i < 10; i++) {
        djlc_vec_int_push(&vi, i * 10);
    }
    assert(vi.size == 10);
    assert(vi.data[9] == 90);
    djlc_vec_int_free(&vi);

    printf("OK\n");
}

void test_string_utils(void) {
    printf("Testing string_utils... ");

    /* hash */
    unsigned int h1 = djlc_hash_str("hello");
    unsigned int h2 = djlc_hash_str("hello");
    unsigned int h3 = djlc_hash_str("world");
    assert(h1 == h2);
    assert(h1 != h3);

    /* sort_str */
    char* sorted = djlc_sort_str("dcba");
    assert(strcmp(sorted, "abcd") == 0);
    free(sorted);

    /* strdup */
    char* dup = djlc_strdup("test");
    assert(strcmp(dup, "test") == 0);
    free(dup);

    /* reverse */
    char str[] = "hello";
    djlc_str_reverse(str);
    assert(strcmp(str, "olleh") == 0);

    /* count */
    assert(djlc_str_count_char("banana", 'a') == 3);

    /* freq */
    int freq[26];
    djlc_str_freq_lower("aabbc", freq);
    assert(freq[0] == 2);  /* 'a' */
    assert(freq[1] == 2);  /* 'b' */
    assert(freq[2] == 1);  /* 'c' */

    printf("OK\n");
}

void test_matrix(void) {
    printf("Testing matrix... ");

    /* Allocate 3x3 matrix */
    int** m = djlc_matrix_alloc(3, 3);
    int val = 1;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            m[i][j] = val++;
        }
    }
    /* m = [[1,2,3],[4,5,6],[7,8,9]] */

    /* Test transpose */
    int** copy = djlc_matrix_copy(m, 3, 3);
    djlc_matrix_transpose(copy, 3);
    assert(copy[0][1] == 4);  /* was m[1][0] */
    assert(copy[1][0] == 2);  /* was m[0][1] */
    djlc_matrix_free(copy, 3);

    /* Test rotate 90 CW */
    copy = djlc_matrix_copy(m, 3, 3);
    djlc_matrix_rotate_90_cw(copy, 3);
    /* [1,2,3]    [7,4,1]
       [4,5,6] -> [8,5,2]
       [7,8,9]    [9,6,3] */
    assert(copy[0][0] == 7);
    assert(copy[0][2] == 1);
    assert(copy[2][0] == 9);
    djlc_matrix_free(copy, 3);

    djlc_matrix_free(m, 3);
    printf("OK\n");
}

void test_heap(void) {
    printf("Testing heap... ");

    /* Min heap */
    djlc_heap_int_t minh;
    djlc_min_heap_int_init(&minh, 8);

    djlc_heap_int_push(&minh, 5);
    djlc_heap_int_push(&minh, 3);
    djlc_heap_int_push(&minh, 8);
    djlc_heap_int_push(&minh, 1);
    djlc_heap_int_push(&minh, 9);

    int val;
    djlc_heap_int_peek(&minh, &val);
    assert(val == 1);

    djlc_heap_int_pop(&minh, &val); assert(val == 1);
    djlc_heap_int_pop(&minh, &val); assert(val == 3);
    djlc_heap_int_pop(&minh, &val); assert(val == 5);
    djlc_heap_int_pop(&minh, &val); assert(val == 8);
    djlc_heap_int_pop(&minh, &val); assert(val == 9);
    assert(djlc_heap_int_empty(&minh));
    djlc_heap_int_free(&minh);

    /* Max heap */
    djlc_heap_int_t maxh;
    djlc_max_heap_int_init(&maxh, 8);

    djlc_heap_int_push(&maxh, 5);
    djlc_heap_int_push(&maxh, 3);
    djlc_heap_int_push(&maxh, 8);

    djlc_heap_int_pop(&maxh, &val); assert(val == 8);
    djlc_heap_int_pop(&maxh, &val); assert(val == 5);
    djlc_heap_int_pop(&maxh, &val); assert(val == 3);
    djlc_heap_int_free(&maxh);

    printf("OK\n");
}

void test_queue(void) {
    printf("Testing queue... ");

    djlc_queue_int_t q;
    djlc_queue_int_init(&q, 4);

    assert(djlc_queue_int_empty(&q));

    djlc_queue_int_push(&q, 1);
    djlc_queue_int_push(&q, 2);
    djlc_queue_int_push(&q, 3);

    int val;
    djlc_queue_int_front(&q, &val);
    assert(val == 1);

    djlc_queue_int_pop(&q, &val); assert(val == 1);
    djlc_queue_int_pop(&q, &val); assert(val == 2);

    /* Push more to test circular behavior */
    djlc_queue_int_push(&q, 4);
    djlc_queue_int_push(&q, 5);
    djlc_queue_int_push(&q, 6);
    djlc_queue_int_push(&q, 7); /* Triggers grow */

    djlc_queue_int_pop(&q, &val); assert(val == 3);
    djlc_queue_int_pop(&q, &val); assert(val == 4);

    djlc_queue_int_free(&q);
    printf("OK\n");
}

void test_union_find(void) {
    printf("Testing union_find... ");

    djlc_uf_t uf;
    djlc_uf_init(&uf, 10);

    assert(djlc_uf_count(&uf) == 10);

    djlc_uf_union(&uf, 0, 1);
    djlc_uf_union(&uf, 2, 3);
    djlc_uf_union(&uf, 0, 2);

    assert(djlc_uf_connected(&uf, 0, 3));
    assert(djlc_uf_connected(&uf, 1, 2));
    assert(!djlc_uf_connected(&uf, 0, 5));
    assert(djlc_uf_count(&uf) == 7);

    djlc_uf_free(&uf);
    printf("OK\n");
}

void test_tree(void) {
    printf("Testing tree... ");

    /* Build tree: [3,9,20,null,null,15,7] */
    int arr[] = {3, 9, 20, DJLC_TREE_NULL_VAL, DJLC_TREE_NULL_VAL, 15, 7};
    struct TreeNode* root = djlc_tree_build(arr, 7);

    assert(root->val == 3);
    assert(root->left->val == 9);
    assert(root->right->val == 20);
    assert(root->right->left->val == 15);
    assert(root->right->right->val == 7);

    assert(djlc_tree_count(root) == 5);
    assert(djlc_tree_height(root) == 3);

    djlc_tree_free(root);
    printf("OK\n");
}

void test_mod_math(void) {
    printf("Testing mod_math... ");

    long long mod = DJLC_MOD_1E9_7;

    /* Basic ops */
    assert(djlc_mod_add(1000000006, 5, mod) == 4);
    assert(djlc_mod_sub(5, 10, mod) == mod - 5);
    assert(djlc_mod_mul(100000, 100000, mod) == (10000000000LL % mod));

    /* Power */
    assert(djlc_mod_pow(2, 10, mod) == 1024);
    assert(djlc_mod_pow(2, 30, mod) == (1LL << 30) % mod);

    /* Inverse */
    long long inv2 = djlc_mod_inv(2, mod);
    assert(djlc_mod_mul(2, inv2, mod) == 1);

    /* Factorial and nCr */
    djlc_mod_factorial_t mf;
    djlc_mod_factorial_init(&mf, 100, mod);

    assert(djlc_mod_ncr(&mf, 5, 2) == 10);   /* 5C2 = 10 */
    assert(djlc_mod_ncr(&mf, 10, 5) == 252); /* 10C5 = 252 */
    assert(djlc_mod_npr(&mf, 5, 2) == 20);   /* 5P2 = 20 */

    djlc_mod_factorial_free(&mf);
    printf("OK\n");
}

int main(void) {
    printf("\n=== djlc Library Tests ===\n\n");

    /* Original components */
    test_stack_int();
    test_stack_char();
    test_stack_char_static();
    test_linked_list();
    test_hashmap_int();
    test_hashmap_str();
    test_trie();
    test_list_merge();
    test_list_reverse();

    /* New components (2025-12-17) */
    test_compare();
    test_utils();
    test_vector();
    test_string_utils();
    test_matrix();

    /* New components (2025-12-24) */
    test_heap();
    test_queue();
    test_union_find();
    test_tree();
    test_mod_math();

    printf("\n=== All tests passed! ===\n\n");
    return 0;
}

