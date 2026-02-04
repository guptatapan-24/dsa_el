/**
 * Red-Black Tree Implementation
 * Self-balancing BST with guaranteed O(log n) operations
 */

#include "rbtree.h"

// Create NIL sentinel node
static RBNode* create_nil_node(void) {
    RBNode* nil = (RBNode*)malloc(sizeof(RBNode));
    if (nil) {
        nil->color = RB_BLACK;
        nil->parent = nil;
        nil->left = nil;
        nil->right = nil;
        nil->transactions = NULL;
        nil->transaction_count = 0;
        nil->capacity = 0;
        nil->date[0] = '\0';
    }
    return nil;
}

// Create a new node
static RBNode* create_node(RBTree* tree, const char* date) {
    RBNode* node = (RBNode*)malloc(sizeof(RBNode));
    if (node) {
        safe_strcpy(node->date, date, sizeof(node->date));
        node->capacity = 10;
        node->transactions = (Transaction*)malloc(sizeof(Transaction) * node->capacity);
        node->transaction_count = 0;
        node->color = RB_RED;  // New nodes are always RED
        node->parent = tree->nil;
        node->left = tree->nil;
        node->right = tree->nil;
    }
    return node;
}

// Free a node
static void free_node(RBTree* tree, RBNode* node) {
    if (node && node != tree->nil) {
        if (node->transactions) free(node->transactions);
        free(node);
    }
}

// Left rotation
static void left_rotate(RBTree* tree, RBNode* x) {
    RBNode* y = x->right;
    
    // Turn y's left subtree into x's right subtree
    x->right = y->left;
    if (y->left != tree->nil) {
        y->left->parent = x;
    }
    
    // Link x's parent to y
    y->parent = x->parent;
    if (x->parent == tree->nil) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }
    
    // Put x on y's left
    y->left = x;
    x->parent = y;
    
    tree->rotations_count++;
}

// Right rotation
static void right_rotate(RBTree* tree, RBNode* y) {
    RBNode* x = y->left;
    
    // Turn x's right subtree into y's left subtree
    y->left = x->right;
    if (x->right != tree->nil) {
        x->right->parent = y;
    }
    
    // Link y's parent to x
    x->parent = y->parent;
    if (y->parent == tree->nil) {
        tree->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }
    
    // Put y on x's right
    x->right = y;
    y->parent = x;
    
    tree->rotations_count++;
}

// Fix up after insertion
static void insert_fixup(RBTree* tree, RBNode* z) {
    while (z->parent->color == RB_RED) {
        if (z->parent == z->parent->parent->left) {
            RBNode* y = z->parent->parent->right;  // Uncle
            if (y->color == RB_RED) {
                // Case 1: Uncle is RED
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    // Case 2: Uncle is BLACK, z is right child
                    z = z->parent;
                    left_rotate(tree, z);
                }
                // Case 3: Uncle is BLACK, z is left child
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                right_rotate(tree, z->parent->parent);
            }
        } else {
            // Symmetric cases (parent is right child)
            RBNode* y = z->parent->parent->left;  // Uncle
            if (y->color == RB_RED) {
                z->parent->color = RB_BLACK;
                y->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    right_rotate(tree, z);
                }
                z->parent->color = RB_BLACK;
                z->parent->parent->color = RB_RED;
                left_rotate(tree, z->parent->parent);
            }
        }
    }
    tree->root->color = RB_BLACK;
}

// Create new Red-Black Tree
RBTree* rbtree_create(void) {
    RBTree* tree = (RBTree*)malloc(sizeof(RBTree));
    if (tree) {
        tree->nil = create_nil_node();
        tree->root = tree->nil;
        tree->node_count = 0;
        tree->total_transactions = 0;
        tree->operations_count = 0;
        tree->rotations_count = 0;
    }
    return tree;
}

// Recursive destroy helper
static void destroy_helper(RBTree* tree, RBNode* node) {
    if (node == tree->nil) return;
    destroy_helper(tree, node->left);
    destroy_helper(tree, node->right);
    free_node(tree, node);
}

// Destroy Red-Black Tree
void rbtree_destroy(RBTree* tree) {
    if (!tree) return;
    destroy_helper(tree, tree->root);
    free(tree->nil);
    free(tree);
}

// Insert a transaction
// Time Complexity: O(log n) guaranteed
bool rbtree_insert(RBTree* tree, const Transaction* t) {
    if (!tree || !t) return false;
    
    tree->operations_count++;
    
    // Find insertion point
    RBNode* y = tree->nil;
    RBNode* x = tree->root;
    
    while (x != tree->nil) {
        y = x;
        int cmp = strcmp(t->date, x->date);
        if (cmp < 0) {
            x = x->left;
        } else if (cmp > 0) {
            x = x->right;
        } else {
            // Same date - add transaction to this node
            if (x->transaction_count >= x->capacity) {
                x->capacity *= 2;
                x->transactions = (Transaction*)realloc(x->transactions,
                                                        sizeof(Transaction) * x->capacity);
            }
            if (x->transactions) {
                memcpy(&x->transactions[x->transaction_count], t, sizeof(Transaction));
                x->transaction_count++;
                tree->total_transactions++;
            }
            return true;
        }
    }
    
    // Create new node
    RBNode* z = create_node(tree, t->date);
    if (!z) return false;
    
    memcpy(&z->transactions[0], t, sizeof(Transaction));
    z->transaction_count = 1;
    z->parent = y;
    
    if (y == tree->nil) {
        tree->root = z;
    } else if (strcmp(t->date, y->date) < 0) {
        y->left = z;
    } else {
        y->right = z;
    }
    
    tree->node_count++;
    tree->total_transactions++;
    
    // Fix up RB properties
    insert_fixup(tree, z);
    
    return true;
}

// Transplant helper for deletion
static void transplant(RBTree* tree, RBNode* u, RBNode* v) {
    if (u->parent == tree->nil) {
        tree->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

// Find minimum node
static RBNode* tree_minimum(RBTree* tree, RBNode* x) {
    while (x->left != tree->nil) {
        x = x->left;
    }
    return x;
}

// Delete fixup
static void delete_fixup(RBTree* tree, RBNode* x) {
    while (x != tree->root && x->color == RB_BLACK) {
        if (x == x->parent->left) {
            RBNode* w = x->parent->right;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                left_rotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == RB_BLACK && w->right->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->right->color == RB_BLACK) {
                    w->left->color = RB_BLACK;
                    w->color = RB_RED;
                    right_rotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->right->color = RB_BLACK;
                left_rotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            RBNode* w = x->parent->left;
            if (w->color == RB_RED) {
                w->color = RB_BLACK;
                x->parent->color = RB_RED;
                right_rotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == RB_BLACK && w->left->color == RB_BLACK) {
                w->color = RB_RED;
                x = x->parent;
            } else {
                if (w->left->color == RB_BLACK) {
                    w->right->color = RB_BLACK;
                    w->color = RB_RED;
                    left_rotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = RB_BLACK;
                w->left->color = RB_BLACK;
                right_rotate(tree, x->parent);
                x = tree->root;
            }
        }
    }
    x->color = RB_BLACK;
}

// Find by ID helper - searches all nodes
static bool find_by_id_helper(RBTree* tree, RBNode* node, const char* id, Transaction* out_t) {
    if (node == tree->nil) return false;
    
    for (int i = 0; i < node->transaction_count; i++) {
        if (strcmp(node->transactions[i].id, id) == 0) {
            if (out_t) {
                memcpy(out_t, &node->transactions[i], sizeof(Transaction));
            }
            return true;
        }
    }
    
    if (find_by_id_helper(tree, node->left, id, out_t)) return true;
    return find_by_id_helper(tree, node->right, id, out_t);
}

// Find transaction by ID
bool rbtree_find_by_id(RBTree* tree, const char* id, Transaction* out_t) {
    if (!tree || !id) return false;
    tree->operations_count++;
    return find_by_id_helper(tree, tree->root, id, out_t);
}

// Delete by ID helper
static bool delete_by_id_helper(RBTree* tree, RBNode* node, const char* id) {
    if (node == tree->nil) return false;
    
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
    if (delete_by_id_helper(tree, node->left, id)) return true;
    return delete_by_id_helper(tree, node->right, id);
}

// Delete transaction by ID
bool rbtree_delete_by_id(RBTree* tree, const char* id) {
    if (!tree || !id) return false;
    tree->operations_count++;
    return delete_by_id_helper(tree, tree->root, id);
}

// Inorder traversal helper
static int inorder_helper(RBTree* tree, RBNode* node, Transaction* out, int* index, int max_count) {
    if (node == tree->nil || *index >= max_count) return *index;
    
    inorder_helper(tree, node->left, out, index, max_count);
    
    for (int i = 0; i < node->transaction_count && *index < max_count; i++) {
        memcpy(&out[*index], &node->transactions[i], sizeof(Transaction));
        (*index)++;
    }
    
    inorder_helper(tree, node->right, out, index, max_count);
    return *index;
}

// Inorder traversal (ascending by date)
int rbtree_inorder_traversal(RBTree* tree, Transaction* out_transactions, int max_count) {
    if (!tree || !out_transactions) return 0;
    tree->operations_count++;
    int index = 0;
    inorder_helper(tree, tree->root, out_transactions, &index, max_count);
    return index;
}

// Reverse inorder helper
static int reverse_inorder_helper(RBTree* tree, RBNode* node, Transaction* out, int* index, int max_count) {
    if (node == tree->nil || *index >= max_count) return *index;
    
    reverse_inorder_helper(tree, node->right, out, index, max_count);
    
    for (int i = node->transaction_count - 1; i >= 0 && *index < max_count; i--) {
        memcpy(&out[*index], &node->transactions[i], sizeof(Transaction));
        (*index)++;
    }
    
    reverse_inorder_helper(tree, node->left, out, index, max_count);
    return *index;
}

// Reverse inorder (descending by date)
int rbtree_reverse_inorder(RBTree* tree, Transaction* out_transactions, int max_count) {
    if (!tree || !out_transactions) return 0;
    tree->operations_count++;
    int index = 0;
    reverse_inorder_helper(tree, tree->root, out_transactions, &index, max_count);
    return index;
}

// Range query helper
static void range_query_helper(RBTree* tree, RBNode* node, const char* start, const char* end,
                               Transaction* out, int* index, int max_count) {
    if (node == tree->nil || *index >= max_count) return;
    
    if (strcmp(node->date, start) > 0) {
        range_query_helper(tree, node->left, start, end, out, index, max_count);
    }
    
    if (strcmp(node->date, start) >= 0 && strcmp(node->date, end) <= 0) {
        for (int i = 0; i < node->transaction_count && *index < max_count; i++) {
            memcpy(&out[*index], &node->transactions[i], sizeof(Transaction));
            (*index)++;
        }
    }
    
    if (strcmp(node->date, end) < 0) {
        range_query_helper(tree, node->right, start, end, out, index, max_count);
    }
}

// Range query: O(log n + k)
int rbtree_range_query(RBTree* tree, const char* start_date, const char* end_date,
                       Transaction* out_transactions, int max_count) {
    if (!tree || !start_date || !end_date || !out_transactions) return 0;
    tree->operations_count++;
    int index = 0;
    range_query_helper(tree, tree->root, start_date, end_date, out_transactions, &index, max_count);
    return index;
}

// Get by month
int rbtree_get_by_month(RBTree* tree, const char* year_month,
                        Transaction* out_transactions, int max_count) {
    if (!tree || !year_month || !out_transactions) return 0;
    
    char start_date[16], end_date[16];
    snprintf(start_date, sizeof(start_date), "%s-01", year_month);
    snprintf(end_date, sizeof(end_date), "%s-31", year_month);
    
    return rbtree_range_query(tree, start_date, end_date, out_transactions, max_count);
}

int rbtree_size(RBTree* tree) {
    return tree ? tree->total_transactions : 0;
}

int rbtree_get_operations_count(RBTree* tree) {
    return tree ? tree->operations_count : 0;
}

int rbtree_get_rotations_count(RBTree* tree) {
    return tree ? tree->rotations_count : 0;
}

// Height helper
static int height_helper(RBTree* tree, RBNode* node) {
    if (node == tree->nil) return 0;
    int left_h = height_helper(tree, node->left);
    int right_h = height_helper(tree, node->right);
    return 1 + (left_h > right_h ? left_h : right_h);
}

int rbtree_height(RBTree* tree) {
    return tree ? height_helper(tree, tree->root) : 0;
}

// Black height helper
static int black_height_helper(RBTree* tree, RBNode* node) {
    if (node == tree->nil) return 1;
    int left_bh = black_height_helper(tree, node->left);
    return left_bh + (node->color == RB_BLACK ? 1 : 0);
}

int rbtree_black_height(RBTree* tree) {
    return tree ? black_height_helper(tree, tree->root) : 0;
}

// Clear helper
static void clear_helper(RBTree* tree, RBNode* node) {
    if (node == tree->nil) return;
    clear_helper(tree, node->left);
    clear_helper(tree, node->right);
    free_node(tree, node);
}

void rbtree_clear(RBTree* tree) {
    if (!tree) return;
    clear_helper(tree, tree->root);
    tree->root = tree->nil;
    tree->node_count = 0;
    tree->total_transactions = 0;
}

// Validate RB properties
static bool validate_helper(RBTree* tree, RBNode* node, int black_count, int* path_black) {
    if (node == tree->nil) {
        if (*path_black == -1) {
            *path_black = black_count + 1;
        }
        return *path_black == black_count + 1;
    }
    
    // Property 4: Red node has black children
    if (node->color == RB_RED) {
        if (node->left->color == RB_RED || node->right->color == RB_RED) {
            return false;
        }
    }
    
    if (node->color == RB_BLACK) {
        black_count++;
    }
    
    return validate_helper(tree, node->left, black_count, path_black) &&
           validate_helper(tree, node->right, black_count, path_black);
}

bool rbtree_validate(RBTree* tree) {
    if (!tree) return false;
    
    // Property 2: Root is black
    if (tree->root != tree->nil && tree->root->color != RB_BLACK) {
        return false;
    }
    
    int path_black = -1;
    return validate_helper(tree, tree->root, 0, &path_black);
}
