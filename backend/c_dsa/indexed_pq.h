/**
 * Indexed Priority Queue Implementation
 * Max-heap with index tracking for efficient priority updates
 * Data Structures & Applications Lab Project - Pure C Implementation
 * 
 * Properties:
 * - O(log n) insert, extractMax, updatePriority
 * - O(1) lookup of position by key
 * - Used for budget alerts prioritized by usage percentage
 */

#ifndef INDEXED_PQ_H
#define INDEXED_PQ_H

#include "common.h"

#define IPQ_MAX_SIZE 1000

// Budget Alert item for priority queue
typedef struct {
    char category[MAX_STRING_LEN];
    double spent;
    double budget_limit;
    double priority;       // percent_used as priority (higher = more urgent)
} BudgetAlert;

// Index mapping for O(1) lookup
typedef struct {
    char key[MAX_STRING_LEN];
    int heap_index;
} IPQIndex;

// Indexed Priority Queue structure
typedef struct {
    BudgetAlert* heap;           // Array of alerts (heap)
    IPQIndex* index_map;         // Key to heap index mapping
    int size;                    // Current size
    int capacity;                // Max capacity
    int operations_count;
    int heapify_count;           // Track heapifications for proof
} IndexedPQ;

// Function declarations
IndexedPQ* ipq_create(int capacity);
void ipq_destroy(IndexedPQ* pq);

// Core operations - all O(log n)
bool ipq_insert(IndexedPQ* pq, const char* category, double spent, double limit);
bool ipq_extract_max(IndexedPQ* pq, BudgetAlert* out);
bool ipq_update_priority(IndexedPQ* pq, const char* category, double new_spent);
bool ipq_peek_max(IndexedPQ* pq, BudgetAlert* out);

// Utility operations
bool ipq_contains(IndexedPQ* pq, const char* category);
bool ipq_get_by_key(IndexedPQ* pq, const char* category, BudgetAlert* out);
int ipq_get_all_sorted(IndexedPQ* pq, BudgetAlert* out, int max_count);
int ipq_get_alerts_above_threshold(IndexedPQ* pq, double threshold, BudgetAlert* out, int max_count);

// Statistics
int ipq_size(IndexedPQ* pq);
int ipq_get_operations_count(IndexedPQ* pq);
int ipq_get_heapify_count(IndexedPQ* pq);
void ipq_clear(IndexedPQ* pq);

// Remove by key
bool ipq_remove(IndexedPQ* pq, const char* category);

#endif // INDEXED_PQ_H
