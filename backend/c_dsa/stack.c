/**
 * Stack Implementation - Pure C
 * LIFO stack for undo operations and recent transactions
 */

#include "stack.h"

// ==================== UNDO STACK ====================

// Create undo stack
UndoStack* undo_stack_create(int max_size) {
    UndoStack* stack = (UndoStack*)malloc(sizeof(UndoStack));
    if (stack) {
        stack->top = NULL;
        stack->count = 0;
        stack->max_size = max_size > 0 ? max_size : MAX_STACK_SIZE;
        stack->operations_count = 0;
    }
    return stack;
}

// Destroy undo stack
void undo_stack_destroy(UndoStack* stack) {
    if (!stack) return;
    
    while (stack->top) {
        StackNode* temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
    free(stack);
}

// Push action onto stack
// Time Complexity: O(1)
bool undo_stack_push(UndoStack* stack, const UndoAction* action) {
    if (!stack || !action) return false;
    
    stack->operations_count++;
    
    // If at max size, remove bottom element
    if (stack->count >= stack->max_size) {
        // Find second to last
        StackNode* current = stack->top;
        while (current->next && current->next->next) {
            current = current->next;
        }
        if (current->next) {
            free(current->next);
            current->next = NULL;
            stack->count--;
        }
    }
    
    StackNode* new_node = (StackNode*)malloc(sizeof(StackNode));
    if (!new_node) return false;
    
    memcpy(&new_node->data, action, sizeof(UndoAction));
    new_node->next = stack->top;
    stack->top = new_node;
    stack->count++;
    
    return true;
}

// Pop action from stack
// Time Complexity: O(1)
bool undo_stack_pop(UndoStack* stack, UndoAction* out_action) {
    if (!stack || !stack->top) return false;
    
    stack->operations_count++;
    if (out_action) {
        memcpy(out_action, &stack->top->data, sizeof(UndoAction));
    }
    
    StackNode* temp = stack->top;
    stack->top = stack->top->next;
    free(temp);
    stack->count--;
    
    return true;
}

// Peek at top action
// Time Complexity: O(1)
bool undo_stack_peek(UndoStack* stack, UndoAction* out_action) {
    if (!stack || !stack->top) return false;
    if (out_action) {
        memcpy(out_action, &stack->top->data, sizeof(UndoAction));
    }
    return true;
}

// Get all actions
int undo_stack_get_all(UndoStack* stack, UndoAction* out_actions, int max_count) {
    if (!stack || !out_actions) return 0;
    
    int count = 0;
    StackNode* current = stack->top;
    
    while (current && count < max_count) {
        memcpy(&out_actions[count], &current->data, sizeof(UndoAction));
        count++;
        current = current->next;
    }
    return count;
}

int undo_stack_size(UndoStack* stack) {
    return stack ? stack->count : 0;
}

bool undo_stack_is_empty(UndoStack* stack) {
    return stack ? stack->count == 0 : true;
}

void undo_stack_clear(UndoStack* stack) {
    if (!stack) return;
    
    while (stack->top) {
        StackNode* temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
    stack->count = 0;
}

int undo_stack_get_operations_count(UndoStack* stack) {
    return stack ? stack->operations_count : 0;
}

// ==================== TRANSACTION STACK ====================

// Create transaction stack
TransactionStack* trans_stack_create(int max_size) {
    TransactionStack* stack = (TransactionStack*)malloc(sizeof(TransactionStack));
    if (stack) {
        stack->top = NULL;
        stack->count = 0;
        stack->max_size = max_size > 0 ? max_size : 100;
        stack->operations_count = 0;
    }
    return stack;
}

// Destroy transaction stack
void trans_stack_destroy(TransactionStack* stack) {
    if (!stack) return;
    
    while (stack->top) {
        TStackNode* temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
    free(stack);
}

// Push transaction
// Time Complexity: O(1)
bool trans_stack_push(TransactionStack* stack, const Transaction* t) {
    if (!stack || !t) return false;
    
    stack->operations_count++;
    TStackNode* new_node = (TStackNode*)malloc(sizeof(TStackNode));
    if (!new_node) return false;
    
    memcpy(&new_node->data, t, sizeof(Transaction));
    new_node->next = stack->top;
    stack->top = new_node;
    stack->count++;
    
    // Trim if over max size
    if (stack->count > stack->max_size) {
        TStackNode* current = stack->top;
        for (int i = 0; i < stack->max_size - 1 && current; i++) {
            current = current->next;
        }
        if (current && current->next) {
            TStackNode* temp = current->next;
            current->next = NULL;
            while (temp) {
                TStackNode* to_delete = temp;
                temp = temp->next;
                free(to_delete);
                stack->count--;
            }
        }
    }
    
    return true;
}

// Pop transaction
// Time Complexity: O(1)
bool trans_stack_pop(TransactionStack* stack, Transaction* out_t) {
    if (!stack || !stack->top) return false;
    
    stack->operations_count++;
    if (out_t) {
        memcpy(out_t, &stack->top->data, sizeof(Transaction));
    }
    
    TStackNode* temp = stack->top;
    stack->top = stack->top->next;
    free(temp);
    stack->count--;
    
    return true;
}

// Peek at top
bool trans_stack_peek(TransactionStack* stack, Transaction* out_t) {
    if (!stack || !stack->top) return false;
    if (out_t) {
        memcpy(out_t, &stack->top->data, sizeof(Transaction));
    }
    return true;
}

// Get all transactions
int trans_stack_get_all(TransactionStack* stack, Transaction* out_transactions, int max_count) {
    if (!stack || !out_transactions) return 0;
    
    int count = 0;
    TStackNode* current = stack->top;
    
    while (current && count < max_count) {
        memcpy(&out_transactions[count], &current->data, sizeof(Transaction));
        count++;
        current = current->next;
    }
    return count;
}

// Get top N transactions
int trans_stack_get_top_n(TransactionStack* stack, int n, Transaction* out_transactions) {
    if (!stack || !out_transactions || n <= 0) return 0;
    
    stack->operations_count++;
    int count = 0;
    TStackNode* current = stack->top;
    
    while (current && count < n) {
        memcpy(&out_transactions[count], &current->data, sizeof(Transaction));
        count++;
        current = current->next;
    }
    return count;
}

int trans_stack_size(TransactionStack* stack) {
    return stack ? stack->count : 0;
}

bool trans_stack_is_empty(TransactionStack* stack) {
    return stack ? stack->count == 0 : true;
}

void trans_stack_clear(TransactionStack* stack) {
    if (!stack) return;
    
    while (stack->top) {
        TStackNode* temp = stack->top;
        stack->top = stack->top->next;
        free(temp);
    }
    stack->count = 0;
}

int trans_stack_get_operations_count(TransactionStack* stack) {
    return stack ? stack->operations_count : 0;
}
