/**
 * Sliding Window Algorithm Implementation
 * Efficient calculation of windowed aggregates
 */

#include "sliding_window.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Helper to compare dates
static int compare_dates(const char* d1, const char* d2) {
    return strcmp(d1, d2);
}

// Helper to find date in window
static int find_date_index(SlidingWindow* window, const char* date) {
    for (int i = 0; i < window->count; i++) {
        int idx = (window->start_index + i) % window->capacity;
        if (strcmp(window->data[idx].date, date) == 0) {
            return idx;
        }
    }
    return -1;
}

// Create sliding window
SlidingWindow* sliding_window_create(int window_size) {
    if (window_size <= 0 || window_size > MAX_WINDOW_SIZE) {
        window_size = 30;  // Default 30 days
    }
    
    SlidingWindow* window = (SlidingWindow*)malloc(sizeof(SlidingWindow));
    if (window) {
        window->data = (DailySpending*)calloc(window_size, sizeof(DailySpending));
        window->capacity = window_size;
        window->start_index = 0;
        window->count = 0;
        window->sum_income = 0;
        window->sum_expenses = 0;
        window->operations_count = 0;
        window->slide_count = 0;
        
        if (!window->data) {
            free(window);
            return NULL;
        }
    }
    return window;
}

// Destroy sliding window
void sliding_window_destroy(SlidingWindow* window) {
    if (!window) return;
    if (window->data) free(window->data);
    free(window);
}

// Add a new day to the window
// Time Complexity: O(1)
bool sliding_window_add_day(SlidingWindow* window, const char* date,
                            double income, double expenses, int tx_count) {
    if (!window || !date) return false;
    
    window->operations_count++;
    
    // Check if date already exists
    int existing = find_date_index(window, date);
    if (existing >= 0) {
        // Update existing
        window->sum_income -= window->data[existing].income;
        window->sum_expenses -= window->data[existing].expenses;
        window->data[existing].income = income;
        window->data[existing].expenses = expenses;
        window->data[existing].transaction_count = tx_count;
        window->sum_income += income;
        window->sum_expenses += expenses;
        return true;
    }
    
    // If window is full, slide it
    if (window->count >= window->capacity) {
        // Remove oldest entry from sums
        int oldest = window->start_index;
        window->sum_income -= window->data[oldest].income;
        window->sum_expenses -= window->data[oldest].expenses;
        window->start_index = (window->start_index + 1) % window->capacity;
        window->count--;
        window->slide_count++;
    }
    
    // Add new entry at end
    int new_idx = (window->start_index + window->count) % window->capacity;
    safe_strcpy(window->data[new_idx].date, date, sizeof(window->data[new_idx].date));
    window->data[new_idx].income = income;
    window->data[new_idx].expenses = expenses;
    window->data[new_idx].transaction_count = tx_count;
    
    window->sum_income += income;
    window->sum_expenses += expenses;
    window->count++;
    
    return true;
}

// Update existing day
bool sliding_window_update_day(SlidingWindow* window, const char* date,
                               double income_delta, double expense_delta, int tx_delta) {
    if (!window || !date) return false;
    
    window->operations_count++;
    
    int idx = find_date_index(window, date);
    if (idx < 0) {
        // Add new day with deltas as initial values
        return sliding_window_add_day(window, date, income_delta, expense_delta, tx_delta);
    }
    
    window->data[idx].income += income_delta;
    window->data[idx].expenses += expense_delta;
    window->data[idx].transaction_count += tx_delta;
    
    window->sum_income += income_delta;
    window->sum_expenses += expense_delta;
    
    return true;
}

// Get trend result
bool sliding_window_get_trend(SlidingWindow* window, TrendResult* result) {
    if (!window || !result || window->count == 0) return false;
    
    window->operations_count++;
    
    result->total_income = window->sum_income;
    result->total_expenses = window->sum_expenses;
    result->avg_daily_income = window->sum_income / window->count;
    result->avg_daily_expenses = window->sum_expenses / window->count;
    result->days_count = window->count;
    
    // Get start and end dates
    int start_idx = window->start_index;
    int end_idx = (window->start_index + window->count - 1) % window->capacity;
    safe_strcpy(result->start_date, window->data[start_idx].date, sizeof(result->start_date));
    safe_strcpy(result->end_date, window->data[end_idx].date, sizeof(result->end_date));
    
    // Calculate trend direction (compare first half vs second half)
    if (window->count >= 2) {
        int half = window->count / 2;
        double first_half = 0, second_half = 0;
        
        for (int i = 0; i < half; i++) {
            int idx = (window->start_index + i) % window->capacity;
            first_half += window->data[idx].expenses;
        }
        for (int i = half; i < window->count; i++) {
            int idx = (window->start_index + i) % window->capacity;
            second_half += window->data[idx].expenses;
        }
        
        first_half /= half;
        second_half /= (window->count - half);
        
        result->trend_direction = second_half - first_half;
    } else {
        result->trend_direction = 0;
    }
    
    return true;
}

// Get daily data
int sliding_window_get_daily_data(SlidingWindow* window, DailySpending* out, int max_count) {
    if (!window || !out) return 0;
    
    window->operations_count++;
    
    int count = window->count < max_count ? window->count : max_count;
    
    for (int i = 0; i < count; i++) {
        int idx = (window->start_index + i) % window->capacity;
        memcpy(&out[i], &window->data[idx], sizeof(DailySpending));
    }
    
    return count;
}

// Build from transactions
void sliding_window_build_from_transactions(SlidingWindow* window,
                                            const Transaction* transactions, int count,
                                            const char* end_date) {
    if (!window || !transactions || count == 0) return;
    
    sliding_window_clear(window);
    
    // Get current date if end_date not provided
    char effective_end[16];
    if (end_date && end_date[0]) {
        safe_strcpy(effective_end, end_date, sizeof(effective_end));
    } else {
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        strftime(effective_end, sizeof(effective_end), "%Y-%m-%d", tm_info);
    }
    
    // Calculate start date (window_size days before end_date)
    char start_date[16];
    struct tm tm = {0};
    sscanf(effective_end, "%d-%d-%d", &tm.tm_year, &tm.tm_mon, &tm.tm_mday);
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    time_t end_time = mktime(&tm);
    time_t start_time = end_time - ((window->capacity - 1) * 24 * 60 * 60);
    struct tm* start_tm = localtime(&start_time);
    strftime(start_date, sizeof(start_date), "%Y-%m-%d", start_tm);
    
    // Group transactions by date
    typedef struct {
        char date[16];
        double income;
        double expenses;
        int tx_count;
    } DateGroup;
    
    DateGroup* groups = (DateGroup*)calloc(count, sizeof(DateGroup));
    int num_groups = 0;
    
    for (int i = 0; i < count; i++) {
        if (compare_dates(transactions[i].date, start_date) < 0 ||
            compare_dates(transactions[i].date, effective_end) > 0) {
            continue;  // Outside window
        }
        
        // Find or create group
        int group_idx = -1;
        for (int j = 0; j < num_groups; j++) {
            if (strcmp(groups[j].date, transactions[i].date) == 0) {
                group_idx = j;
                break;
            }
        }
        
        if (group_idx < 0) {
            group_idx = num_groups++;
            safe_strcpy(groups[group_idx].date, transactions[i].date, 16);
            groups[group_idx].income = 0;
            groups[group_idx].expenses = 0;
            groups[group_idx].tx_count = 0;
        }
        
        if (strcmp(transactions[i].type, "income") == 0) {
            groups[group_idx].income += transactions[i].amount;
        } else {
            groups[group_idx].expenses += transactions[i].amount;
        }
        groups[group_idx].tx_count++;
    }
    
    // Sort groups by date
    for (int i = 0; i < num_groups - 1; i++) {
        for (int j = 0; j < num_groups - i - 1; j++) {
            if (strcmp(groups[j].date, groups[j+1].date) > 0) {
                DateGroup temp = groups[j];
                groups[j] = groups[j+1];
                groups[j+1] = temp;
            }
        }
    }
    
    // Add to window
    for (int i = 0; i < num_groups; i++) {
        sliding_window_add_day(window, groups[i].date, groups[i].income,
                               groups[i].expenses, groups[i].tx_count);
    }
    
    free(groups);
}

int sliding_window_get_operations_count(SlidingWindow* window) {
    return window ? window->operations_count : 0;
}

int sliding_window_get_slide_count(SlidingWindow* window) {
    return window ? window->slide_count : 0;
}

void sliding_window_clear(SlidingWindow* window) {
    if (!window) return;
    window->start_index = 0;
    window->count = 0;
    window->sum_income = 0;
    window->sum_expenses = 0;
}

// Calculate trend for any date range
void sliding_window_calc_trend(const Transaction* transactions, int count,
                               const char* start_date, const char* end_date,
                               TrendResult* result) {
    if (!transactions || !result || count == 0) return;
    
    memset(result, 0, sizeof(TrendResult));
    safe_strcpy(result->start_date, start_date, sizeof(result->start_date));
    safe_strcpy(result->end_date, end_date, sizeof(result->end_date));
    
    int days_count = 0;
    double first_half_expenses = 0, second_half_expenses = 0;
    int first_half_days = 0, second_half_days = 0;
    
    // Find midpoint date
    char mid_date[16];
    struct tm tm_start = {0}, tm_end = {0};
    sscanf(start_date, "%d-%d-%d", &tm_start.tm_year, &tm_start.tm_mon, &tm_start.tm_mday);
    sscanf(end_date, "%d-%d-%d", &tm_end.tm_year, &tm_end.tm_mon, &tm_end.tm_mday);
    tm_start.tm_year -= 1900; tm_start.tm_mon -= 1;
    tm_end.tm_year -= 1900; tm_end.tm_mon -= 1;
    time_t t_start = mktime(&tm_start);
    time_t t_end = mktime(&tm_end);
    time_t t_mid = t_start + (t_end - t_start) / 2;
    struct tm* tm_mid = localtime(&t_mid);
    strftime(mid_date, sizeof(mid_date), "%Y-%m-%d", tm_mid);
    
    for (int i = 0; i < count; i++) {
        if (compare_dates(transactions[i].date, start_date) >= 0 &&
            compare_dates(transactions[i].date, end_date) <= 0) {
            
            if (strcmp(transactions[i].type, "income") == 0) {
                result->total_income += transactions[i].amount;
            } else {
                result->total_expenses += transactions[i].amount;
                
                if (compare_dates(transactions[i].date, mid_date) < 0) {
                    first_half_expenses += transactions[i].amount;
                } else {
                    second_half_expenses += transactions[i].amount;
                }
            }
            days_count++;
        }
    }
    
    // Calculate days in range
    result->days_count = (int)((t_end - t_start) / (24 * 60 * 60)) + 1;
    
    if (result->days_count > 0) {
        result->avg_daily_income = result->total_income / result->days_count;
        result->avg_daily_expenses = result->total_expenses / result->days_count;
    }
    
    // Trend: compare averages of first and second half
    int half_days = result->days_count / 2;
    if (half_days > 0) {
        double first_avg = first_half_expenses / half_days;
        double second_avg = second_half_expenses / (result->days_count - half_days);
        result->trend_direction = second_avg - first_avg;
    }
}
