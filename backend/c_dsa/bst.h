/**
 * Binary Search Tree Implementation for Date-wise Transaction Storage
 * Data Structures & Applications Lab Project - Pure C Implementation
 * Operations: insert O(log n), search O(log n), range query O(log n + k)
 */

#ifndef BST_H
#define BST_H

#include "common.h"

#define MAX_TRANSACTIONS_PER_DATE 100

// BST Node - stores transactions for a specific date
typedef struct BSTNode {
    char date[16];  // YYYY-MM-DD
    Transaction* transactions;
    int transaction_count;
    int capacity;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

// BST structure
typedef struct {
    BSTNode* root;
    int node_count;
    int total_transactions;
    int operations_count;  // Track operations for proof
} BST;

// Function declarations
BST* bst_create(void);
void bst_destroy(BST* tree);
bool bst_insert(BST* tree, const Transaction* t);
int bst_inorder_traversal(BST* tree, Transaction* out_transactions, int max_count);
int bst_reverse_inorder(BST* tree, Transaction* out_transactions, int max_count);
int bst_range_query(BST* tree, const char* start_date, const char* end_date, Transaction* out_transactions, int max_count);
bool bst_delete_by_id(BST* tree, const char* id);
bool bst_find_by_id(BST* tree, const char* id, Transaction* out_t);
int bst_get_by_month(BST* tree, const char* year_month, Transaction* out_transactions, int max_count);
int bst_size(BST* tree);
int bst_get_operations_count(BST* tree);
void bst_clear(BST* tree);

#endif // BST_H
