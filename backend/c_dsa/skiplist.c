/**
 * Skip List Implementation
 * Probabilistic data structure with expected O(log n) operations
 */

#include "skiplist.h"
#include <stdlib.h>
#include <string.h>

// Random level generator using probabilistic method
static int random_level(void) {
    int level = 1;
    while (((double)rand() / RAND_MAX) < SKIPLIST_P && level < SKIPLIST_MAX_LEVEL) {
        level++;
    }
    return level;
}

// Create a new skip node
static SkipNode* create_skip_node(int level, const char* id, const Transaction* t) {
    SkipNode* node = (SkipNode*)malloc(sizeof(SkipNode));
    if (node) {
        node->forward = (SkipNode**)malloc(sizeof(SkipNode*) * (level + 1));
        if (!node->forward) {
            free(node);
            return NULL;
        }
        for (int i = 0; i <= level; i++) {
            node->forward[i] = NULL;
        }
        node->level = level;
        if (id) {
            safe_strcpy(node->id, id, sizeof(node->id));
        }
        if (t) {
            memcpy(&node->transaction, t, sizeof(Transaction));
        }
    }
    return node;
}

// Free a skip node
static void free_skip_node(SkipNode* node) {
    if (node) {
        if (node->forward) free(node->forward);
        free(node);
    }
}

// Create new Skip List
SkipList* skiplist_create(void) {
    SkipList* list = (SkipList*)malloc(sizeof(SkipList));
    if (list) {
        list->header = create_skip_node(SKIPLIST_MAX_LEVEL, "", NULL);
        list->level = 0;
        list->size = 0;
        list->operations_count = 0;
        memset(list->level_distribution, 0, sizeof(list->level_distribution));
        
        // Seed random number generator
        srand((unsigned int)time(NULL));
    }
    return list;
}

// Destroy Skip List
void skiplist_destroy(SkipList* list) {
    if (!list) return;
    
    SkipNode* current = list->header->forward[0];
    while (current) {
        SkipNode* next = current->forward[0];
        free_skip_node(current);
        current = next;
    }
    free_skip_node(list->header);
    free(list);
}

// Insert a transaction
// Time Complexity: O(log n) expected
bool skiplist_insert(SkipList* list, const Transaction* t) {
    if (!list || !t) return false;
    
    list->operations_count++;
    
    SkipNode* update[SKIPLIST_MAX_LEVEL + 1];
    SkipNode* current = list->header;
    
    // Find insertion position at each level
    for (int i = list->level; i >= 0; i--) {
        while (current->forward[i] && strcmp(current->forward[i]->id, t->id) < 0) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    
    current = current->forward[0];
    
    // Check if ID already exists
    if (current && strcmp(current->id, t->id) == 0) {
        // Update existing transaction
        memcpy(&current->transaction, t, sizeof(Transaction));
        return true;
    }
    
    // Generate random level for new node
    int new_level = random_level();
    
    // Track level distribution
    if (new_level > 0 && new_level <= SKIPLIST_MAX_LEVEL) {
        list->level_distribution[new_level - 1]++;
    }
    
    // Update list level if necessary
    if (new_level > list->level) {
        for (int i = list->level + 1; i <= new_level; i++) {
            update[i] = list->header;
        }
        list->level = new_level;
    }
    
    // Create new node
    SkipNode* new_node = create_skip_node(new_level, t->id, t);
    if (!new_node) return false;
    
    // Insert node at each level
    for (int i = 0; i <= new_level; i++) {
        new_node->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = new_node;
    }
    
    list->size++;
    return true;
}

// Search for a transaction by ID
// Time Complexity: O(log n) expected
bool skiplist_search(SkipList* list, const char* id, Transaction* out_t) {
    if (!list || !id) return false;
    
    list->operations_count++;
    
    SkipNode* current = list->header;
    
    // Start from highest level and work down
    for (int i = list->level; i >= 0; i--) {
        while (current->forward[i] && strcmp(current->forward[i]->id, id) < 0) {
            current = current->forward[i];
        }
    }
    
    current = current->forward[0];
    
    if (current && strcmp(current->id, id) == 0) {
        if (out_t) {
            memcpy(out_t, &current->transaction, sizeof(Transaction));
        }
        return true;
    }
    
    return false;
}

// Delete a transaction by ID
// Time Complexity: O(log n) expected
bool skiplist_delete(SkipList* list, const char* id) {
    if (!list || !id) return false;
    
    list->operations_count++;
    
    SkipNode* update[SKIPLIST_MAX_LEVEL + 1];
    SkipNode* current = list->header;
    
    // Find node at each level
    for (int i = list->level; i >= 0; i--) {
        while (current->forward[i] && strcmp(current->forward[i]->id, id) < 0) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    
    current = current->forward[0];
    
    if (!current || strcmp(current->id, id) != 0) {
        return false;  // Not found
    }
    
    // Remove node from each level
    for (int i = 0; i <= list->level; i++) {
        if (update[i]->forward[i] != current) break;
        update[i]->forward[i] = current->forward[i];
    }
    
    free_skip_node(current);
    
    // Reduce list level if necessary
    while (list->level > 0 && list->header->forward[list->level] == NULL) {
        list->level--;
    }
    
    list->size--;
    return true;
}

// Get all transactions
int skiplist_get_all(SkipList* list, Transaction* out_transactions, int max_count) {
    if (!list || !out_transactions) return 0;
    
    list->operations_count++;
    
    SkipNode* current = list->header->forward[0];
    int count = 0;
    
    while (current && count < max_count) {
        memcpy(&out_transactions[count], &current->transaction, sizeof(Transaction));
        count++;
        current = current->forward[0];
    }
    
    return count;
}

int skiplist_size(SkipList* list) {
    return list ? list->size : 0;
}

int skiplist_get_operations_count(SkipList* list) {
    return list ? list->operations_count : 0;
}

int skiplist_get_level(SkipList* list) {
    return list ? list->level : 0;
}

void skiplist_clear(SkipList* list) {
    if (!list) return;
    
    SkipNode* current = list->header->forward[0];
    while (current) {
        SkipNode* next = current->forward[0];
        free_skip_node(current);
        current = next;
    }
    
    for (int i = 0; i <= list->level; i++) {
        list->header->forward[i] = NULL;
    }
    
    list->level = 0;
    list->size = 0;
    memset(list->level_distribution, 0, sizeof(list->level_distribution));
}

void skiplist_get_level_distribution(SkipList* list, int* distribution, int max_levels) {
    if (!list || !distribution) return;
    
    int count = max_levels < SKIPLIST_MAX_LEVEL ? max_levels : SKIPLIST_MAX_LEVEL;
    for (int i = 0; i < count; i++) {
        distribution[i] = list->level_distribution[i];
    }
}
