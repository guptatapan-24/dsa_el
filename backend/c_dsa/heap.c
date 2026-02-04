/**
 * Max Heap Implementation - Pure C
 * Used for finding top spending transactions and categories
 */

#include "heap.h"

// Helper macros
#define PARENT(i) (((i) - 1) / 2)
#define LEFT_CHILD(i) (2 * (i) + 1)
#define RIGHT_CHILD(i) (2 * (i) + 2)

// ==================== TRANSACTION MAX HEAP ====================

// Create max heap
MaxHeap* maxheap_create(int capacity) {
    MaxHeap* heap = (MaxHeap*)malloc(sizeof(MaxHeap));
    if (heap) {
        heap->data = (Transaction*)malloc(sizeof(Transaction) * capacity);
        heap->size = 0;
        heap->capacity = capacity;
        heap->operations_count = 0;
    }
    return heap;
}

// Destroy heap
void maxheap_destroy(MaxHeap* heap) {
    if (heap) {
        if (heap->data) free(heap->data);
        free(heap);
    }
}

// Swap transactions
static void swap_transactions(Transaction* a, Transaction* b) {
    Transaction temp;
    memcpy(&temp, a, sizeof(Transaction));
    memcpy(a, b, sizeof(Transaction));
    memcpy(b, &temp, sizeof(Transaction));
}

// Heapify up (for insertion)
// Time Complexity: O(log n)
static void heapify_up(MaxHeap* heap, int i) {
    while (i > 0 && heap->data[PARENT(i)].amount < heap->data[i].amount) {
        swap_transactions(&heap->data[PARENT(i)], &heap->data[i]);
        i = PARENT(i);
    }
}

// Heapify down (for extraction)
// Time Complexity: O(log n)
static void heapify_down(MaxHeap* heap, int i) {
    int largest = i;
    int left = LEFT_CHILD(i);
    int right = RIGHT_CHILD(i);
    
    if (left < heap->size && heap->data[left].amount > heap->data[largest].amount) {
        largest = left;
    }
    if (right < heap->size && heap->data[right].amount > heap->data[largest].amount) {
        largest = right;
    }
    
    if (largest != i) {
        swap_transactions(&heap->data[i], &heap->data[largest]);
        heapify_down(heap, largest);
    }
}

// Insert transaction
// Time Complexity: O(log n)
bool maxheap_insert(MaxHeap* heap, const Transaction* t) {
    if (!heap || !t || heap->size >= heap->capacity) return false;
    
    heap->operations_count++;
    memcpy(&heap->data[heap->size], t, sizeof(Transaction));
    heapify_up(heap, heap->size);
    heap->size++;
    
    return true;
}

// Extract max (highest amount)
// Time Complexity: O(log n)
bool maxheap_extract_max(MaxHeap* heap, Transaction* out_t) {
    if (!heap || heap->size == 0) return false;
    
    heap->operations_count++;
    if (out_t) {
        memcpy(out_t, &heap->data[0], sizeof(Transaction));
    }
    
    memcpy(&heap->data[0], &heap->data[heap->size - 1], sizeof(Transaction));
    heap->size--;
    
    if (heap->size > 0) {
        heapify_down(heap, 0);
    }
    
    return true;
}

// Peek at max without removing
// Time Complexity: O(1)
bool maxheap_peek(MaxHeap* heap, Transaction* out_t) {
    if (!heap || heap->size == 0) return false;
    if (out_t) {
        memcpy(out_t, &heap->data[0], sizeof(Transaction));
    }
    return true;
}

// Get top K transactions
// Time Complexity: O(k log n)
int maxheap_get_top_k(MaxHeap* heap, int k, Transaction* out_transactions) {
    if (!heap || !out_transactions || k <= 0) return 0;
    
    heap->operations_count++;
    
    // Create backup
    Transaction* backup = (Transaction*)malloc(sizeof(Transaction) * heap->size);
    int original_size = heap->size;
    memcpy(backup, heap->data, sizeof(Transaction) * heap->size);
    
    int count = 0;
    while (count < k && heap->size > 0) {
        maxheap_extract_max(heap, &out_transactions[count]);
        count++;
    }
    
    // Restore heap
    memcpy(heap->data, backup, sizeof(Transaction) * original_size);
    heap->size = original_size;
    free(backup);
    
    return count;
}

// Build heap from array
// Time Complexity: O(n)
void maxheap_build(MaxHeap* heap, const Transaction* transactions, int count) {
    if (!heap || !transactions) return;
    
    heap->operations_count++;
    int copy_count = (count < heap->capacity) ? count : heap->capacity;
    memcpy(heap->data, transactions, sizeof(Transaction) * copy_count);
    heap->size = copy_count;
    
    // Build heap from bottom up
    for (int i = heap->size / 2 - 1; i >= 0; i--) {
        heapify_down(heap, i);
    }
}

int maxheap_size(MaxHeap* heap) {
    return heap ? heap->size : 0;
}

void maxheap_clear(MaxHeap* heap) {
    if (heap) heap->size = 0;
}

int maxheap_get_operations_count(MaxHeap* heap) {
    return heap ? heap->operations_count : 0;
}

// ==================== CATEGORY MAX HEAP ====================

static void swap_categories(CategoryAmount* a, CategoryAmount* b) {
    CategoryAmount temp;
    memcpy(&temp, a, sizeof(CategoryAmount));
    memcpy(a, b, sizeof(CategoryAmount));
    memcpy(b, &temp, sizeof(CategoryAmount));
}

static void cat_heapify_up(CategoryMaxHeap* heap, int i) {
    while (i > 0 && heap->data[PARENT(i)].total_amount < heap->data[i].total_amount) {
        swap_categories(&heap->data[PARENT(i)], &heap->data[i]);
        i = PARENT(i);
    }
}

static void cat_heapify_down(CategoryMaxHeap* heap, int i) {
    int largest = i;
    int left = LEFT_CHILD(i);
    int right = RIGHT_CHILD(i);
    
    if (left < heap->size && heap->data[left].total_amount > heap->data[largest].total_amount) {
        largest = left;
    }
    if (right < heap->size && heap->data[right].total_amount > heap->data[largest].total_amount) {
        largest = right;
    }
    
    if (largest != i) {
        swap_categories(&heap->data[i], &heap->data[largest]);
        cat_heapify_down(heap, largest);
    }
}

CategoryMaxHeap* cat_maxheap_create(int capacity) {
    CategoryMaxHeap* heap = (CategoryMaxHeap*)malloc(sizeof(CategoryMaxHeap));
    if (heap) {
        heap->data = (CategoryAmount*)malloc(sizeof(CategoryAmount) * capacity);
        heap->size = 0;
        heap->capacity = capacity;
        heap->operations_count = 0;
    }
    return heap;
}

void cat_maxheap_destroy(CategoryMaxHeap* heap) {
    if (heap) {
        if (heap->data) free(heap->data);
        free(heap);
    }
}

bool cat_maxheap_insert(CategoryMaxHeap* heap, const CategoryAmount* ca) {
    if (!heap || !ca || heap->size >= heap->capacity) return false;
    
    heap->operations_count++;
    memcpy(&heap->data[heap->size], ca, sizeof(CategoryAmount));
    cat_heapify_up(heap, heap->size);
    heap->size++;
    
    return true;
}

bool cat_maxheap_extract_max(CategoryMaxHeap* heap, CategoryAmount* out_ca) {
    if (!heap || heap->size == 0) return false;
    
    heap->operations_count++;
    if (out_ca) {
        memcpy(out_ca, &heap->data[0], sizeof(CategoryAmount));
    }
    
    memcpy(&heap->data[0], &heap->data[heap->size - 1], sizeof(CategoryAmount));
    heap->size--;
    
    if (heap->size > 0) {
        cat_heapify_down(heap, 0);
    }
    
    return true;
}

int cat_maxheap_get_top_k(CategoryMaxHeap* heap, int k, CategoryAmount* out_categories) {
    if (!heap || !out_categories || k <= 0) return 0;
    
    heap->operations_count++;
    
    CategoryAmount* backup = (CategoryAmount*)malloc(sizeof(CategoryAmount) * heap->size);
    int original_size = heap->size;
    memcpy(backup, heap->data, sizeof(CategoryAmount) * heap->size);
    
    int count = 0;
    while (count < k && heap->size > 0) {
        cat_maxheap_extract_max(heap, &out_categories[count]);
        count++;
    }
    
    memcpy(heap->data, backup, sizeof(CategoryAmount) * original_size);
    heap->size = original_size;
    free(backup);
    
    return count;
}

void cat_maxheap_build(CategoryMaxHeap* heap, const CategoryAmount* categories, int count) {
    if (!heap || !categories) return;
    
    heap->operations_count++;
    int copy_count = (count < heap->capacity) ? count : heap->capacity;
    memcpy(heap->data, categories, sizeof(CategoryAmount) * copy_count);
    heap->size = copy_count;
    
    for (int i = heap->size / 2 - 1; i >= 0; i--) {
        cat_heapify_down(heap, i);
    }
}

int cat_maxheap_size(CategoryMaxHeap* heap) {
    return heap ? heap->size : 0;
}

void cat_maxheap_clear(CategoryMaxHeap* heap) {
    if (heap) heap->size = 0;
}

int cat_maxheap_get_operations_count(CategoryMaxHeap* heap) {
    return heap ? heap->operations_count : 0;
}
