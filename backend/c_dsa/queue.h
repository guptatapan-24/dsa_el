/**
 * Queue Implementation for Upcoming Bill Payments (FIFO)
 * Data Structures & Applications Lab Project - Pure C Implementation
 * Operations: enqueue O(1), dequeue O(1), peek O(1)
 */

#ifndef QUEUE_H
#define QUEUE_H

#include "common.h"

// Queue node
typedef struct QueueNode {
    Bill data;
    struct QueueNode* next;
} QueueNode;

// Bill Queue structure
typedef struct {
    QueueNode* front;
    QueueNode* rear;
    int count;
    int operations_count;
} BillQueue;

// Function declarations
BillQueue* queue_create(void);
void queue_destroy(BillQueue* queue);
bool queue_enqueue(BillQueue* queue, const Bill* bill);
bool queue_dequeue(BillQueue* queue, Bill* out_bill);
bool queue_peek(BillQueue* queue, Bill* out_bill);
int queue_get_all_bills(BillQueue* queue, Bill* out_bills, int max_count);
bool queue_find_by_id(BillQueue* queue, const char* id, Bill* out_bill);
bool queue_remove_by_id(BillQueue* queue, const char* id);
bool queue_mark_as_paid(BillQueue* queue, const char* id);
int queue_get_unpaid_bills(BillQueue* queue, Bill* out_bills, int max_count);
int queue_get_overdue_bills(BillQueue* queue, const char* current_date, Bill* out_bills, int max_count);
int queue_size(BillQueue* queue);
bool queue_is_empty(BillQueue* queue);
void queue_clear(BillQueue* queue);
int queue_get_operations_count(BillQueue* queue);

#endif // QUEUE_H
