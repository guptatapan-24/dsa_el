/**
 * Skip List Implementation
 * Probabilistic data structure with expected O(log n) operations
 * Data Structures & Applications Lab Project - Pure C Implementation
 * 
 * Properties:
 * - Multiple levels of linked lists
 * - Each level skips over elements in lower levels
 * - Probabilistic level assignment (coin flip)
 * - Expected O(log n) search, insert, delete
 */

#ifndef SKIPLIST_H
#define SKIPLIST_H

#include "common.h"

#define SKIPLIST_MAX_LEVEL 16
#define SKIPLIST_P 0.5  // Probability for level up

// Skip List Node
typedef struct SkipNode {
    char id[64];                            // Key: Transaction ID
    Transaction transaction;                 // Transaction data
    struct SkipNode** forward;              // Array of forward pointers
    int level;                              // Current node level
} SkipNode;

// Skip List structure
typedef struct {
    SkipNode* header;                       // Header node
    int level;                              // Current max level
    int size;                               // Number of elements
    int operations_count;
    int level_distribution[SKIPLIST_MAX_LEVEL];  // Track levels for proof
} SkipList;

// Function declarations
SkipList* skiplist_create(void);
void skiplist_destroy(SkipList* list);

// Core operations - O(log n) expected
bool skiplist_insert(SkipList* list, const Transaction* t);
bool skiplist_search(SkipList* list, const char* id, Transaction* out_t);
bool skiplist_delete(SkipList* list, const char* id);

// Utility operations
int skiplist_get_all(SkipList* list, Transaction* out_transactions, int max_count);
int skiplist_size(SkipList* list);
int skiplist_get_operations_count(SkipList* list);
int skiplist_get_level(SkipList* list);
void skiplist_clear(SkipList* list);

// Statistics for proof
void skiplist_get_level_distribution(SkipList* list, int* distribution, int max_levels);

#endif // SKIPLIST_H
