/**
 * Max Heap Implementation for Top Spending Transactions
 * Data Structures & Applications Lab Project - Pure C Implementation
 * Operations: insert O(log n), extract max O(log n), build O(n)
 */

#ifndef HEAP_H
#define HEAP_H

#include "common.h"

#define MAX_HEAP_SIZE 1000

// Max Heap for transactions (by amount)
typedef struct {
    Transaction* data;
    int size;
    int capacity;
    int operations_count;
} MaxHeap;

// Category Max Heap
typedef struct {
    CategoryAmount* data;
    int size;
    int capacity;
    int operations_count;
} CategoryMaxHeap;

// Transaction Heap functions
MaxHeap* maxheap_create(int capacity);
void maxheap_destroy(MaxHeap* heap);
bool maxheap_insert(MaxHeap* heap, const Transaction* t);
bool maxheap_extract_max(MaxHeap* heap, Transaction* out_t);
bool maxheap_peek(MaxHeap* heap, Transaction* out_t);
int maxheap_get_top_k(MaxHeap* heap, int k, Transaction* out_transactions);
void maxheap_build(MaxHeap* heap, const Transaction* transactions, int count);
int maxheap_size(MaxHeap* heap);
void maxheap_clear(MaxHeap* heap);
int maxheap_get_operations_count(MaxHeap* heap);

// Category Heap functions
CategoryMaxHeap* cat_maxheap_create(int capacity);
void cat_maxheap_destroy(CategoryMaxHeap* heap);
bool cat_maxheap_insert(CategoryMaxHeap* heap, const CategoryAmount* ca);
bool cat_maxheap_extract_max(CategoryMaxHeap* heap, CategoryAmount* out_ca);
int cat_maxheap_get_top_k(CategoryMaxHeap* heap, int k, CategoryAmount* out_categories);
void cat_maxheap_build(CategoryMaxHeap* heap, const CategoryAmount* categories, int count);
int cat_maxheap_size(CategoryMaxHeap* heap);
void cat_maxheap_clear(CategoryMaxHeap* heap);
int cat_maxheap_get_operations_count(CategoryMaxHeap* heap);

#endif // HEAP_H
