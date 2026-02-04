/**
 * Z-Score Implementation using Welford's Online Algorithm
 * Time Complexity: O(1) per update and query
 */

#include "zscore.h"
#include <math.h>

// ==================== WELFORD'S ALGORITHM ====================

WelfordStats welford_create(void) {
    WelfordStats stats;
    stats.count = 0;
    stats.mean = 0.0;
    stats.m2 = 0.0;
    stats.min_value = 0.0;
    stats.max_value = 0.0;
    stats.sum = 0.0;
    stats.operations_count = 0;
    stats.anomalies_detected = 0;
    return stats;
}

// Welford's online update - O(1)
void welford_update(WelfordStats* stats, double value) {
    if (!stats) return;
    
    stats->operations_count++;
    stats->count++;
    stats->sum += value;
    
    // Update min/max
    if (stats->count == 1) {
        stats->min_value = value;
        stats->max_value = value;
    } else {
        if (value < stats->min_value) stats->min_value = value;
        if (value > stats->max_value) stats->max_value = value;
    }
    
    // Welford's algorithm for numerical stability
    double delta = value - stats->mean;
    stats->mean += delta / stats->count;
    double delta2 = value - stats->mean;
    stats->m2 += delta * delta2;
}

// Approximate removal (not exact, but useful for sliding windows)
void welford_remove(WelfordStats* stats, double value) {
    if (!stats || stats->count <= 0) return;
    
    stats->operations_count++;
    
    if (stats->count == 1) {
        welford_reset(stats);
        return;
    }
    
    stats->sum -= value;
    
    // Reverse Welford's update (approximate)
    double delta = value - stats->mean;
    stats->mean = (stats->mean * stats->count - value) / (stats->count - 1);
    double delta2 = value - stats->mean;
    stats->m2 -= delta * delta2;
    if (stats->m2 < 0) stats->m2 = 0;  // Numerical stability
    
    stats->count--;
}

double welford_get_mean(WelfordStats* stats) {
    return stats ? stats->mean : 0.0;
}

double welford_get_variance(WelfordStats* stats) {
    if (!stats || stats->count < 2) return 0.0;
    return stats->m2 / (stats->count - 1);  // Sample variance (Bessel's correction)
}

double welford_get_std_dev(WelfordStats* stats) {
    return sqrt(welford_get_variance(stats));
}

// Calculate Z-score - O(1)
double welford_get_z_score(WelfordStats* stats, double value) {
    if (!stats) return 0.0;
    
    stats->operations_count++;
    
    double std_dev = welford_get_std_dev(stats);
    if (std_dev < 0.0001) return 0.0;  // Avoid division by near-zero
    
    return (value - stats->mean) / std_dev;
}

// Check if value is an anomaly - O(1)
bool welford_is_anomaly(WelfordStats* stats, double value, double threshold) {
    if (!stats || stats->count < 3) return false;  // Need enough data
    
    double z = fabs(welford_get_z_score(stats, value));
    bool is_anomaly = z > threshold;
    
    if (is_anomaly) {
        stats->anomalies_detected++;
    }
    
    return is_anomaly;
}

void welford_reset(WelfordStats* stats) {
    if (!stats) return;
    stats->count = 0;
    stats->mean = 0.0;
    stats->m2 = 0.0;
    stats->min_value = 0.0;
    stats->max_value = 0.0;
    stats->sum = 0.0;
    // Keep operations count for tracking
}

int welford_get_operations_count(WelfordStats* stats) {
    return stats ? stats->operations_count : 0;
}

// ==================== Z-SCORE TRACKER ====================

ZScoreTracker* zscore_tracker_create(void) {
    ZScoreTracker* tracker = (ZScoreTracker*)malloc(sizeof(ZScoreTracker));
    if (tracker) {
        tracker->overall_expenses = welford_create();
        tracker->overall_income = welford_create();
        tracker->daily_expenses = welford_create();
        tracker->category_capacity = 100;
        tracker->category_stats = (CategorySpendingStats*)calloc(
            tracker->category_capacity, sizeof(CategorySpendingStats));
        tracker->category_count = 0;
        tracker->operations_count = 0;
    }
    return tracker;
}

void zscore_tracker_destroy(ZScoreTracker* tracker) {
    if (!tracker) return;
    if (tracker->category_stats) free(tracker->category_stats);
    free(tracker);
}

// Find or create category stats
static CategorySpendingStats* find_or_create_category(ZScoreTracker* tracker, const char* category) {
    // Find existing
    for (int i = 0; i < tracker->category_count; i++) {
        if (strcmp(tracker->category_stats[i].category, category) == 0) {
            return &tracker->category_stats[i];
        }
    }
    
    // Create new
    if (tracker->category_count >= tracker->category_capacity) {
        // Expand array
        tracker->category_capacity *= 2;
        tracker->category_stats = (CategorySpendingStats*)realloc(
            tracker->category_stats, 
            tracker->category_capacity * sizeof(CategorySpendingStats));
    }
    
    CategorySpendingStats* new_cat = &tracker->category_stats[tracker->category_count];
    safe_strcpy(new_cat->category, category, MAX_STRING_LEN);
    new_cat->stats = welford_create();
    tracker->category_count++;
    
    return new_cat;
}

// Update expense statistics - O(1)
void zscore_update_expense(ZScoreTracker* tracker, double amount, const char* category) {
    if (!tracker || amount <= 0) return;
    
    tracker->operations_count++;
    
    // Update overall expense stats
    welford_update(&tracker->overall_expenses, amount);
    
    // Update category stats
    if (category && category[0]) {
        CategorySpendingStats* cat_stats = find_or_create_category(tracker, category);
        welford_update(&cat_stats->stats, amount);
    }
}

void zscore_update_income(ZScoreTracker* tracker, double amount) {
    if (!tracker || amount <= 0) return;
    
    tracker->operations_count++;
    welford_update(&tracker->overall_income, amount);
}

void zscore_update_daily(ZScoreTracker* tracker, double daily_expense) {
    if (!tracker) return;
    
    tracker->operations_count++;
    welford_update(&tracker->daily_expenses, daily_expense);
}

// Check for expense anomaly - O(1)
bool zscore_check_expense_anomaly(ZScoreTracker* tracker, double amount, AnomalyResult* result) {
    if (!tracker || !result) return false;
    
    tracker->operations_count++;
    
    memset(result, 0, sizeof(AnomalyResult));
    result->value = amount;
    result->mean = tracker->overall_expenses.mean;
    result->std_dev = welford_get_std_dev(&tracker->overall_expenses);
    result->z_score = welford_get_z_score(&tracker->overall_expenses, amount);
    result->is_anomaly = welford_is_anomaly(&tracker->overall_expenses, amount, ZSCORE_ANOMALY_THRESHOLD);
    
    if (result->is_anomaly) {
        if (result->z_score > 0) {
            snprintf(result->description, MAX_STRING_LEN,
                     "Unusually high expense: $%.2f (%.1f std devs above average $%.2f)",
                     amount, result->z_score, result->mean);
        } else {
            snprintf(result->description, MAX_STRING_LEN,
                     "Unusually low expense: $%.2f (%.1f std devs below average $%.2f)",
                     amount, fabs(result->z_score), result->mean);
        }
    }
    
    return result->is_anomaly;
}

bool zscore_check_category_anomaly(ZScoreTracker* tracker, const char* category,
                                   double amount, AnomalyResult* result) {
    if (!tracker || !category || !result) return false;
    
    tracker->operations_count++;
    
    memset(result, 0, sizeof(AnomalyResult));
    result->value = amount;
    
    // Find category
    for (int i = 0; i < tracker->category_count; i++) {
        if (strcmp(tracker->category_stats[i].category, category) == 0) {
            WelfordStats* stats = &tracker->category_stats[i].stats;
            result->mean = stats->mean;
            result->std_dev = welford_get_std_dev(stats);
            result->z_score = welford_get_z_score(stats, amount);
            result->is_anomaly = welford_is_anomaly(stats, amount, ZSCORE_ANOMALY_THRESHOLD);
            
            if (result->is_anomaly) {
                snprintf(result->description, MAX_STRING_LEN,
                         "Unusual %s expense: $%.2f (z-score: %.2f, avg: $%.2f)",
                         category, amount, result->z_score, result->mean);
            }
            
            return result->is_anomaly;
        }
    }
    
    return false;
}

bool zscore_check_daily_anomaly(ZScoreTracker* tracker, double daily_expense, AnomalyResult* result) {
    if (!tracker || !result) return false;
    
    tracker->operations_count++;
    
    memset(result, 0, sizeof(AnomalyResult));
    result->value = daily_expense;
    result->mean = tracker->daily_expenses.mean;
    result->std_dev = welford_get_std_dev(&tracker->daily_expenses);
    result->z_score = welford_get_z_score(&tracker->daily_expenses, daily_expense);
    result->is_anomaly = welford_is_anomaly(&tracker->daily_expenses, daily_expense, ZSCORE_ANOMALY_THRESHOLD);
    
    if (result->is_anomaly) {
        snprintf(result->description, MAX_STRING_LEN,
                 "Unusual daily spending: $%.2f (z-score: %.2f, avg: $%.2f/day)",
                 daily_expense, result->z_score, result->mean);
    }
    
    return result->is_anomaly;
}

double zscore_get_avg_expense(ZScoreTracker* tracker) {
    return tracker ? tracker->overall_expenses.mean : 0.0;
}

double zscore_get_expense_std_dev(ZScoreTracker* tracker) {
    return tracker ? welford_get_std_dev(&tracker->overall_expenses) : 0.0;
}

double zscore_get_avg_income(ZScoreTracker* tracker) {
    return tracker ? tracker->overall_income.mean : 0.0;
}

double zscore_get_category_avg(ZScoreTracker* tracker, const char* category) {
    if (!tracker || !category) return 0.0;
    
    for (int i = 0; i < tracker->category_count; i++) {
        if (strcmp(tracker->category_stats[i].category, category) == 0) {
            return tracker->category_stats[i].stats.mean;
        }
    }
    return 0.0;
}

int zscore_get_anomaly_count(ZScoreTracker* tracker) {
    if (!tracker) return 0;
    return tracker->overall_expenses.anomalies_detected +
           tracker->daily_expenses.anomalies_detected;
}

int zscore_get_operations_count(ZScoreTracker* tracker) {
    return tracker ? tracker->operations_count : 0;
}

void zscore_get_expense_stats(ZScoreTracker* tracker, double* mean, double* std_dev,
                              double* min, double* max, int* count) {
    if (!tracker) return;
    
    if (mean) *mean = tracker->overall_expenses.mean;
    if (std_dev) *std_dev = welford_get_std_dev(&tracker->overall_expenses);
    if (min) *min = tracker->overall_expenses.min_value;
    if (max) *max = tracker->overall_expenses.max_value;
    if (count) *count = tracker->overall_expenses.count;
}

void zscore_get_category_stats(ZScoreTracker* tracker, const char* category,
                               double* mean, double* std_dev, int* count) {
    if (!tracker || !category) return;
    
    for (int i = 0; i < tracker->category_count; i++) {
        if (strcmp(tracker->category_stats[i].category, category) == 0) {
            WelfordStats* stats = &tracker->category_stats[i].stats;
            if (mean) *mean = stats->mean;
            if (std_dev) *std_dev = welford_get_std_dev(stats);
            if (count) *count = stats->count;
            return;
        }
    }
}

void zscore_reset(ZScoreTracker* tracker) {
    if (!tracker) return;
    
    welford_reset(&tracker->overall_expenses);
    welford_reset(&tracker->overall_income);
    welford_reset(&tracker->daily_expenses);
    
    for (int i = 0; i < tracker->category_count; i++) {
        welford_reset(&tracker->category_stats[i].stats);
    }
    tracker->category_count = 0;
}
