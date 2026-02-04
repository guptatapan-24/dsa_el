/**
 * Z-Score Implementation using Welford's Online Algorithm
 * Efficient O(1) incremental calculation of mean, variance, and Z-score
 * Data Structures & Applications Lab Project - Pure C Implementation
 * 
 * Properties:
 * - O(1) per update using Welford's online algorithm
 * - Numerically stable variance calculation
 * - Used for anomaly detection in spending patterns
 * - Detects unusually high/low transactions
 */

#ifndef ZSCORE_H
#define ZSCORE_H

#include "common.h"

// Anomaly threshold (number of standard deviations)
#define ZSCORE_ANOMALY_THRESHOLD 2.0

// Welford's online statistics tracker
typedef struct {
    int count;              // Number of data points
    double mean;            // Running mean
    double m2;              // Sum of squared differences from mean
    double min_value;       // Minimum observed value
    double max_value;       // Maximum observed value
    double sum;             // Running sum (for convenience)
    int operations_count;   // Track operations for proof
    int anomalies_detected; // Count of anomalies found
} WelfordStats;

// Anomaly result
typedef struct {
    bool is_anomaly;
    double z_score;
    double value;
    double mean;
    double std_dev;
    char description[MAX_STRING_LEN];
} AnomalyResult;

// Spending statistics for a category
typedef struct {
    char category[MAX_STRING_LEN];
    WelfordStats stats;
} CategorySpendingStats;

// Main Z-Score tracker structure
typedef struct {
    WelfordStats overall_expenses;     // Overall expense tracking
    WelfordStats overall_income;       // Overall income tracking
    WelfordStats daily_expenses;       // Daily expense patterns
    CategorySpendingStats* category_stats;  // Per-category stats
    int category_count;
    int category_capacity;
    int operations_count;
} ZScoreTracker;

// Function declarations

// Welford's online algorithm functions
WelfordStats welford_create(void);
void welford_update(WelfordStats* stats, double value);
void welford_remove(WelfordStats* stats, double value);  // Approximate removal
double welford_get_mean(WelfordStats* stats);
double welford_get_variance(WelfordStats* stats);
double welford_get_std_dev(WelfordStats* stats);
double welford_get_z_score(WelfordStats* stats, double value);
bool welford_is_anomaly(WelfordStats* stats, double value, double threshold);
void welford_reset(WelfordStats* stats);
int welford_get_operations_count(WelfordStats* stats);

// Z-Score Tracker functions
ZScoreTracker* zscore_tracker_create(void);
void zscore_tracker_destroy(ZScoreTracker* tracker);

// Update statistics - O(1)
void zscore_update_expense(ZScoreTracker* tracker, double amount, const char* category);
void zscore_update_income(ZScoreTracker* tracker, double amount);
void zscore_update_daily(ZScoreTracker* tracker, double daily_expense);

// Anomaly detection - O(1)
bool zscore_check_expense_anomaly(ZScoreTracker* tracker, double amount, AnomalyResult* result);
bool zscore_check_category_anomaly(ZScoreTracker* tracker, const char* category, 
                                   double amount, AnomalyResult* result);
bool zscore_check_daily_anomaly(ZScoreTracker* tracker, double daily_expense, AnomalyResult* result);

// Get statistics
double zscore_get_avg_expense(ZScoreTracker* tracker);
double zscore_get_expense_std_dev(ZScoreTracker* tracker);
double zscore_get_avg_income(ZScoreTracker* tracker);
double zscore_get_category_avg(ZScoreTracker* tracker, const char* category);
int zscore_get_anomaly_count(ZScoreTracker* tracker);
int zscore_get_operations_count(ZScoreTracker* tracker);

// Get detailed stats
void zscore_get_expense_stats(ZScoreTracker* tracker, double* mean, double* std_dev, 
                              double* min, double* max, int* count);
void zscore_get_category_stats(ZScoreTracker* tracker, const char* category,
                               double* mean, double* std_dev, int* count);

// Reset tracker
void zscore_reset(ZScoreTracker* tracker);

#endif // ZSCORE_H
