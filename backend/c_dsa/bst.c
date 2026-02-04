/**
 * Binary Search Tree Implementation - Pure C
 * Stores transactions indexed by date for efficient range queries
 */

#include "bst.h"

// Create a new BST node
static BSTNode* create_node(const char* date) {
    BSTNode* node = (BSTNode*)malloc(sizeof(BSTNode));
    if (node) {
        safe_strcpy(node->date, date, sizeof(node->date));
        node->capacity = 10;
        node->transactions = (Transaction*)malloc(sizeof(Transaction) * node->capacity);
        node->transaction_count = 0;
        node->left = NULL;
        node->right = NULL;
    }
    return node;
}

// Free a BST node
static void free_node(BSTNode* node) {
    if (node) {
        if (node->transactions) free(node->transactions);
        free(node);
    }
}

// Recursive destroy helper
static void destroy_helper(BSTNode* node) {
    if (!node) return;
    destroy_helper(node->left);
    destroy_helper(node->right);
    free_node(node);
}

// Create new BST
BST* bst_create(void) {
    BST* tree = (BST*)malloc(sizeof(BST));
    if (tree) {
        tree->root = NULL;
        tree->node_count = 0;
        tree->total_transactions = 0;
        tree->operations_count = 0;
    }
    return tree;
}

// Destroy BST
void bst_destroy(BST* tree) {
    if (!tree) return;
    destroy_helper(tree->root);
    free(tree);
}

// Insert helper - recursive
static BSTNode* insert_helper(BSTNode* node, const Transaction* t, BST* tree) {
    if (!node) {
        BSTNode* new_node = create_node(t->date);
        if (new_node) {
            memcpy(&new_node->transactions[0], t, sizeof(Transaction));
            new_node->transaction_count = 1;
            tree->node_count++;
        }
        return new_node;
    }
    
    int cmp = strcmp(t->date, node->date);
    if (cmp < 0) {
        node->left = insert_helper(node->left, t, tree);
    } else if (cmp > 0) {
        node->right = insert_helper(node->right, t, tree);
    } else {
        // Same date - add transaction to this node
        if (node->transaction_count >= node->capacity) {
            node->capacity *= 2;
            node->transactions = (Transaction*)realloc(node->transactions, 
                                                       sizeof(Transaction) * node->capacity);
        }
        if (node->transactions) {
            memcpy(&node->transactions[node->transaction_count], t, sizeof(Transaction));
            node->transaction_count++;
        }
    }
    return node;
}

// Insert transaction
// Time Complexity: O(log n) average, O(n) worst case
bool bst_insert(BST* tree, const Transaction* t) {
    if (!tree || !t) return false;
    
    tree->operations_count++;
    tree->root = insert_helper(tree->root, t, tree);
    tree->total_transactions++;
    return true;
}

// Inorder traversal helper
static int inorder_helper(BSTNode* node, Transaction* out, int* index, int max_count) {
    if (!node || *index >= max_count) return *index;
    
    inorder_helper(node->left, out, index, max_count);
    
    for (int i = 0; i < node->transaction_count && *index < max_count; i++) {
        memcpy(&out[*index], &node->transactions[i], sizeof(Transaction));
        (*index)++;
    }
    
    inorder_helper(node->right, out, index, max_count);
    return *index;
}

// Inorder traversal (ascending by date)
// Time Complexity: O(n)
int bst_inorder_traversal(BST* tree, Transaction* out_transactions, int max_count) {
    if (!tree || !out_transactions) return 0;
    
    tree->operations_count++;
    int index = 0;
    inorder_helper(tree->root, out_transactions, &index, max_count);
    return index;
}

// Reverse inorder helper
static int reverse_inorder_helper(BSTNode* node, Transaction* out, int* index, int max_count) {
    if (!node || *index >= max_count) return *index;
    
    reverse_inorder_helper(node->right, out, index, max_count);
    
    for (int i = node->transaction_count - 1; i >= 0 && *index < max_count; i--) {
        memcpy(&out[*index], &node->transactions[i], sizeof(Transaction));
        (*index)++;
    }
    
    reverse_inorder_helper(node->left, out, index, max_count);
    return *index;
}

// Reverse inorder (descending by date)
// Time Complexity: O(n)
int bst_reverse_inorder(BST* tree, Transaction* out_transactions, int max_count) {
    if (!tree || !out_transactions) return 0;
    
    tree->operations_count++;
    int index = 0;
    reverse_inorder_helper(tree->root, out_transactions, &index, max_count);
    return index;
}

// Range query helper
static void range_query_helper(BSTNode* node, const char* start, const char* end, 
                               Transaction* out, int* index, int max_count) {
    if (!node || *index >= max_count) return;
    
    // If node's date > start, search left subtree
    if (strcmp(node->date, start) > 0) {
        range_query_helper(node->left, start, end, out, index, max_count);
    }
    
    // If node's date is in range, add its transactions
    if (strcmp(node->date, start) >= 0 && strcmp(node->date, end) <= 0) {
        for (int i = 0; i < node->transaction_count && *index < max_count; i++) {
            memcpy(&out[*index], &node->transactions[i], sizeof(Transaction));
            (*index)++;
        }
    }
    
    // If node's date < end, search right subtree
    if (strcmp(node->date, end) < 0) {
        range_query_helper(node->right, start, end, out, index, max_count);
    }
}

// Range query: Get transactions between two dates
// Time Complexity: O(log n + k) where k is number of results
int bst_range_query(BST* tree, const char* start_date, const char* end_date, 
                    Transaction* out_transactions, int max_count) {
    if (!tree || !start_date || !end_date || !out_transactions) return 0;
    
    tree->operations_count++;
    int index = 0;
    range_query_helper(tree->root, start_date, end_date, out_transactions, &index, max_count);
    return index;
}

// Delete by ID helper - searches all nodes
static bool delete_by_id_helper(BSTNode* node, const char* id, BST* tree) {
    if (!node) return false;
    
    // Search in current node
    for (int i = 0; i < node->transaction_count; i++) {
        if (strcmp(node->transactions[i].id, id) == 0) {
            // Shift remaining transactions
            for (int j = i; j < node->transaction_count - 1; j++) {
                memcpy(&node->transactions[j], &node->transactions[j+1], sizeof(Transaction));
            }
            node->transaction_count--;
            tree->total_transactions--;
            return true;
        }
    }
    
    // Search in subtrees
    if (delete_by_id_helper(node->left, id, tree)) return true;
    return delete_by_id_helper(node->right, id, tree);
}

// Delete transaction by ID
// Time Complexity: O(n)
bool bst_delete_by_id(BST* tree, const char* id) {
    if (!tree || !id) return false;
    
    tree->operations_count++;
    return delete_by_id_helper(tree->root, id, tree);
}

// Find by ID helper
static bool find_by_id_helper(BSTNode* node, const char* id, Transaction* out_t) {
    if (!node) return false;
    
    for (int i = 0; i < node->transaction_count; i++) {
        if (strcmp(node->transactions[i].id, id) == 0) {
            if (out_t) {
                memcpy(out_t, &node->transactions[i], sizeof(Transaction));
            }
            return true;
        }
    }
    
    if (find_by_id_helper(node->left, id, out_t)) return true;
    return find_by_id_helper(node->right, id, out_t);
}

// Find transaction by ID
bool bst_find_by_id(BST* tree, const char* id, Transaction* out_t) {
    if (!tree || !id) return false;
    
    tree->operations_count++;
    return find_by_id_helper(tree->root, id, out_t);
}

// Get transactions by month (YYYY-MM)
int bst_get_by_month(BST* tree, const char* year_month, Transaction* out_transactions, int max_count) {
    if (!tree || !year_month || !out_transactions) return 0;
    
    char start_date[16], end_date[16];
    snprintf(start_date, sizeof(start_date), "%s-01", year_month);
    snprintf(end_date, sizeof(end_date), "%s-31", year_month);
    
    return bst_range_query(tree, start_date, end_date, out_transactions, max_count);
}

int bst_size(BST* tree) {
    return tree ? tree->total_transactions : 0;
}

int bst_get_operations_count(BST* tree) {
    return tree ? tree->operations_count : 0;
}

// Clear helper
static void clear_helper(BSTNode* node) {
    if (!node) return;
    clear_helper(node->left);
    clear_helper(node->right);
    free_node(node);
}

void bst_clear(BST* tree) {
    if (!tree) return;
    clear_helper(tree->root);
    tree->root = NULL;
    tree->node_count = 0;
    tree->total_transactions = 0;
}
