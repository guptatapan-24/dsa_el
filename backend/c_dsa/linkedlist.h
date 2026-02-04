/**
 * Doubly Linked List Implementation for Transaction History
 * Data Structures & Applications Lab Project - Pure C Implementation
 * Operations: add front/back, delete, traverse - O(1) add, O(n) search
 */

#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include "common.h"

// Doubly linked list node
typedef struct DLLNode {
    Transaction data;
    struct DLLNode* prev;
    struct DLLNode* next;
} DLLNode;

// Doubly linked list structure
typedef struct {
    DLLNode* head;
    DLLNode* tail;
    int count;
    int operations_count;  // Track operations for proof
} DoublyLinkedList;

// Function declarations
DoublyLinkedList* dll_create(void);
void dll_destroy(DoublyLinkedList* list);
bool dll_add_front(DoublyLinkedList* list, const Transaction* t);
bool dll_add_back(DoublyLinkedList* list, const Transaction* t);
bool dll_delete_by_id(DoublyLinkedList* list, const char* id);
bool dll_delete_front(DoublyLinkedList* list, Transaction* out_t);
bool dll_find_by_id(DoublyLinkedList* list, const char* id, Transaction* out_t);
bool dll_get_front(DoublyLinkedList* list, Transaction* out_t);
int dll_traverse_forward(DoublyLinkedList* list, Transaction* out_transactions, int max_count);
int dll_traverse_backward(DoublyLinkedList* list, Transaction* out_transactions, int max_count);
int dll_filter_by_category(DoublyLinkedList* list, const char* category, Transaction* out_transactions, int max_count);
int dll_filter_by_type(DoublyLinkedList* list, const char* type, Transaction* out_transactions, int max_count);
int dll_size(DoublyLinkedList* list);
bool dll_is_empty(DoublyLinkedList* list);
void dll_clear(DoublyLinkedList* list);
int dll_get_operations_count(DoublyLinkedList* list);

#endif // LINKEDLIST_H
