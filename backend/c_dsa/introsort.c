/**
 * IntroSort Implementation
 * Hybrid sorting algorithm combining QuickSort, HeapSort, and InsertionSort
 * Time Complexity: O(n log n) guaranteed
 */

#include "introsort.h"
#include <math.h>

// ==================== HELPER FUNCTIONS ====================

static void swap_transactions(Transaction* a, Transaction* b) {
    Transaction temp;
    memcpy(&temp, a, sizeof(Transaction));
    memcpy(a, b, sizeof(Transaction));
    memcpy(b, &temp, sizeof(Transaction));
}

static void swap_categories(CategoryAmount* a, CategoryAmount* b) {
    CategoryAmount temp;
    memcpy(&temp, a, sizeof(CategoryAmount));
    memcpy(a, b, sizeof(CategoryAmount));
    memcpy(b, &temp, sizeof(CategoryAmount));
}

// ==================== INSERTION SORT (for small arrays) ====================

static void insertion_sort_transactions_amount_desc(Transaction* arr, int left, int right, IntroSortStats* stats) {
    if (stats) stats->insertion_sort_calls++;
    
    for (int i = left + 1; i <= right; i++) {
        Transaction key;
        memcpy(&key, &arr[i], sizeof(Transaction));
        int j = i - 1;
        
        while (j >= left && arr[j].amount < key.amount) {
            if (stats) stats->comparisons++;
            memcpy(&arr[j + 1], &arr[j], sizeof(Transaction));
            if (stats) stats->swaps++;
            j--;
        }
        if (j >= left && stats) stats->comparisons++;
        
        memcpy(&arr[j + 1], &key, sizeof(Transaction));
    }
}

static void insertion_sort_transactions_date_asc(Transaction* arr, int left, int right, IntroSortStats* stats) {
    if (stats) stats->insertion_sort_calls++;
    
    for (int i = left + 1; i <= right; i++) {
        Transaction key;
        memcpy(&key, &arr[i], sizeof(Transaction));
        int j = i - 1;
        
        while (j >= left && strcmp(arr[j].date, key.date) > 0) {
            if (stats) stats->comparisons++;
            memcpy(&arr[j + 1], &arr[j], sizeof(Transaction));
            if (stats) stats->swaps++;
            j--;
        }
        if (j >= left && stats) stats->comparisons++;
        
        memcpy(&arr[j + 1], &key, sizeof(Transaction));
    }
}

static void insertion_sort_categories_amount_desc(CategoryAmount* arr, int left, int right, IntroSortStats* stats) {
    if (stats) stats->insertion_sort_calls++;
    
    for (int i = left + 1; i <= right; i++) {
        CategoryAmount key;
        memcpy(&key, &arr[i], sizeof(CategoryAmount));
        int j = i - 1;
        
        while (j >= left && arr[j].total_amount < key.total_amount) {
            if (stats) stats->comparisons++;
            memcpy(&arr[j + 1], &arr[j], sizeof(CategoryAmount));
            if (stats) stats->swaps++;
            j--;
        }
        if (j >= left && stats) stats->comparisons++;
        
        memcpy(&arr[j + 1], &key, sizeof(CategoryAmount));
    }
}

// ==================== HEAPSORT (fallback for deep recursion) ====================

static void heapify_transactions_amount_desc(Transaction* arr, int n, int i, int offset, IntroSortStats* stats) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < n) {
        if (stats) stats->comparisons++;
        if (arr[offset + left].amount > arr[offset + largest].amount) {
            largest = left;
        }
    }
    
    if (right < n) {
        if (stats) stats->comparisons++;
        if (arr[offset + right].amount > arr[offset + largest].amount) {
            largest = right;
        }
    }
    
    if (largest != i) {
        swap_transactions(&arr[offset + i], &arr[offset + largest]);
        if (stats) stats->swaps++;
        heapify_transactions_amount_desc(arr, n, largest, offset, stats);
    }
}

static void heapsort_transactions_amount_desc(Transaction* arr, int left, int right, IntroSortStats* stats) {
    if (stats) stats->heapsort_calls++;
    
    int n = right - left + 1;
    
    // Build max heap (for descending, we want min at root, so build min heap)
    // Actually for descending order, we extract max repeatedly
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify_transactions_amount_desc(arr, n, i, left, stats);
    }
    
    // Extract elements from heap one by one
    for (int i = n - 1; i > 0; i--) {
        swap_transactions(&arr[left], &arr[left + i]);
        if (stats) stats->swaps++;
        heapify_transactions_amount_desc(arr, i, 0, left, stats);
    }
    
    // Reverse for descending order
    for (int i = 0; i < n / 2; i++) {
        swap_transactions(&arr[left + i], &arr[left + n - 1 - i]);
        if (stats) stats->swaps++;
    }
}

static void heapify_transactions_date_asc(Transaction* arr, int n, int i, int offset, IntroSortStats* stats) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < n) {
        if (stats) stats->comparisons++;
        if (strcmp(arr[offset + left].date, arr[offset + largest].date) > 0) {
            largest = left;
        }
    }
    
    if (right < n) {
        if (stats) stats->comparisons++;
        if (strcmp(arr[offset + right].date, arr[offset + largest].date) > 0) {
            largest = right;
        }
    }
    
    if (largest != i) {
        swap_transactions(&arr[offset + i], &arr[offset + largest]);
        if (stats) stats->swaps++;
        heapify_transactions_date_asc(arr, n, largest, offset, stats);
    }
}

static void heapsort_transactions_date_asc(Transaction* arr, int left, int right, IntroSortStats* stats) {
    if (stats) stats->heapsort_calls++;
    
    int n = right - left + 1;
    
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify_transactions_date_asc(arr, n, i, left, stats);
    }
    
    for (int i = n - 1; i > 0; i--) {
        swap_transactions(&arr[left], &arr[left + i]);
        if (stats) stats->swaps++;
        heapify_transactions_date_asc(arr, i, 0, left, stats);
    }
}

static void heapify_categories_amount_desc(CategoryAmount* arr, int n, int i, int offset, IntroSortStats* stats) {
    int largest = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;
    
    if (left < n) {
        if (stats) stats->comparisons++;
        if (arr[offset + left].total_amount > arr[offset + largest].total_amount) {
            largest = left;
        }
    }
    
    if (right < n) {
        if (stats) stats->comparisons++;
        if (arr[offset + right].total_amount > arr[offset + largest].total_amount) {
            largest = right;
        }
    }
    
    if (largest != i) {
        swap_categories(&arr[offset + i], &arr[offset + largest]);
        if (stats) stats->swaps++;
        heapify_categories_amount_desc(arr, n, largest, offset, stats);
    }
}

static void heapsort_categories_amount_desc(CategoryAmount* arr, int left, int right, IntroSortStats* stats) {
    if (stats) stats->heapsort_calls++;
    
    int n = right - left + 1;
    
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify_categories_amount_desc(arr, n, i, left, stats);
    }
    
    for (int i = n - 1; i > 0; i--) {
        swap_categories(&arr[left], &arr[left + i]);
        if (stats) stats->swaps++;
        heapify_categories_amount_desc(arr, i, 0, left, stats);
    }
    
    // Reverse for descending order
    for (int i = 0; i < n / 2; i++) {
        swap_categories(&arr[left + i], &arr[left + n - 1 - i]);
        if (stats) stats->swaps++;
    }
}

// ==================== QUICKSORT PARTITION ====================

// Median of three pivot selection
static int median_of_three_transactions_amount(Transaction* arr, int left, int right) {
    int mid = left + (right - left) / 2;
    
    if (arr[left].amount < arr[mid].amount) {
        if (arr[mid].amount < arr[right].amount) return mid;
        else if (arr[left].amount < arr[right].amount) return right;
        else return left;
    } else {
        if (arr[left].amount < arr[right].amount) return left;
        else if (arr[mid].amount < arr[right].amount) return right;
        else return mid;
    }
}

static int partition_transactions_amount_desc(Transaction* arr, int left, int right, IntroSortStats* stats) {
    if (stats) stats->quicksort_partitions++;
    
    // Use median of three for pivot selection
    int pivot_idx = median_of_three_transactions_amount(arr, left, right);
    swap_transactions(&arr[pivot_idx], &arr[right]);
    if (stats) stats->swaps++;
    
    double pivot = arr[right].amount;
    int i = left - 1;
    
    for (int j = left; j < right; j++) {
        if (stats) stats->comparisons++;
        if (arr[j].amount >= pivot) {  // Descending order
            i++;
            swap_transactions(&arr[i], &arr[j]);
            if (stats) stats->swaps++;
        }
    }
    
    swap_transactions(&arr[i + 1], &arr[right]);
    if (stats) stats->swaps++;
    
    return i + 1;
}

static int partition_transactions_date_asc(Transaction* arr, int left, int right, IntroSortStats* stats) {
    if (stats) stats->quicksort_partitions++;
    
    int mid = left + (right - left) / 2;
    // Simple median of three for dates
    if (strcmp(arr[left].date, arr[mid].date) > 0) {
        swap_transactions(&arr[left], &arr[mid]);
        if (stats) stats->swaps++;
    }
    if (strcmp(arr[left].date, arr[right].date) > 0) {
        swap_transactions(&arr[left], &arr[right]);
        if (stats) stats->swaps++;
    }
    if (strcmp(arr[mid].date, arr[right].date) > 0) {
        swap_transactions(&arr[mid], &arr[right]);
        if (stats) stats->swaps++;
    }
    swap_transactions(&arr[mid], &arr[right]);
    if (stats) stats->swaps++;
    
    char pivot[16];
    safe_strcpy(pivot, arr[right].date, sizeof(pivot));
    int i = left - 1;
    
    for (int j = left; j < right; j++) {
        if (stats) stats->comparisons++;
        if (strcmp(arr[j].date, pivot) <= 0) {  // Ascending order
            i++;
            swap_transactions(&arr[i], &arr[j]);
            if (stats) stats->swaps++;
        }
    }
    
    swap_transactions(&arr[i + 1], &arr[right]);
    if (stats) stats->swaps++;
    
    return i + 1;
}

static int partition_categories_amount_desc(CategoryAmount* arr, int left, int right, IntroSortStats* stats) {
    if (stats) stats->quicksort_partitions++;
    
    int mid = left + (right - left) / 2;
    // Median of three
    if (arr[left].total_amount < arr[mid].total_amount) {
        swap_categories(&arr[left], &arr[mid]);
        if (stats) stats->swaps++;
    }
    if (arr[left].total_amount < arr[right].total_amount) {
        swap_categories(&arr[left], &arr[right]);
        if (stats) stats->swaps++;
    }
    if (arr[mid].total_amount < arr[right].total_amount) {
        swap_categories(&arr[mid], &arr[right]);
        if (stats) stats->swaps++;
    }
    swap_categories(&arr[mid], &arr[right]);
    if (stats) stats->swaps++;
    
    double pivot = arr[right].total_amount;
    int i = left - 1;
    
    for (int j = left; j < right; j++) {
        if (stats) stats->comparisons++;
        if (arr[j].total_amount >= pivot) {  // Descending order
            i++;
            swap_categories(&arr[i], &arr[j]);
            if (stats) stats->swaps++;
        }
    }
    
    swap_categories(&arr[i + 1], &arr[right]);
    if (stats) stats->swaps++;
    
    return i + 1;
}

// ==================== INTROSORT MAIN FUNCTIONS ====================

static void introsort_util_transactions_amount_desc(Transaction* arr, int left, int right, int depth_limit, IntroSortStats* stats) {
    int size = right - left + 1;
    
    if (size <= INSERTION_SORT_THRESHOLD) {
        insertion_sort_transactions_amount_desc(arr, left, right, stats);
        return;
    }
    
    if (depth_limit == 0) {
        heapsort_transactions_amount_desc(arr, left, right, stats);
        return;
    }
    
    int pivot = partition_transactions_amount_desc(arr, left, right, stats);
    introsort_util_transactions_amount_desc(arr, left, pivot - 1, depth_limit - 1, stats);
    introsort_util_transactions_amount_desc(arr, pivot + 1, right, depth_limit - 1, stats);
}

static void introsort_util_transactions_date_asc(Transaction* arr, int left, int right, int depth_limit, IntroSortStats* stats) {
    int size = right - left + 1;
    
    if (size <= INSERTION_SORT_THRESHOLD) {
        insertion_sort_transactions_date_asc(arr, left, right, stats);
        return;
    }
    
    if (depth_limit == 0) {
        heapsort_transactions_date_asc(arr, left, right, stats);
        return;
    }
    
    int pivot = partition_transactions_date_asc(arr, left, right, stats);
    introsort_util_transactions_date_asc(arr, left, pivot - 1, depth_limit - 1, stats);
    introsort_util_transactions_date_asc(arr, pivot + 1, right, depth_limit - 1, stats);
}

static void introsort_util_categories_amount_desc(CategoryAmount* arr, int left, int right, int depth_limit, IntroSortStats* stats) {
    int size = right - left + 1;
    
    if (size <= INSERTION_SORT_THRESHOLD) {
        insertion_sort_categories_amount_desc(arr, left, right, stats);
        return;
    }
    
    if (depth_limit == 0) {
        heapsort_categories_amount_desc(arr, left, right, stats);
        return;
    }
    
    int pivot = partition_categories_amount_desc(arr, left, right, stats);
    introsort_util_categories_amount_desc(arr, left, pivot - 1, depth_limit - 1, stats);
    introsort_util_categories_amount_desc(arr, pivot + 1, right, depth_limit - 1, stats);
}

// ==================== PUBLIC API ====================

void introsort_transactions_by_amount(Transaction* arr, int n, IntroSortStats* stats) {
    if (!arr || n <= 1) return;
    
    if (stats) {
        stats->total_operations++;
    }
    
    int depth_limit = 2 * (int)log2((double)n);
    introsort_util_transactions_amount_desc(arr, 0, n - 1, depth_limit, stats);
}

void introsort_transactions_by_date(Transaction* arr, int n, IntroSortStats* stats) {
    if (!arr || n <= 1) return;
    
    if (stats) {
        stats->total_operations++;
    }
    
    int depth_limit = 2 * (int)log2((double)n);
    introsort_util_transactions_date_asc(arr, 0, n - 1, depth_limit, stats);
}

void introsort_categories_by_amount(CategoryAmount* arr, int n, IntroSortStats* stats) {
    if (!arr || n <= 1) return;
    
    if (stats) {
        stats->total_operations++;
    }
    
    int depth_limit = 2 * (int)log2((double)n);
    introsort_util_categories_amount_desc(arr, 0, n - 1, depth_limit, stats);
}

int introsort_get_top_k_expenses(Transaction* arr, int n, int k, Transaction* out, IntroSortStats* stats) {
    if (!arr || !out || n <= 0 || k <= 0) return 0;
    
    // Sort by amount descending
    introsort_transactions_by_amount(arr, n, stats);
    
    // Copy top k
    int count = k < n ? k : n;
    memcpy(out, arr, sizeof(Transaction) * count);
    
    return count;
}

int introsort_get_top_k_categories(CategoryAmount* arr, int n, int k, CategoryAmount* out, IntroSortStats* stats) {
    if (!arr || !out || n <= 0 || k <= 0) return 0;
    
    // Sort by amount descending
    introsort_categories_by_amount(arr, n, stats);
    
    // Copy top k
    int count = k < n ? k : n;
    memcpy(out, arr, sizeof(CategoryAmount) * count);
    
    return count;
}

void introsort_reset_stats(IntroSortStats* stats) {
    if (!stats) return;
    memset(stats, 0, sizeof(IntroSortStats));
}

int introsort_get_operations_count(IntroSortStats* stats) {
    if (!stats) return 0;
    return stats->total_operations + stats->quicksort_partitions + 
           stats->heapsort_calls + stats->insertion_sort_calls;
}
