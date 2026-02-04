/**
 * Stack Implementation for Undo Operations
 * Data Structures & Applications Lab Project - Pure C Implementation
 * Operations: push O(1), pop O(1), peek O(1)
 */

#ifndef STACK_H
#define STACK_H

#include "common.h"

#define MAX_STACK_SIZE 50

// Stack node for undo actions
typedef struct StackNode {
    UndoAction data;
    struct StackNode* next;
} StackNode;

// Undo Stack structure
typedef struct {
    StackNode* top;
    int count;
    int max_size;
    int operations_count;
} UndoStack;

// Transaction Stack node
typedef struct TStackNode {
    Transaction data;
    struct TStackNode* next;
} TStackNode;

// Transaction Stack (for recent transactions)
typedef struct {
    TStackNode* top;
    int count;
    int max_size;
    int operations_count;
} TransactionStack;

// Undo Stack functions
UndoStack* undo_stack_create(int max_size);
void undo_stack_destroy(UndoStack* stack);
bool undo_stack_push(UndoStack* stack, const UndoAction* action);
bool undo_stack_pop(UndoStack* stack, UndoAction* out_action);
bool undo_stack_peek(UndoStack* stack, UndoAction* out_action);
int undo_stack_get_all(UndoStack* stack, UndoAction* out_actions, int max_count);
int undo_stack_size(UndoStack* stack);
bool undo_stack_is_empty(UndoStack* stack);
void undo_stack_clear(UndoStack* stack);
int undo_stack_get_operations_count(UndoStack* stack);

// Transaction Stack functions
TransactionStack* trans_stack_create(int max_size);
void trans_stack_destroy(TransactionStack* stack);
bool trans_stack_push(TransactionStack* stack, const Transaction* t);
bool trans_stack_pop(TransactionStack* stack, Transaction* out_t);
bool trans_stack_peek(TransactionStack* stack, Transaction* out_t);
int trans_stack_get_all(TransactionStack* stack, Transaction* out_transactions, int max_count);
int trans_stack_get_top_n(TransactionStack* stack, int n, Transaction* out_transactions);
int trans_stack_size(TransactionStack* stack);
bool trans_stack_is_empty(TransactionStack* stack);
void trans_stack_clear(TransactionStack* stack);
int trans_stack_get_operations_count(TransactionStack* stack);

#endif // STACK_H
