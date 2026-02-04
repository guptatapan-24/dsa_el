/**
 * Finance Engine - Main Integration Layer (Pure C)
 * Combines all required data structures for the Finance Tracker
 * Exposes functions that Python can call via ctypes
 * 
 * Data Structures Used (as per requirements):
 * 1. Red-Black Tree - Transaction storage with O(log n) insert, O(log n + k) range query
 * 2. Skip List - Transaction lookup by ID with O(log n) expected
 * 3. IntroSort - Top K expenses with O(n log n) sorting
 * 4. Polynomial Hash Map - Budget storage with O(1) average set/get
 * 5. Indexed Priority Queue - Budget alerts sorted with O(n log n)
 * 6. Sliding Window - 7-day trend with O(7) = O(1)
 * 7. Z-Score (Welford's) - Anomaly detection with O(1) update
 * 8. Queue - Bill management with O(1) enqueue, O(n) search
 * 9. Stack - Undo operations with O(1) pop
 */

#ifndef FINANCE_ENGINE_H
#define FINANCE_ENGINE_H

#include "common.h"
#include "rbtree.h"
#include "skiplist.h"
#include "introsort.h"
#include "hashmap.h"
#include "indexed_pq.h"
#include "sliding_window.h"
#include "zscore.h"
#include "queue.h"
#include "stack.h"
#include "trie.h"

#define MAX_TRANSACTIONS 10000
#define MAX_BUDGETS 100
#define MAX_BILLS 100
#define MAX_CATEGORIES 200

// DSA Usage Statistics for proof
typedef struct {
    int rbtree_ops;         // Red-Black Tree operations
    int skiplist_ops;       // Skip List operations
    int introsort_ops;      // IntroSort operations
    int hashmap_ops;        // Polynomial HashMap operations
    int indexed_pq_ops;     // Indexed Priority Queue operations
    int sliding_window_ops; // Sliding Window operations
    int zscore_ops;         // Z-Score (Welford's) operations
    int queue_ops;          // Queue operations
    int stack_ops;          // Stack operations
    int trie_ops;           // Trie operations (for autocomplete)
    int total_ops;          // Total operations
} DSAStats;

// Finance Engine structure
typedef struct {
    // Primary data structures as per requirements
    RBTree* transaction_tree;       // Red-Black Tree: O(log n) insert, O(log n + k) range query
    SkipList* transaction_skiplist; // Skip List: O(log n) expected lookup by ID
    HashMap* budget_map;            // Polynomial HashMap: O(1) average budget operations
    HashMap* expense_map;           // Polynomial HashMap: O(1) average expense tracking
    IndexedPQ* budget_alerts_pq;    // Indexed Priority Queue: O(n log n) sorted alerts
    SlidingWindow* spending_window_7day;  // Sliding Window: O(7) = O(1) for 7-day trend
    SlidingWindow* spending_window_30day; // Sliding Window: for 30-day trend
    ZScoreTracker* anomaly_tracker; // Z-Score (Welford's): O(1) anomaly detection
    BillQueue* bill_queue;          // Queue: O(1) enqueue, O(n) search
    UndoStack* undo_stack;          // Stack: O(1) pop for undo
    
    // Additional structures for functionality
    Trie* category_trie;            // Trie: O(m) autocomplete
    Trie* payee_trie;               // Trie: O(m) autocomplete
    IntroSortStats sort_stats;      // IntroSort: O(n log n) tracking
    
    // Statistics
    DSAStats stats;                 // Operation statistics
    int transaction_counter;        // For ID generation
    int bill_counter;               // For bill ID generation
} FinanceEngine;

// Export functions for Python ctypes
#ifdef __cplusplus
extern "C" {
#endif

// Engine lifecycle
FinanceEngine* engine_create(void);
void engine_destroy(FinanceEngine* engine);

// Transaction operations
// Add: Uses Red-Black Tree O(log n) + Skip List O(log n) + Z-Score O(1)
const char* engine_add_transaction(FinanceEngine* engine, const char* type, 
                                   double amount, const char* category, 
                                   const char* description, const char* date);

// Delete: Uses Skip List O(log n) + Red-Black Tree O(log n)
bool engine_delete_transaction(FinanceEngine* engine, const char* id);

// Get by ID: Uses Skip List O(log n) expected
bool engine_find_transaction(FinanceEngine* engine, const char* id, Transaction* out);

// Get all: Uses Red-Black Tree traversal O(n)
int engine_get_all_transactions(FinanceEngine* engine, Transaction* out, int max_count);

// Get descending: Uses Red-Black Tree reverse traversal O(n)
int engine_get_transactions_desc(FinanceEngine* engine, Transaction* out, int max_count);

// Get in range: Uses Red-Black Tree range query O(log n + k)
int engine_get_transactions_in_range(FinanceEngine* engine, const char* start, 
                                     const char* end, Transaction* out, int max_count);

// Get recent: Uses Stack O(k) where k is count
int engine_get_recent_transactions(FinanceEngine* engine, int count, Transaction* out);

// Budget operations
// Set/Get: Uses Polynomial HashMap O(1) average
bool engine_set_budget(FinanceEngine* engine, const char* category, double limit);
bool engine_get_budget(FinanceEngine* engine, const char* category, Budget* out);
int engine_get_all_budgets(FinanceEngine* engine, Budget* out, int max_count);

// Get alerts sorted: Uses Indexed Priority Queue O(n log n)
int engine_get_budget_alerts(FinanceEngine* engine, Budget* out, int max_count);

// Bill operations
// Add: Uses Queue O(1) enqueue
const char* engine_add_bill(FinanceEngine* engine, const char* name, double amount,
                            const char* due_date, const char* category);
int engine_get_all_bills(FinanceEngine* engine, Bill* out, int max_count);
// Pay/Delete: Uses Queue O(n) search
bool engine_pay_bill(FinanceEngine* engine, const char* id);
bool engine_delete_bill(FinanceEngine* engine, const char* id);

// Analytics
// Top K expenses: Uses IntroSort O(n log n)
int engine_get_top_expenses(FinanceEngine* engine, int k, Transaction* out);
int engine_get_top_categories(FinanceEngine* engine, int k, CategoryAmount* out);

// Spending trends: Uses Sliding Window O(7) = O(1) for 7-day
bool engine_get_spending_trend_7day(FinanceEngine* engine, TrendResult* result);
bool engine_get_spending_trend_30day(FinanceEngine* engine, TrendResult* result);
bool engine_get_spending_trend_custom(FinanceEngine* engine, int days, TrendResult* result);

// Anomaly detection: Uses Z-Score (Welford's) O(1)
bool engine_check_transaction_anomaly(FinanceEngine* engine, double amount, 
                                      const char* category, AnomalyResult* result);
bool engine_get_spending_stats(FinanceEngine* engine, double* mean, double* std_dev,
                               double* min, double* max, int* count);

// Autocomplete: Uses Trie O(m + k)
int engine_get_category_suggestions(FinanceEngine* engine, const char* prefix, 
                                    char** out, int max_count);
int engine_get_all_categories(FinanceEngine* engine, char** out, int max_count);

// Undo: Uses Stack O(1) pop
bool engine_undo(FinanceEngine* engine);
bool engine_can_undo(FinanceEngine* engine);

// Statistics
double engine_get_total_balance(FinanceEngine* engine);
double engine_get_total_income(FinanceEngine* engine);
double engine_get_total_expenses(FinanceEngine* engine);
int engine_get_transaction_count(FinanceEngine* engine);
int engine_get_budget_count(FinanceEngine* engine);
int engine_get_bill_count(FinanceEngine* engine);

// DSA Stats for proof
void engine_get_dsa_stats(FinanceEngine* engine, DSAStats* stats);
void engine_reset_stats(FinanceEngine* engine);

// Data loading (for persistence integration)
void engine_load_transaction(FinanceEngine* engine, const char* id, const char* type,
                             double amount, const char* category, 
                             const char* description, const char* date);
void engine_load_budget(FinanceEngine* engine, const char* category, double limit);
void engine_load_bill(FinanceEngine* engine, const char* id, const char* name,
                      double amount, const char* due_date, const char* category, bool is_paid);

#ifdef __cplusplus
}
#endif

#endif // FINANCE_ENGINE_H
