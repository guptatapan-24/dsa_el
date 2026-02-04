/**
 * Doubly Linked List Implementation - Pure C
 * Maintains transaction history with O(1) insertions at both ends
 */

#include "linkedlist.h"

// Create new doubly linked list
DoublyLinkedList* dll_create(void) {
    DoublyLinkedList* list = (DoublyLinkedList*)malloc(sizeof(DoublyLinkedList));
    if (list) {
        list->head = NULL;
        list->tail = NULL;
        list->count = 0;
        list->operations_count = 0;
    }
    return list;
}

// Destroy list and free all nodes
void dll_destroy(DoublyLinkedList* list) {
    if (!list) return;
    
    DLLNode* current = list->head;
    while (current) {
        DLLNode* temp = current;
        current = current->next;
        free(temp);
    }
    free(list);
}

// Add transaction to front (most recent)
// Time Complexity: O(1)
bool dll_add_front(DoublyLinkedList* list, const Transaction* t) {
    if (!list || !t) return false;
    
    list->operations_count++;
    DLLNode* new_node = (DLLNode*)malloc(sizeof(DLLNode));
    if (!new_node) return false;
    
    memcpy(&new_node->data, t, sizeof(Transaction));
    new_node->prev = NULL;
    new_node->next = list->head;
    
    if (list->head) {
        list->head->prev = new_node;
    } else {
        list->tail = new_node;
    }
    list->head = new_node;
    list->count++;
    
    return true;
}

// Add transaction to back
// Time Complexity: O(1)
bool dll_add_back(DoublyLinkedList* list, const Transaction* t) {
    if (!list || !t) return false;
    
    list->operations_count++;
    DLLNode* new_node = (DLLNode*)malloc(sizeof(DLLNode));
    if (!new_node) return false;
    
    memcpy(&new_node->data, t, sizeof(Transaction));
    new_node->next = NULL;
    new_node->prev = list->tail;
    
    if (list->tail) {
        list->tail->next = new_node;
    } else {
        list->head = new_node;
    }
    list->tail = new_node;
    list->count++;
    
    return true;
}

// Delete transaction by ID
// Time Complexity: O(n)
bool dll_delete_by_id(DoublyLinkedList* list, const char* id) {
    if (!list || !id) return false;
    
    list->operations_count++;
    DLLNode* current = list->head;
    
    while (current) {
        if (strcmp(current->data.id, id) == 0) {
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                list->head = current->next;
            }
            
            if (current->next) {
                current->next->prev = current->prev;
            } else {
                list->tail = current->prev;
            }
            
            free(current);
            list->count--;
            return true;
        }
        current = current->next;
    }
    return false;
}

// Delete and return front transaction (for undo)
// Time Complexity: O(1)
bool dll_delete_front(DoublyLinkedList* list, Transaction* out_t) {
    if (!list || !list->head) return false;
    
    list->operations_count++;
    if (out_t) {
        memcpy(out_t, &list->head->data, sizeof(Transaction));
    }
    
    DLLNode* temp = list->head;
    list->head = list->head->next;
    
    if (list->head) {
        list->head->prev = NULL;
    } else {
        list->tail = NULL;
    }
    
    free(temp);
    list->count--;
    return true;
}

// Find transaction by ID
// Time Complexity: O(n)
bool dll_find_by_id(DoublyLinkedList* list, const char* id, Transaction* out_t) {
    if (!list || !id) return false;
    
    list->operations_count++;
    DLLNode* current = list->head;
    
    while (current) {
        if (strcmp(current->data.id, id) == 0) {
            if (out_t) {
                memcpy(out_t, &current->data, sizeof(Transaction));
            }
            return true;
        }
        current = current->next;
    }
    return false;
}

// Get front transaction without removing
bool dll_get_front(DoublyLinkedList* list, Transaction* out_t) {
    if (!list || !list->head) return false;
    if (out_t) {
        memcpy(out_t, &list->head->data, sizeof(Transaction));
    }
    return true;
}

// Traverse forward and collect transactions
// Time Complexity: O(n)
int dll_traverse_forward(DoublyLinkedList* list, Transaction* out_transactions, int max_count) {
    if (!list || !out_transactions) return 0;
    
    list->operations_count++;
    int count = 0;
    DLLNode* current = list->head;
    
    while (current && count < max_count) {
        memcpy(&out_transactions[count], &current->data, sizeof(Transaction));
        count++;
        current = current->next;
    }
    return count;
}

// Traverse backward
// Time Complexity: O(n)
int dll_traverse_backward(DoublyLinkedList* list, Transaction* out_transactions, int max_count) {
    if (!list || !out_transactions) return 0;
    
    list->operations_count++;
    int count = 0;
    DLLNode* current = list->tail;
    
    while (current && count < max_count) {
        memcpy(&out_transactions[count], &current->data, sizeof(Transaction));
        count++;
        current = current->prev;
    }
    return count;
}

// Filter by category
// Time Complexity: O(n)
int dll_filter_by_category(DoublyLinkedList* list, const char* category, Transaction* out_transactions, int max_count) {
    if (!list || !category || !out_transactions) return 0;
    
    list->operations_count++;
    int count = 0;
    DLLNode* current = list->head;
    
    while (current && count < max_count) {
        if (strcmp(current->data.category, category) == 0) {
            memcpy(&out_transactions[count], &current->data, sizeof(Transaction));
            count++;
        }
        current = current->next;
    }
    return count;
}

// Filter by type
// Time Complexity: O(n)
int dll_filter_by_type(DoublyLinkedList* list, const char* type, Transaction* out_transactions, int max_count) {
    if (!list || !type || !out_transactions) return 0;
    
    list->operations_count++;
    int count = 0;
    DLLNode* current = list->head;
    
    while (current && count < max_count) {
        if (strcmp(current->data.type, type) == 0) {
            memcpy(&out_transactions[count], &current->data, sizeof(Transaction));
            count++;
        }
        current = current->next;
    }
    return count;
}

int dll_size(DoublyLinkedList* list) {
    return list ? list->count : 0;
}

bool dll_is_empty(DoublyLinkedList* list) {
    return list ? list->count == 0 : true;
}

// Clear all nodes
void dll_clear(DoublyLinkedList* list) {
    if (!list) return;
    
    DLLNode* current = list->head;
    while (current) {
        DLLNode* temp = current;
        current = current->next;
        free(temp);
    }
    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
}

int dll_get_operations_count(DoublyLinkedList* list) {
    return list ? list->operations_count : 0;
}
