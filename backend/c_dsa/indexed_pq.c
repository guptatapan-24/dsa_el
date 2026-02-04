/**
 * Indexed Priority Queue Implementation
 * Max-heap with index tracking for efficient priority updates
 */

#include "indexed_pq.h"
#include <stdlib.h>
#include <string.h>

// Calculate priority (percent used)
static double calc_priority(double spent, double limit) {
    return limit > 0 ? (spent / limit) * 100.0 : 0.0;
}

// Swap two alerts and update index
static void swap(IndexedPQ* pq, int i, int j) {
    // Swap heap elements
    BudgetAlert temp = pq->heap[i];
    pq->heap[i] = pq->heap[j];
    pq->heap[j] = temp;
    
    // Update index map
    for (int k = 0; k < pq->size; k++) {
        if (strcmp(pq->index_map[k].key, pq->heap[i].category) == 0) {
            pq->index_map[k].heap_index = i;
        }
        if (strcmp(pq->index_map[k].key, pq->heap[j].category) == 0) {
            pq->index_map[k].heap_index = j;
        }
    }
}

// Find index in index_map by key
static int find_index(IndexedPQ* pq, const char* key) {
    for (int i = 0; i < pq->size; i++) {
        if (strcmp(pq->index_map[i].key, key) == 0) {
            return pq->index_map[i].heap_index;
        }
    }
    return -1;
}

// Bubble up (swim)
static void swim(IndexedPQ* pq, int k) {
    while (k > 0 && pq->heap[(k-1)/2].priority < pq->heap[k].priority) {
        swap(pq, k, (k-1)/2);
        k = (k-1)/2;
        pq->heapify_count++;
    }
}

// Sink down
static void sink(IndexedPQ* pq, int k) {
    while (2*k + 1 < pq->size) {
        int j = 2*k + 1;  // Left child
        if (j + 1 < pq->size && pq->heap[j].priority < pq->heap[j+1].priority) {
            j++;  // Right child is larger
        }
        if (pq->heap[k].priority >= pq->heap[j].priority) break;
        swap(pq, k, j);
        k = j;
        pq->heapify_count++;
    }
}

// Create new Indexed Priority Queue
IndexedPQ* ipq_create(int capacity) {
    if (capacity <= 0) capacity = IPQ_MAX_SIZE;
    
    IndexedPQ* pq = (IndexedPQ*)malloc(sizeof(IndexedPQ));
    if (pq) {
        pq->heap = (BudgetAlert*)malloc(sizeof(BudgetAlert) * capacity);
        pq->index_map = (IPQIndex*)malloc(sizeof(IPQIndex) * capacity);
        pq->size = 0;
        pq->capacity = capacity;
        pq->operations_count = 0;
        pq->heapify_count = 0;
        
        if (!pq->heap || !pq->index_map) {
            if (pq->heap) free(pq->heap);
            if (pq->index_map) free(pq->index_map);
            free(pq);
            return NULL;
        }
    }
    return pq;
}

// Destroy priority queue
void ipq_destroy(IndexedPQ* pq) {
    if (!pq) return;
    if (pq->heap) free(pq->heap);
    if (pq->index_map) free(pq->index_map);
    free(pq);
}

// Insert new item - O(log n)
bool ipq_insert(IndexedPQ* pq, const char* category, double spent, double limit) {
    if (!pq || !category || pq->size >= pq->capacity) return false;
    
    pq->operations_count++;
    
    // Check if category already exists
    int existing = find_index(pq, category);
    if (existing >= 0) {
        // Update instead
        return ipq_update_priority(pq, category, spent);
    }
    
    // Add new item
    int i = pq->size;
    safe_strcpy(pq->heap[i].category, category, MAX_STRING_LEN);
    pq->heap[i].spent = spent;
    pq->heap[i].budget_limit = limit;
    pq->heap[i].priority = calc_priority(spent, limit);
    
    // Add to index map
    safe_strcpy(pq->index_map[i].key, category, MAX_STRING_LEN);
    pq->index_map[i].heap_index = i;
    
    pq->size++;
    
    // Restore heap property
    swim(pq, i);
    
    return true;
}

// Extract max (highest priority) - O(log n)
bool ipq_extract_max(IndexedPQ* pq, BudgetAlert* out) {
    if (!pq || pq->size == 0) return false;
    
    pq->operations_count++;
    
    if (out) {
        memcpy(out, &pq->heap[0], sizeof(BudgetAlert));
    }
    
    // Move last to root and sink
    pq->size--;
    if (pq->size > 0) {
        pq->heap[0] = pq->heap[pq->size];
        
        // Update index map
        for (int i = 0; i < pq->size; i++) {
            if (strcmp(pq->index_map[i].key, pq->heap[0].category) == 0) {
                pq->index_map[i].heap_index = 0;
                break;
            }
        }
        
        sink(pq, 0);
    }
    
    return true;
}

// Update priority - O(log n)
bool ipq_update_priority(IndexedPQ* pq, const char* category, double new_spent) {
    if (!pq || !category) return false;
    
    pq->operations_count++;
    
    int idx = find_index(pq, category);
    if (idx < 0) return false;
    
    double old_priority = pq->heap[idx].priority;
    pq->heap[idx].spent = new_spent;
    pq->heap[idx].priority = calc_priority(new_spent, pq->heap[idx].budget_limit);
    
    // Restore heap property
    if (pq->heap[idx].priority > old_priority) {
        swim(pq, idx);
    } else {
        sink(pq, idx);
    }
    
    return true;
}

// Peek max without removing
bool ipq_peek_max(IndexedPQ* pq, BudgetAlert* out) {
    if (!pq || pq->size == 0 || !out) return false;
    pq->operations_count++;
    memcpy(out, &pq->heap[0], sizeof(BudgetAlert));
    return true;
}

// Check if category exists
bool ipq_contains(IndexedPQ* pq, const char* category) {
    return pq && category && find_index(pq, category) >= 0;
}

// Get by key
bool ipq_get_by_key(IndexedPQ* pq, const char* category, BudgetAlert* out) {
    if (!pq || !category || !out) return false;
    pq->operations_count++;
    
    int idx = find_index(pq, category);
    if (idx < 0) return false;
    
    memcpy(out, &pq->heap[idx], sizeof(BudgetAlert));
    return true;
}

// Get all sorted by priority (descending)
int ipq_get_all_sorted(IndexedPQ* pq, BudgetAlert* out, int max_count) {
    if (!pq || !out || max_count <= 0) return 0;
    
    pq->operations_count++;
    
    // Copy heap to temp array for sorting
    BudgetAlert* temp = (BudgetAlert*)malloc(sizeof(BudgetAlert) * pq->size);
    if (!temp) return 0;
    
    memcpy(temp, pq->heap, sizeof(BudgetAlert) * pq->size);
    
    // Simple bubble sort for small arrays (could use heap sort for larger)
    for (int i = 0; i < pq->size - 1; i++) {
        for (int j = 0; j < pq->size - i - 1; j++) {
            if (temp[j].priority < temp[j+1].priority) {
                BudgetAlert t = temp[j];
                temp[j] = temp[j+1];
                temp[j+1] = t;
            }
        }
    }
    
    int count = pq->size < max_count ? pq->size : max_count;
    memcpy(out, temp, sizeof(BudgetAlert) * count);
    
    free(temp);
    return count;
}

// Get alerts above threshold
int ipq_get_alerts_above_threshold(IndexedPQ* pq, double threshold, BudgetAlert* out, int max_count) {
    if (!pq || !out || max_count <= 0) return 0;
    
    pq->operations_count++;
    
    // First get sorted
    BudgetAlert* sorted = (BudgetAlert*)malloc(sizeof(BudgetAlert) * pq->size);
    if (!sorted) return 0;
    
    int total = ipq_get_all_sorted(pq, sorted, pq->size);
    
    int count = 0;
    for (int i = 0; i < total && count < max_count; i++) {
        if (sorted[i].priority >= threshold) {
            memcpy(&out[count], &sorted[i], sizeof(BudgetAlert));
            count++;
        }
    }
    
    free(sorted);
    return count;
}

// Remove by key
bool ipq_remove(IndexedPQ* pq, const char* category) {
    if (!pq || !category || pq->size == 0) return false;
    
    pq->operations_count++;
    
    int idx = find_index(pq, category);
    if (idx < 0) return false;
    
    // Move last to this position
    pq->size--;
    if (idx < pq->size) {
        pq->heap[idx] = pq->heap[pq->size];
        pq->index_map[idx] = pq->index_map[pq->size];
        pq->index_map[idx].heap_index = idx;
        
        // Restore heap property
        swim(pq, idx);
        sink(pq, idx);
    }
    
    return true;
}

int ipq_size(IndexedPQ* pq) {
    return pq ? pq->size : 0;
}

int ipq_get_operations_count(IndexedPQ* pq) {
    return pq ? pq->operations_count : 0;
}

int ipq_get_heapify_count(IndexedPQ* pq) {
    return pq ? pq->heapify_count : 0;
}

void ipq_clear(IndexedPQ* pq) {
    if (!pq) return;
    pq->size = 0;
}
