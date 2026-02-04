/**
 * Finance Engine - Main Integration Layer (Pure C)
 * Combines all 7 data structures for the Finance Tracker
 * Exposes functions that Python can call via ctypes
 * 
 * Data Structures Used:
 * 1. HashMap - Category to Budget mapping
 * 2. DoublyLinkedList - Transaction history
 * 3. BST - Date-sorted transactions
 * 4. MaxHeap - Top expenses
 * 5. Queue - Bill payment queue (FIFO)
 * 6. Stack - Undo operations
 * 7. Trie - Category autocomplete
 */

#ifndef FINANCE_ENGINE_H
#define FINANCE_ENGINE_H

#include "common.h"
#include "hashmap.h"
#include "linkedlist.h"
#include "bst.h"
#include "heap.h"
#include "queue.h"
#include "stack.h"
#include "trie.h"

#define MAX_TRANSACTIONS 10000
#define MAX_BUDGETS 100
#define MAX_BILLS 100
#define MAX_CATEGORIES 200

// DSA Usage Statistics for proof
typedef struct {
    int hashmap_ops;
    int linkedlist_ops;
    int bst_ops;
    int heap_ops;
    int queue_ops;
    int stack_ops;
    int trie_ops;
    int total_ops;
} DSAStats;

// Finance Engine structure
typedef struct {
    HashMap* budget_map;            // Category -> Budget (O(1) lookup)
    HashMap* expense_map;           // Category -> Total expense
    DoublyLinkedList* transactions; // Transaction history
    BST* transaction_bst;           // Date-sorted transactions
    MaxHeap* expense_heap;          // Top expenses
    CategoryMaxHeap* category_heap; // Top categories
    BillQueue* bill_queue;          // FIFO bill queue
    UndoStack* undo_stack;          // Undo operations
    TransactionStack* recent_stack; // Recent transactions
    Trie* category_trie;            // Category autocomplete
    Trie* payee_trie;               // Payee autocomplete
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
const char* engine_add_transaction(FinanceEngine* engine, const char* type, 
                                   double amount, const char* category, 
                                   const char* description, const char* date);
bool engine_delete_transaction(FinanceEngine* engine, const char* id);
int engine_get_all_transactions(FinanceEngine* engine, Transaction* out, int max_count);
int engine_get_transactions_desc(FinanceEngine* engine, Transaction* out, int max_count);
int engine_get_transactions_in_range(FinanceEngine* engine, const char* start, 
                                     const char* end, Transaction* out, int max_count);
int engine_get_recent_transactions(FinanceEngine* engine, int count, Transaction* out);
bool engine_find_transaction(FinanceEngine* engine, const char* id, Transaction* out);

// Budget operations
bool engine_set_budget(FinanceEngine* engine, const char* category, double limit);
bool engine_get_budget(FinanceEngine* engine, const char* category, Budget* out);
int engine_get_all_budgets(FinanceEngine* engine, Budget* out, int max_count);
int engine_get_budget_alerts(FinanceEngine* engine, Budget* out, int max_count);

// Bill operations
const char* engine_add_bill(FinanceEngine* engine, const char* name, double amount,
                            const char* due_date, const char* category);
int engine_get_all_bills(FinanceEngine* engine, Bill* out, int max_count);
bool engine_pay_bill(FinanceEngine* engine, const char* id);
bool engine_delete_bill(FinanceEngine* engine, const char* id);

// Analytics
int engine_get_top_expenses(FinanceEngine* engine, int k, Transaction* out);
int engine_get_top_categories(FinanceEngine* engine, int k, CategoryAmount* out);

// Autocomplete
int engine_get_category_suggestions(FinanceEngine* engine, const char* prefix, 
                                    char** out, int max_count);
int engine_get_all_categories(FinanceEngine* engine, char** out, int max_count);

// Undo
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
