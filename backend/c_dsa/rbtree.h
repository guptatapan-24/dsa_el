/**
 * Red-Black Tree Implementation
 * Self-balancing BST with guaranteed O(log n) operations
 * Data Structures & Applications Lab Project - Pure C Implementation
 * 
 * Properties:
 * 1. Every node is either RED or BLACK
 * 2. Root is always BLACK
 * 3. Every leaf (NIL) is BLACK
 * 4. If a node is RED, both children are BLACK
 * 5. All paths from root to leaves have same number of BLACK nodes
 */

#ifndef RBTREE_H
#define RBTREE_H

#include "common.h"

// Node colors
typedef enum {
    RB_RED = 0,
    RB_BLACK = 1
} RBColor;

// Red-Black Tree Node
typedef struct RBNode {
    char date[16];               // Key: YYYY-MM-DD
    Transaction* transactions;   // Array of transactions for this date
    int transaction_count;
    int capacity;
    RBColor color;
    struct RBNode* parent;
    struct RBNode* left;
    struct RBNode* right;
} RBNode;

// Red-Black Tree structure
typedef struct {
    RBNode* root;
    RBNode* nil;           // Sentinel NIL node
    int node_count;
    int total_transactions;
    int operations_count;
    int rotations_count;   // Track rotations for proof
} RBTree;

// Function declarations
RBTree* rbtree_create(void);
void rbtree_destroy(RBTree* tree);

// Core operations - O(log n) guaranteed
bool rbtree_insert(RBTree* tree, const Transaction* t);
bool rbtree_delete_by_id(RBTree* tree, const char* id);
bool rbtree_find_by_id(RBTree* tree, const char* id, Transaction* out_t);

// Traversal operations
int rbtree_inorder_traversal(RBTree* tree, Transaction* out_transactions, int max_count);
int rbtree_reverse_inorder(RBTree* tree, Transaction* out_transactions, int max_count);
int rbtree_range_query(RBTree* tree, const char* start_date, const char* end_date, 
                       Transaction* out_transactions, int max_count);
int rbtree_get_by_month(RBTree* tree, const char* year_month, 
                        Transaction* out_transactions, int max_count);

// Statistics
int rbtree_size(RBTree* tree);
int rbtree_get_operations_count(RBTree* tree);
int rbtree_get_rotations_count(RBTree* tree);
int rbtree_height(RBTree* tree);
int rbtree_black_height(RBTree* tree);
void rbtree_clear(RBTree* tree);

// Validation (for testing)
bool rbtree_validate(RBTree* tree);

#endif // RBTREE_H
