/**
 * Queue Implementation - Pure C
 * FIFO queue for managing bill payments
 */

#include "queue.h"

// Create new queue
BillQueue* queue_create(void) {
    BillQueue* queue = (BillQueue*)malloc(sizeof(BillQueue));
    if (queue) {
        queue->front = NULL;
        queue->rear = NULL;
        queue->count = 0;
        queue->operations_count = 0;
    }
    return queue;
}

// Destroy queue
void queue_destroy(BillQueue* queue) {
    if (!queue) return;
    
    QueueNode* current = queue->front;
    while (current) {
        QueueNode* temp = current;
        current = current->next;
        free(temp);
    }
    free(queue);
}

// Enqueue bill (add to rear)
// Time Complexity: O(1)
bool queue_enqueue(BillQueue* queue, const Bill* bill) {
    if (!queue || !bill) return false;
    
    queue->operations_count++;
    QueueNode* new_node = (QueueNode*)malloc(sizeof(QueueNode));
    if (!new_node) return false;
    
    memcpy(&new_node->data, bill, sizeof(Bill));
    new_node->next = NULL;
    
    if (queue->rear) {
        queue->rear->next = new_node;
    } else {
        queue->front = new_node;
    }
    queue->rear = new_node;
    queue->count++;
    
    return true;
}

// Dequeue bill (remove from front)
// Time Complexity: O(1)
bool queue_dequeue(BillQueue* queue, Bill* out_bill) {
    if (!queue || !queue->front) return false;
    
    queue->operations_count++;
    if (out_bill) {
        memcpy(out_bill, &queue->front->data, sizeof(Bill));
    }
    
    QueueNode* temp = queue->front;
    queue->front = queue->front->next;
    
    if (!queue->front) {
        queue->rear = NULL;
    }
    
    free(temp);
    queue->count--;
    return true;
}

// Peek at front bill
// Time Complexity: O(1)
bool queue_peek(BillQueue* queue, Bill* out_bill) {
    if (!queue || !queue->front) return false;
    if (out_bill) {
        memcpy(out_bill, &queue->front->data, sizeof(Bill));
    }
    return true;
}

// Get all bills
// Time Complexity: O(n)
int queue_get_all_bills(BillQueue* queue, Bill* out_bills, int max_count) {
    if (!queue || !out_bills) return 0;
    
    queue->operations_count++;
    int count = 0;
    QueueNode* current = queue->front;
    
    while (current && count < max_count) {
        memcpy(&out_bills[count], &current->data, sizeof(Bill));
        count++;
        current = current->next;
    }
    return count;
}

// Find bill by ID
// Time Complexity: O(n)
bool queue_find_by_id(BillQueue* queue, const char* id, Bill* out_bill) {
    if (!queue || !id) return false;
    
    queue->operations_count++;
    QueueNode* current = queue->front;
    
    while (current) {
        if (strcmp(current->data.id, id) == 0) {
            if (out_bill) {
                memcpy(out_bill, &current->data, sizeof(Bill));
            }
            return true;
        }
        current = current->next;
    }
    return false;
}

// Remove bill by ID
// Time Complexity: O(n)
bool queue_remove_by_id(BillQueue* queue, const char* id) {
    if (!queue || !id || !queue->front) return false;
    
    queue->operations_count++;
    
    // Special case: front is the target
    if (strcmp(queue->front->data.id, id) == 0) {
        return queue_dequeue(queue, NULL);
    }
    
    QueueNode* current = queue->front;
    while (current->next) {
        if (strcmp(current->next->data.id, id) == 0) {
            QueueNode* temp = current->next;
            current->next = temp->next;
            
            if (temp == queue->rear) {
                queue->rear = current;
            }
            
            free(temp);
            queue->count--;
            return true;
        }
        current = current->next;
    }
    return false;
}

// Mark bill as paid
bool queue_mark_as_paid(BillQueue* queue, const char* id) {
    if (!queue || !id) return false;
    
    queue->operations_count++;
    QueueNode* current = queue->front;
    
    while (current) {
        if (strcmp(current->data.id, id) == 0) {
            current->data.is_paid = true;
            return true;
        }
        current = current->next;
    }
    return false;
}

// Get unpaid bills
int queue_get_unpaid_bills(BillQueue* queue, Bill* out_bills, int max_count) {
    if (!queue || !out_bills) return 0;
    
    queue->operations_count++;
    int count = 0;
    QueueNode* current = queue->front;
    
    while (current && count < max_count) {
        if (!current->data.is_paid) {
            memcpy(&out_bills[count], &current->data, sizeof(Bill));
            count++;
        }
        current = current->next;
    }
    return count;
}

// Get overdue bills (due_date < current_date)
int queue_get_overdue_bills(BillQueue* queue, const char* current_date, Bill* out_bills, int max_count) {
    if (!queue || !current_date || !out_bills) return 0;
    
    queue->operations_count++;
    int count = 0;
    QueueNode* current = queue->front;
    
    while (current && count < max_count) {
        if (!current->data.is_paid && strcmp(current->data.due_date, current_date) < 0) {
            memcpy(&out_bills[count], &current->data, sizeof(Bill));
            count++;
        }
        current = current->next;
    }
    return count;
}

int queue_size(BillQueue* queue) {
    return queue ? queue->count : 0;
}

bool queue_is_empty(BillQueue* queue) {
    return queue ? queue->count == 0 : true;
}

void queue_clear(BillQueue* queue) {
    if (!queue) return;
    
    while (queue->front) {
        QueueNode* temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
    }
    queue->rear = NULL;
    queue->count = 0;
}

int queue_get_operations_count(BillQueue* queue) {
    return queue ? queue->operations_count : 0;
}
