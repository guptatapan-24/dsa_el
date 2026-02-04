/**
 * Finance Engine Implementation - Pure C
 * Integrates all required data structures with operation tracking
 * 
 * Data Structures Used (as per requirements):
 * 1. Red-Black Tree - Transaction storage with O(log n) insert, O(log n + k) range query
 * 2. Skip List - Transaction lookup by ID with O(log n) expected
 * 3. IntroSort - Top K expenses with O(n log n) sorting
 * 4. Polynomial Hash Map - Budget storage with O(1) average set/get
 * 5. Indexed Priority Queue - Budget alerts sorted with O(n log n)
 * 6. Sliding Window - 7-day trend with O(7) = O(1)
 * 7. Z-Score (Welford's) - Anomaly detection with O(1) update
 * 8. Queue - Bill management with O(1) enqueue, O(n) search
 * 9. Stack - Undo operations with O(1) pop
 */

#include "finance_engine.h"
#include <stdio.h>
#include <time.h>

// Static buffer for returning generated IDs
static char id_buffer[64];
static char bill_id_buffer[64];

// Helper to generate transaction ID
static const char* generate_transaction_id(FinanceEngine* engine) {
    snprintf(id_buffer, sizeof(id_buffer), "txn_%ld_%d", 
             (long)time(NULL), ++engine->transaction_counter);
    return id_buffer;
}

// Helper to generate bill ID
static const char* generate_bill_id(FinanceEngine* engine) {
    snprintf(bill_id_buffer, sizeof(bill_id_buffer), "bill_%ld_%d",
             (long)time(NULL), ++engine->bill_counter);
    return bill_id_buffer;
}

// Update DSA stats from all data structures
static void update_stats(FinanceEngine* engine) {
    if (!engine) return;
    
    // Red-Black Tree operations
    engine->stats.rbtree_ops = rbtree_get_operations_count(engine->transaction_tree);
    
    // Skip List operations
    engine->stats.skiplist_ops = skiplist_get_operations_count(engine->transaction_skiplist);
    
    // IntroSort operations
    engine->stats.introsort_ops = introsort_get_operations_count(&engine->sort_stats);
    
    // Polynomial HashMap operations
    engine->stats.hashmap_ops = hashmap_get_operations_count(engine->budget_map) +
                                hashmap_get_operations_count(engine->expense_map);
    
    // Indexed Priority Queue operations
    engine->stats.indexed_pq_ops = ipq_get_operations_count(engine->budget_alerts_pq);
    
    // Sliding Window operations
    engine->stats.sliding_window_ops = sliding_window_get_operations_count(engine->spending_window_7day) +
                                       sliding_window_get_operations_count(engine->spending_window_30day);
    
    // Z-Score (Welford's) operations
    engine->stats.zscore_ops = zscore_get_operations_count(engine->anomaly_tracker);
    
    // Queue operations
    engine->stats.queue_ops = queue_get_operations_count(engine->bill_queue);
    
    // Stack operations
    engine->stats.stack_ops = undo_stack_get_operations_count(engine->undo_stack);
    
    // Trie operations
    engine->stats.trie_ops = trie_get_operations_count(engine->category_trie) +
                             trie_get_operations_count(engine->payee_trie);
    
    // Total operations
    engine->stats.total_ops = engine->stats.rbtree_ops +
                              engine->stats.skiplist_ops +
                              engine->stats.introsort_ops +
                              engine->stats.hashmap_ops +
                              engine->stats.indexed_pq_ops +
                              engine->stats.sliding_window_ops +
                              engine->stats.zscore_ops +
                              engine->stats.queue_ops +
                              engine->stats.stack_ops +
                              engine->stats.trie_ops;
}

// ==================== ENGINE LIFECYCLE ====================

FinanceEngine* engine_create(void) {
    FinanceEngine* engine = (FinanceEngine*)malloc(sizeof(FinanceEngine));
    if (!engine) return NULL;
    
    // Initialize all data structures
    
    // 1. Red-Black Tree for date-based transactions
    engine->transaction_tree = rbtree_create();
    
    // 2. Skip List for O(log n) ID lookup
    engine->transaction_skiplist = skiplist_create();
    
    // 3. Polynomial Hash Maps for budgets and expenses
    engine->budget_map = hashmap_create();
    engine->expense_map = hashmap_create();
    
    // 4. Indexed Priority Queue for budget alerts
    engine->budget_alerts_pq = ipq_create(MAX_BUDGETS);
    
    // 5. Sliding Windows for spending trends
    engine->spending_window_7day = sliding_window_create(7);
    engine->spending_window_30day = sliding_window_create(30);
    
    // 6. Z-Score Tracker for anomaly detection
    engine->anomaly_tracker = zscore_tracker_create();
    
    // 7. Queue for bills (FIFO)
    engine->bill_queue = queue_create();
    
    // 8. Stack for undo operations
    engine->undo_stack = undo_stack_create(50);
    
    // 9. Tries for autocomplete
    engine->category_trie = trie_create();
    engine->payee_trie = trie_create();
    
    // Initialize IntroSort stats
    introsort_reset_stats(&engine->sort_stats);
    
    // Initialize counters and stats
    engine->transaction_counter = 0;
    engine->bill_counter = 0;
    memset(&engine->stats, 0, sizeof(DSAStats));
    
    // Add default categories to trie
    const char* default_categories[] = {
        "Food", "Transport", "Shopping", "Entertainment", "Bills",
        "Healthcare", "Education", "Salary", "Freelance", "Investment",
        "Rent", "Utilities", "Groceries", "Dining", "Travel"
    };
    
    for (int i = 0; i < 15; i++) {
        trie_insert(engine->category_trie, default_categories[i]);
    }
    
    return engine;
}

void engine_destroy(FinanceEngine* engine) {
    if (!engine) return;
    
    // Destroy all data structures
    rbtree_destroy(engine->transaction_tree);
    skiplist_destroy(engine->transaction_skiplist);
    hashmap_destroy(engine->budget_map);
    hashmap_destroy(engine->expense_map);
    ipq_destroy(engine->budget_alerts_pq);
    sliding_window_destroy(engine->spending_window_7day);
    sliding_window_destroy(engine->spending_window_30day);
    zscore_tracker_destroy(engine->anomaly_tracker);
    queue_destroy(engine->bill_queue);
    undo_stack_destroy(engine->undo_stack);
    trie_destroy(engine->category_trie);
    trie_destroy(engine->payee_trie);
    
    free(engine);
}

// ==================== EXPENSE TRACKING HELPER ====================

static void update_expense_tracking(FinanceEngine* engine, const Transaction* t, bool is_add) {
    if (strcmp(t->type, "expense") != 0) return;
    
    Budget current_budget;
    memset(&current_budget, 0, sizeof(Budget));
    safe_strcpy(current_budget.category, t->category, MAX_STRING_LEN);
    
    // Get current expense total from expense map
    double current_total = 0;
    Budget temp_budget;
    if (hashmap_search(engine->expense_map, t->category, &temp_budget)) {
        current_total = temp_budget.spent;
    }
    
    // Update expense total
    if (is_add) {
        current_total += t->amount;
        // Update Z-Score tracker with new expense
        zscore_update_expense(engine->anomaly_tracker, t->amount, t->category);
    } else {
        current_total = current_total > t->amount ? current_total - t->amount : 0;
    }
    
    current_budget.spent = current_total;
    hashmap_insert(engine->expense_map, t->category, &current_budget);
    
    // Update budget spent if exists and update indexed priority queue
    Budget budget;
    if (hashmap_search(engine->budget_map, t->category, &budget)) {
        budget.spent = current_total;
        hashmap_update(engine->budget_map, t->category, &budget);
        
        // Update the priority queue with new spending
        ipq_update_priority(engine->budget_alerts_pq, t->category, current_total);
    }
}

// ==================== TRANSACTION OPERATIONS ====================

const char* engine_add_transaction(FinanceEngine* engine, const char* type,
                                   double amount, const char* category,
                                   const char* description, const char* date) {
    if (!engine || !type || !category) return NULL;
    
    // Generate ID
    const char* id = generate_transaction_id(engine);
    
    // Create transaction
    Transaction t;
    safe_strcpy(t.id, id, sizeof(t.id));
    safe_strcpy(t.type, type, sizeof(t.type));
    t.amount = amount;
    safe_strcpy(t.category, category, sizeof(t.category));
    safe_strcpy(t.description, description ? description : "", sizeof(t.description));
    
    // Use current date if not provided
    if (date && date[0] != '\0') {
        safe_strcpy(t.date, date, sizeof(t.date));
    } else {
        time_t now = time(NULL);
        struct tm* tm_info = localtime(&now);
        strftime(t.date, sizeof(t.date), "%Y-%m-%d", tm_info);
    }
    
    // Add to data structures (PROOF: Each DS is used!)
    
    // 1. Red-Black Tree: O(log n) insert by date
    rbtree_insert(engine->transaction_tree, &t);
    
    // 2. Skip List: O(log n) expected insert for ID lookup
    skiplist_insert(engine->transaction_skiplist, &t);
    
    // Update expense tracking (uses HashMap and Z-Score)
    update_expense_tracking(engine, &t, true);
    
    // Update income stats if income
    if (strcmp(type, "income") == 0) {
        zscore_update_income(engine->anomaly_tracker, amount);
    }
    
    // Add to tries (Trie: O(m))
    trie_insert(engine->category_trie, category);
    if (description && description[0] != '\0') {
        trie_insert(engine->payee_trie, description);
    }
    
    // Record for undo (Stack: O(1))
    UndoAction action;
    action.type = ACTION_ADD_TRANSACTION;
    snprintf(action.data, sizeof(action.data), "%s|%s|%.2f|%s|%s|%s",
             t.id, t.type, t.amount, t.category, t.description, t.date);
    undo_stack_push(engine->undo_stack, &action);
    
    update_stats(engine);
    return id_buffer;
}

bool engine_delete_transaction(FinanceEngine* engine, const char* id) {
    if (!engine || !id) return false;
    
    // Find transaction first using Skip List: O(log n) expected
    Transaction t;
    if (!skiplist_search(engine->transaction_skiplist, id, &t)) {
        return false;
    }
    
    // Record for undo
    UndoAction action;
    action.type = ACTION_DELETE_TRANSACTION;
    snprintf(action.data, sizeof(action.data), "%s|%s|%.2f|%s|%s|%s",
             t.id, t.type, t.amount, t.category, t.description, t.date);
    undo_stack_push(engine->undo_stack, &action);
    
    // Delete from Skip List: O(log n) expected
    skiplist_delete(engine->transaction_skiplist, id);
    
    // Delete from Red-Black Tree: O(log n)
    rbtree_delete_by_id(engine->transaction_tree, id);
    
    // Update expense tracking
    update_expense_tracking(engine, &t, false);
    
    update_stats(engine);
    return true;
}

bool engine_find_transaction(FinanceEngine* engine, const char* id, Transaction* out) {
    if (!engine || !id || !out) return false;
    
    // Use Skip List for O(log n) expected lookup
    bool result = skiplist_search(engine->transaction_skiplist, id, out);
    update_stats(engine);
    return result;
}

int engine_get_all_transactions(FinanceEngine* engine, Transaction* out, int max_count) {
    if (!engine || !out) return 0;
    
    // Use Red-Black Tree inorder traversal: O(n)
    int count = rbtree_inorder_traversal(engine->transaction_tree, out, max_count);
    update_stats(engine);
    return count;
}

int engine_get_transactions_desc(FinanceEngine* engine, Transaction* out, int max_count) {
    if (!engine || !out) return 0;
    
    // Use Red-Black Tree reverse inorder: O(n)
    int count = rbtree_reverse_inorder(engine->transaction_tree, out, max_count);
    update_stats(engine);
    return count;
}

int engine_get_transactions_in_range(FinanceEngine* engine, const char* start,
                                     const char* end, Transaction* out, int max_count) {
    if (!engine || !start || !end || !out) return 0;
    
    // Use Red-Black Tree range query: O(log n + k)
    int count = rbtree_range_query(engine->transaction_tree, start, end, out, max_count);
    update_stats(engine);
    return count;
}

int engine_get_recent_transactions(FinanceEngine* engine, int count, Transaction* out) {
    if (!engine || !out || count <= 0) return 0;
    
    // Get all transactions in descending order (most recent first)
    Transaction all[MAX_TRANSACTIONS];
    int total = rbtree_reverse_inorder(engine->transaction_tree, all, MAX_TRANSACTIONS);
    
    // Copy the requested count
    int result = count < total ? count : total;
    memcpy(out, all, sizeof(Transaction) * result);
    
    update_stats(engine);
    return result;
}

// ==================== BUDGET OPERATIONS ====================

bool engine_set_budget(FinanceEngine* engine, const char* category, double limit) {
    if (!engine || !category) return false;
    
    Budget budget;
    safe_strcpy(budget.category, category, MAX_STRING_LEN);
    budget.limit = limit;
    
    // Get current spending from expense map
    Budget expense;
    if (hashmap_search(engine->expense_map, category, &expense)) {
        budget.spent = expense.spent;
    } else {
        budget.spent = 0;
    }
    
    // Check if updating existing
    Budget existing;
    bool is_update = hashmap_search(engine->budget_map, category, &existing);
    
    if (is_update) {
        UndoAction action;
        action.type = ACTION_UPDATE_BUDGET;
        snprintf(action.data, sizeof(action.data), "%s|%.2f", category, existing.limit);
        undo_stack_push(engine->undo_stack, &action);
        
        // Update in indexed priority queue
        ipq_update_priority(engine->budget_alerts_pq, category, budget.spent);
    } else {
        UndoAction action;
        action.type = ACTION_ADD_BUDGET;
        snprintf(action.data, sizeof(action.data), "%s|%.2f", category, limit);
        undo_stack_push(engine->undo_stack, &action);
        
        // Insert into indexed priority queue
        ipq_insert(engine->budget_alerts_pq, category, budget.spent, limit);
    }
    
    // Insert/update in HashMap: O(1) average
    hashmap_insert(engine->budget_map, category, &budget);
    trie_insert(engine->category_trie, category);
    
    update_stats(engine);
    return true;
}

bool engine_get_budget(FinanceEngine* engine, const char* category, Budget* out) {
    if (!engine || !category) return false;
    
    // Use HashMap for O(1) average lookup
    bool result = hashmap_search(engine->budget_map, category, out);
    update_stats(engine);
    return result;
}

int engine_get_all_budgets(FinanceEngine* engine, Budget* out, int max_count) {
    if (!engine || !out) return 0;
    
    int count = hashmap_get_all(engine->budget_map, out, max_count);
    update_stats(engine);
    return count;
}

int engine_get_budget_alerts(FinanceEngine* engine, Budget* out, int max_count) {
    if (!engine || !out) return 0;
    
    // Get alerts from Indexed Priority Queue sorted by priority (usage %)
    BudgetAlert alerts[MAX_BUDGETS];
    int alert_count = ipq_get_alerts_above_threshold(engine->budget_alerts_pq, 50.0, alerts, max_count);
    
    // Convert BudgetAlert to Budget
    for (int i = 0; i < alert_count; i++) {
        safe_strcpy(out[i].category, alerts[i].category, MAX_STRING_LEN);
        out[i].spent = alerts[i].spent;
        out[i].limit = alerts[i].budget_limit;
    }
    
    update_stats(engine);
    return alert_count;
}

// ==================== BILL OPERATIONS ====================

const char* engine_add_bill(FinanceEngine* engine, const char* name, double amount,
                            const char* due_date, const char* category) {
    if (!engine || !name || !due_date || !category) return NULL;
    
    const char* id = generate_bill_id(engine);
    
    Bill bill;
    safe_strcpy(bill.id, id, sizeof(bill.id));
    safe_strcpy(bill.name, name, sizeof(bill.name));
    bill.amount = amount;
    safe_strcpy(bill.due_date, due_date, sizeof(bill.due_date));
    safe_strcpy(bill.category, category, sizeof(bill.category));
    bill.is_paid = false;
    
    // Queue: O(1) enqueue
    queue_enqueue(engine->bill_queue, &bill);
    
    // Record for undo
    UndoAction action;
    action.type = ACTION_ADD_BILL;
    snprintf(action.data, sizeof(action.data), "%s|%s|%.2f|%s|%s",
             bill.id, bill.name, bill.amount, bill.due_date, bill.category);
    undo_stack_push(engine->undo_stack, &action);
    
    update_stats(engine);
    return bill_id_buffer;
}

int engine_get_all_bills(FinanceEngine* engine, Bill* out, int max_count) {
    if (!engine || !out) return 0;
    
    int count = queue_get_all_bills(engine->bill_queue, out, max_count);
    update_stats(engine);
    return count;
}

bool engine_pay_bill(FinanceEngine* engine, const char* id) {
    if (!engine || !id) return false;
    
    UndoAction action;
    action.type = ACTION_PAY_BILL;
    safe_strcpy(action.data, id, sizeof(action.data));
    undo_stack_push(engine->undo_stack, &action);
    
    // Queue: O(n) search to find and mark as paid
    bool result = queue_mark_as_paid(engine->bill_queue, id);
    update_stats(engine);
    return result;
}

bool engine_delete_bill(FinanceEngine* engine, const char* id) {
    if (!engine || !id) return false;
    
    Bill bill;
    if (queue_find_by_id(engine->bill_queue, id, &bill)) {
        UndoAction action;
        action.type = ACTION_DELETE_BILL;
        snprintf(action.data, sizeof(action.data), "%s|%s|%.2f|%s|%s",
                 bill.id, bill.name, bill.amount, bill.due_date, bill.category);
        undo_stack_push(engine->undo_stack, &action);
    }
    
    // Queue: O(n) search to remove
    bool result = queue_remove_by_id(engine->bill_queue, id);
    update_stats(engine);
    return result;
}

// ==================== ANALYTICS ====================

int engine_get_top_expenses(FinanceEngine* engine, int k, Transaction* out) {
    if (!engine || !out || k <= 0) return 0;
    
    // Get all transactions from Skip List
    Transaction all[MAX_TRANSACTIONS];
    int total = skiplist_get_all(engine->transaction_skiplist, all, MAX_TRANSACTIONS);
    
    // Filter expenses only
    Transaction expenses[MAX_TRANSACTIONS];
    int expense_count = 0;
    for (int i = 0; i < total; i++) {
        if (strcmp(all[i].type, "expense") == 0) {
            memcpy(&expenses[expense_count++], &all[i], sizeof(Transaction));
        }
    }
    
    // Use IntroSort to get top K expenses: O(n log n)
    int count = introsort_get_top_k_expenses(expenses, expense_count, k, out, &engine->sort_stats);
    
    update_stats(engine);
    return count;
}

int engine_get_top_categories(FinanceEngine* engine, int k, CategoryAmount* out) {
    if (!engine || !out || k <= 0) return 0;
    
    // Get all expenses from expense map
    Budget expenses[MAX_CATEGORIES];
    int expense_count = hashmap_get_all(engine->expense_map, expenses, MAX_CATEGORIES);
    
    // Convert to CategoryAmount
    CategoryAmount categories[MAX_CATEGORIES];
    int cat_count = 0;
    
    for (int i = 0; i < expense_count; i++) {
        if (expenses[i].spent > 0) {
            safe_strcpy(categories[cat_count].category, expenses[i].category, MAX_STRING_LEN);
            categories[cat_count].total_amount = expenses[i].spent;
            cat_count++;
        }
    }
    
    // Use IntroSort to get top K categories: O(n log n)
    int count = introsort_get_top_k_categories(categories, cat_count, k, out, &engine->sort_stats);
    
    update_stats(engine);
    return count;
}

// ==================== SPENDING TRENDS (Sliding Window) ====================

bool engine_get_spending_trend_7day(FinanceEngine* engine, TrendResult* result) {
    if (!engine || !result) return false;
    
    // Get all transactions
    Transaction all[MAX_TRANSACTIONS];
    int total = rbtree_inorder_traversal(engine->transaction_tree, all, MAX_TRANSACTIONS);
    
    // Get current date as end date
    char end_date[16];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(end_date, sizeof(end_date), "%Y-%m-%d", tm_info);
    
    // Build 7-day sliding window
    sliding_window_clear(engine->spending_window_7day);
    sliding_window_build_from_transactions(engine->spending_window_7day, all, total, end_date);
    
    // Get trend result
    bool success = sliding_window_get_trend(engine->spending_window_7day, result);
    
    update_stats(engine);
    return success;
}

bool engine_get_spending_trend_30day(FinanceEngine* engine, TrendResult* result) {
    if (!engine || !result) return false;
    
    // Get all transactions
    Transaction all[MAX_TRANSACTIONS];
    int total = rbtree_inorder_traversal(engine->transaction_tree, all, MAX_TRANSACTIONS);
    
    // Get current date as end date
    char end_date[16];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(end_date, sizeof(end_date), "%Y-%m-%d", tm_info);
    
    // Build 30-day sliding window
    sliding_window_clear(engine->spending_window_30day);
    sliding_window_build_from_transactions(engine->spending_window_30day, all, total, end_date);
    
    // Get trend result
    bool success = sliding_window_get_trend(engine->spending_window_30day, result);
    
    update_stats(engine);
    return success;
}

bool engine_get_spending_trend_custom(FinanceEngine* engine, int days, TrendResult* result) {
    if (!engine || !result || days <= 0) return false;
    
    // Get all transactions
    Transaction all[MAX_TRANSACTIONS];
    int total = rbtree_inorder_traversal(engine->transaction_tree, all, MAX_TRANSACTIONS);
    
    // Get current date as end date
    char end_date[16];
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    strftime(end_date, sizeof(end_date), "%Y-%m-%d", tm_info);
    
    // Calculate start date
    char start_date[16];
    time_t start_time = now - (days * 24 * 60 * 60);
    struct tm* start_tm = localtime(&start_time);
    strftime(start_date, sizeof(start_date), "%Y-%m-%d", start_tm);
    
    // Calculate trend for custom range
    sliding_window_calc_trend(all, total, start_date, end_date, result);
    
    update_stats(engine);
    return true;
}

// ==================== ANOMALY DETECTION (Z-Score/Welford's) ====================

bool engine_check_transaction_anomaly(FinanceEngine* engine, double amount, 
                                      const char* category, AnomalyResult* result) {
    if (!engine || !result) return false;
    
    // First check overall expense anomaly
    bool is_anomaly = zscore_check_expense_anomaly(engine->anomaly_tracker, amount, result);
    
    // Also check category-specific anomaly
    if (!is_anomaly && category && category[0] != '\0') {
        AnomalyResult cat_result;
        if (zscore_check_category_anomaly(engine->anomaly_tracker, category, amount, &cat_result)) {
            memcpy(result, &cat_result, sizeof(AnomalyResult));
            is_anomaly = true;
        }
    }
    
    update_stats(engine);
    return is_anomaly;
}

bool engine_get_spending_stats(FinanceEngine* engine, double* mean, double* std_dev,
                               double* min, double* max, int* count) {
    if (!engine) return false;
    
    // Use Z-Score tracker to get stats: O(1)
    zscore_get_expense_stats(engine->anomaly_tracker, mean, std_dev, min, max, count);
    
    update_stats(engine);
    return true;
}

// ==================== AUTOCOMPLETE (Trie) ====================

int engine_get_category_suggestions(FinanceEngine* engine, const char* prefix,
                                    char** out, int max_count) {
    if (!engine || !out) return 0;
    
    // Use Trie for O(m + k) prefix search
    int count = trie_get_words_with_prefix(engine->category_trie, prefix ? prefix : "", out, max_count);
    update_stats(engine);
    return count;
}

int engine_get_all_categories(FinanceEngine* engine, char** out, int max_count) {
    if (!engine || !out) return 0;
    
    int count = trie_get_all_words(engine->category_trie, out, max_count);
    update_stats(engine);
    return count;
}

// ==================== UNDO (Stack) ====================

bool engine_undo(FinanceEngine* engine) {
    if (!engine) return false;
    
    UndoAction action;
    if (!undo_stack_pop(engine->undo_stack, &action)) {
        return false;
    }
    
    char parts[6][MAX_STRING_LEN];
    int part_count = 0;
    char temp[512];
    safe_strcpy(temp, action.data, sizeof(temp));
    
    char* token = strtok(temp, "|");
    while (token && part_count < 6) {
        safe_strcpy(parts[part_count], token, MAX_STRING_LEN);
        part_count++;
        token = strtok(NULL, "|");
    }
    
    switch (action.type) {
        case ACTION_ADD_TRANSACTION: {
            // Undo add = delete (without adding to undo stack again)
            skiplist_delete(engine->transaction_skiplist, parts[0]);
            rbtree_delete_by_id(engine->transaction_tree, parts[0]);
            break;
        }
        case ACTION_DELETE_TRANSACTION: {
            // Undo delete = add back
            Transaction t;
            safe_strcpy(t.id, parts[0], sizeof(t.id));
            safe_strcpy(t.type, parts[1], sizeof(t.type));
            t.amount = atof(parts[2]);
            safe_strcpy(t.category, parts[3], sizeof(t.category));
            safe_strcpy(t.description, parts[4], sizeof(t.description));
            safe_strcpy(t.date, parts[5], sizeof(t.date));
            
            rbtree_insert(engine->transaction_tree, &t);
            skiplist_insert(engine->transaction_skiplist, &t);
            update_expense_tracking(engine, &t, true);
            break;
        }
        case ACTION_ADD_BUDGET: {
            hashmap_remove(engine->budget_map, parts[0]);
            ipq_remove(engine->budget_alerts_pq, parts[0]);
            break;
        }
        case ACTION_UPDATE_BUDGET: {
            Budget budget;
            if (hashmap_search(engine->budget_map, parts[0], &budget)) {
                budget.limit = atof(parts[1]);
                hashmap_update(engine->budget_map, parts[0], &budget);
            }
            break;
        }
        default:
            break;
    }
    
    update_stats(engine);
    return true;
}

bool engine_can_undo(FinanceEngine* engine) {
    return engine && !undo_stack_is_empty(engine->undo_stack);
}

// ==================== STATISTICS ====================

double engine_get_total_balance(FinanceEngine* engine) {
    if (!engine) return 0;
    
    Transaction transactions[MAX_TRANSACTIONS];
    int count = skiplist_get_all(engine->transaction_skiplist, transactions, MAX_TRANSACTIONS);
    
    double balance = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(transactions[i].type, "income") == 0) {
            balance += transactions[i].amount;
        } else {
            balance -= transactions[i].amount;
        }
    }
    
    return balance;
}

double engine_get_total_income(FinanceEngine* engine) {
    if (!engine) return 0;
    
    Transaction transactions[MAX_TRANSACTIONS];
    int count = skiplist_get_all(engine->transaction_skiplist, transactions, MAX_TRANSACTIONS);
    
    double total = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(transactions[i].type, "income") == 0) {
            total += transactions[i].amount;
        }
    }
    return total;
}

double engine_get_total_expenses(FinanceEngine* engine) {
    if (!engine) return 0;
    
    Transaction transactions[MAX_TRANSACTIONS];
    int count = skiplist_get_all(engine->transaction_skiplist, transactions, MAX_TRANSACTIONS);
    
    double total = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(transactions[i].type, "expense") == 0) {
            total += transactions[i].amount;
        }
    }
    return total;
}

int engine_get_transaction_count(FinanceEngine* engine) {
    return engine ? skiplist_size(engine->transaction_skiplist) : 0;
}

int engine_get_budget_count(FinanceEngine* engine) {
    return engine ? hashmap_size(engine->budget_map) : 0;
}

int engine_get_bill_count(FinanceEngine* engine) {
    return engine ? queue_size(engine->bill_queue) : 0;
}

// ==================== DSA STATS ====================

void engine_get_dsa_stats(FinanceEngine* engine, DSAStats* stats) {
    if (!engine || !stats) return;
    update_stats(engine);
    memcpy(stats, &engine->stats, sizeof(DSAStats));
}

void engine_reset_stats(FinanceEngine* engine) {
    if (!engine) return;
    memset(&engine->stats, 0, sizeof(DSAStats));
    introsort_reset_stats(&engine->sort_stats);
}

// ==================== DATA LOADING ====================

void engine_load_transaction(FinanceEngine* engine, const char* id, const char* type,
                             double amount, const char* category,
                             const char* description, const char* date) {
    if (!engine || !id || !type || !category) return;
    
    Transaction t;
    safe_strcpy(t.id, id, sizeof(t.id));
    safe_strcpy(t.type, type, sizeof(t.type));
    t.amount = amount;
    safe_strcpy(t.category, category, sizeof(t.category));
    safe_strcpy(t.description, description ? description : "", sizeof(t.description));
    safe_strcpy(t.date, date ? date : "", sizeof(t.date));
    
    // Add to all data structures
    rbtree_insert(engine->transaction_tree, &t);
    skiplist_insert(engine->transaction_skiplist, &t);
    
    // Update expense tracking and Z-Score
    update_expense_tracking(engine, &t, true);
    
    // Update income stats
    if (strcmp(type, "income") == 0) {
        zscore_update_income(engine->anomaly_tracker, amount);
    }
    
    // Add to tries
    trie_insert(engine->category_trie, category);
    if (description && description[0] != '\0') {
        trie_insert(engine->payee_trie, description);
    }
}

void engine_load_budget(FinanceEngine* engine, const char* category, double limit) {
    if (!engine || !category) return;
    
    Budget budget;
    safe_strcpy(budget.category, category, MAX_STRING_LEN);
    budget.limit = limit;
    
    // Get current spending from expense map
    Budget expense;
    if (hashmap_search(engine->expense_map, category, &expense)) {
        budget.spent = expense.spent;
    } else {
        budget.spent = 0;
    }
    
    // Insert into HashMap and Indexed Priority Queue
    hashmap_insert(engine->budget_map, category, &budget);
    ipq_insert(engine->budget_alerts_pq, category, budget.spent, limit);
    trie_insert(engine->category_trie, category);
}

void engine_load_bill(FinanceEngine* engine, const char* id, const char* name,
                      double amount, const char* due_date, const char* category, bool is_paid) {
    if (!engine || !id || !name) return;
    
    Bill bill;
    safe_strcpy(bill.id, id, sizeof(bill.id));
    safe_strcpy(bill.name, name, sizeof(bill.name));
    bill.amount = amount;
    safe_strcpy(bill.due_date, due_date ? due_date : "", sizeof(bill.due_date));
    safe_strcpy(bill.category, category ? category : "", sizeof(bill.category));
    bill.is_paid = is_paid;
    
    queue_enqueue(engine->bill_queue, &bill);
}
