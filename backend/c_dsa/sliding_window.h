/**
 * Sliding Window Algorithm Implementation
 * Efficient calculation of windowed aggregates
 * Data Structures & Applications Lab Project - Pure C Implementation
 * 
 * Properties:
 * - O(1) per update (add new, remove oldest)
 * - Fixed-size window over time series data
 * - Used for 7-day and 30-day spending trends
 */

#ifndef SLIDING_WINDOW_H
#define SLIDING_WINDOW_H

#include "common.h"

#define MAX_WINDOW_SIZE 365  // Max 1 year

// Daily spending record
typedef struct {
    char date[16];          // YYYY-MM-DD
    double income;
    double expenses;
    int transaction_count;
} DailySpending;

// Sliding window for trends
typedef struct {
    DailySpending* data;    // Circular buffer
    int capacity;           // Window size
    int start_index;        // Start of current window
    int count;              // Current number of days
    double sum_income;      // Running sum of income
    double sum_expenses;    // Running sum of expenses
    int operations_count;
    int slide_count;        // Track window slides for proof
} SlidingWindow;

// Trend result
typedef struct {
    double total_income;
    double total_expenses;
    double avg_daily_income;
    double avg_daily_expenses;
    double trend_direction;   // Positive = increasing, negative = decreasing
    int days_count;
    char start_date[16];
    char end_date[16];
} TrendResult;

// Function declarations
SlidingWindow* sliding_window_create(int window_size);
void sliding_window_destroy(SlidingWindow* window);

// Core operations
bool sliding_window_add_day(SlidingWindow* window, const char* date, 
                            double income, double expenses, int tx_count);
bool sliding_window_update_day(SlidingWindow* window, const char* date,
                               double income_delta, double expense_delta, int tx_delta);

// Get trends
bool sliding_window_get_trend(SlidingWindow* window, TrendResult* result);
int sliding_window_get_daily_data(SlidingWindow* window, DailySpending* out, int max_count);

// Build from transactions
void sliding_window_build_from_transactions(SlidingWindow* window, 
                                            const Transaction* transactions, int count,
                                            const char* end_date);

// Statistics
int sliding_window_get_operations_count(SlidingWindow* window);
int sliding_window_get_slide_count(SlidingWindow* window);
void sliding_window_clear(SlidingWindow* window);

// Calculate trend for any date range
void sliding_window_calc_trend(const Transaction* transactions, int count,
                               const char* start_date, const char* end_date,
                               TrendResult* result);

#endif // SLIDING_WINDOW_H
