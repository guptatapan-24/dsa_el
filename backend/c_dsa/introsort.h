/**
 * IntroSort Implementation
 * Hybrid sorting algorithm: QuickSort + HeapSort + InsertionSort
 * Data Structures & Applications Lab Project - Pure C Implementation
 * 
 * Properties:
 * - O(n log n) worst-case guaranteed
 * - Starts with QuickSort for average-case performance
 * - Switches to HeapSort when recursion depth exceeds 2*log(n)
 * - Uses InsertionSort for small subarrays (< 16 elements)
 */

#ifndef INTROSORT_H
#define INTROSORT_H

#include "common.h"

#define INSERTION_SORT_THRESHOLD 16

// Statistics for proof of algorithm usage
typedef struct {
    int quicksort_partitions;
    int heapsort_calls;
    int insertion_sort_calls;
    int comparisons;
    int swaps;
    int total_operations;
} IntroSortStats;

// Function declarations

// Sort transactions by amount (descending) - for top expenses
void introsort_transactions_by_amount(Transaction* arr, int n, IntroSortStats* stats);

// Sort transactions by date (ascending)
void introsort_transactions_by_date(Transaction* arr, int n, IntroSortStats* stats);

// Sort category amounts by total (descending) - for top categories
void introsort_categories_by_amount(CategoryAmount* arr, int n, IntroSortStats* stats);

// Get top K transactions by amount
int introsort_get_top_k_expenses(Transaction* arr, int n, int k, Transaction* out, IntroSortStats* stats);

// Get top K categories by amount
int introsort_get_top_k_categories(CategoryAmount* arr, int n, int k, CategoryAmount* out, IntroSortStats* stats);

// Reset statistics
void introsort_reset_stats(IntroSortStats* stats);

// Get total operations count
int introsort_get_operations_count(IntroSortStats* stats);

#endif // INTROSORT_H
