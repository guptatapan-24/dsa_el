/**
 * Finance Engine Implementation - Pure C
 * Integrates all 7 data structures with operation tracking
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

// Update DSA stats
static void update_stats(FinanceEngine* engine) {
    if (!engine) return;
    
    engine->stats.hashmap_ops = hashmap_get_operations_count(engine->budget_map) +
                                hashmap_get_operations_count(engine->expense_map);
    engine->stats.linkedlist_ops = dll_get_operations_count(engine->transactions);
    engine->stats.bst_ops = bst_get_operations_count(engine->transaction_bst);
    engine->stats.heap_ops = maxheap_get_operations_count(engine->expense_heap) +
                             cat_maxheap_get_operations_count(engine->category_heap);
    engine->stats.queue_ops = queue_get_operations_count(engine->bill_queue);
    engine->stats.stack_ops = undo_stack_get_operations_count(engine->undo_stack) +
                              trans_stack_get_operations_count(engine->recent_stack);
    engine->stats.trie_ops = trie_get_operations_count(engine->category_trie) +
                             trie_get_operations_count(engine->payee_trie);
    
    engine->stats.total_ops = engine->stats.hashmap_ops +
                              engine->stats.linkedlist_ops +
                              engine->stats.bst_ops +
                              engine->stats.heap_ops +
                              engine->stats.queue_ops +
                              engine->stats.stack_ops +
                              engine->stats.trie_ops;
}

// Create new finance engine
FinanceEngine* engine_create(void) {
    FinanceEngine* engine = (FinanceEngine*)malloc(sizeof(FinanceEngine));
    if (!engine) return NULL;
    
    // Initialize all data structures
    engine->budget_map = hashmap_create();
    engine->expense_map = hashmap_create();
    engine->transactions = dll_create();
    engine->transaction_bst = bst_create();
    engine->expense_heap = maxheap_create(MAX_TRANSACTIONS);
    engine->category_heap = cat_maxheap_create(MAX_CATEGORIES);
    engine->bill_queue = queue_create();
    engine->undo_stack = undo_stack_create(50);
    engine->recent_stack = trans_stack_create(100);
    engine->category_trie = trie_create();
    engine->payee_trie = trie_create();
    
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

// Destroy finance engine
void engine_destroy(FinanceEngine* engine) {
    if (!engine) return;
    
    hashmap_destroy(engine->budget_map);
    hashmap_destroy(engine->expense_map);
    dll_destroy(engine->transactions);
    bst_destroy(engine->transaction_bst);
    maxheap_destroy(engine->expense_heap);
    cat_maxheap_destroy(engine->category_heap);
    queue_destroy(engine->bill_queue);
    undo_stack_destroy(engine->undo_stack);
    trans_stack_destroy(engine->recent_stack);
    trie_destroy(engine->category_trie);
    trie_destroy(engine->payee_trie);
    
    free(engine);
}

// Update expense tracking after transaction
static void update_expense_tracking(FinanceEngine* engine, const Transaction* t, bool is_add) {
    if (strcmp(t->type, "expense") != 0) return;
    
    Budget current_budget;
    memset(&current_budget, 0, sizeof(Budget));
    safe_strcpy(current_budget.category, t->category, MAX_STRING_LEN);
    
    // Get current expense total
    double current_total = 0;
    Budget temp_budget;
    if (hashmap_search(engine->expense_map, t->category, &temp_budget)) {
        current_total = temp_budget.spent;
    }
    
    // Update expense total
    if (is_add) {
        current_budget.spent = current_total + t->amount;
    } else {
        current_budget.spent = current_total > t->amount ? current_total - t->amount : 0;
    }
    
    hashmap_insert(engine->expense_map, t->category, &current_budget);
    
    // Update budget spent if exists
    Budget budget;
    if (hashmap_search(engine->budget_map, t->category, &budget)) {
        budget.spent = current_budget.spent;
        hashmap_update(engine->budget_map, t->category, &budget);
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
    dll_add_front(engine->transactions, &t);     // LinkedList: O(1)
    bst_insert(engine->transaction_bst, &t);     // BST: O(log n)
    trans_stack_push(engine->recent_stack, &t);  // Stack: O(1)
    
    // Update expense tracking (uses HashMap)
    update_expense_tracking(engine, &t, true);
    
    // Add to heap if expense
    if (strcmp(type, "expense") == 0) {
        maxheap_insert(engine->expense_heap, &t); // Heap: O(log n)
    }
    
    // Add to tries (Trie: O(m))
    trie_insert(engine->category_trie, category);
    if (description && description[0] != '\0') {
        trie_insert(engine->payee_trie, description);
    }
    
    // Record for undo (Stack)
    UndoAction action;
    action.type = ACTION_ADD_TRANSACTION;
    snprintf(action.data, sizeof(action.data), "%s|%s|%.2f|%s|%s|%s",
             t.id, t.type, t.amount, t.category, t.description, t.date);
    undo_stack_push(engine->undo_stack, &action);
    
    update_stats(engine);
    return id_buffer;  // Return the ID
}

bool engine_delete_transaction(FinanceEngine* engine, const char* id) {
    if (!engine || !id) return false;
    
    // Find transaction first (BST search)
    Transaction t;
    if (!bst_find_by_id(engine->transaction_bst, id, &t)) {
        return false;
    }
    
    // Record for undo
    UndoAction action;
    action.type = ACTION_DELETE_TRANSACTION;
    snprintf(action.data, sizeof(action.data), "%s|%s|%.2f|%s|%s|%s",
             t.id, t.type, t.amount, t.category, t.description, t.date);
    undo_stack_push(engine->undo_stack, &action);
    
    // Delete from data structures
    dll_delete_by_id(engine->transactions, id);      // LinkedList: O(n)
    bst_delete_by_id(engine->transaction_bst, id);   // BST: O(n) for ID search
    
    // Update expense tracking
    update_expense_tracking(engine, &t, false);
    
    update_stats(engine);
    return true;
}

int engine_get_all_transactions(FinanceEngine* engine, Transaction* out, int max_count) {
    if (!engine || !out) return 0;
    int count = dll_traverse_forward(engine->transactions, out, max_count);
    update_stats(engine);
    return count;
}

int engine_get_transactions_desc(FinanceEngine* engine, Transaction* out, int max_count) {
    if (!engine || !out) return 0;
    int count = bst_reverse_inorder(engine->transaction_bst, out, max_count);
    update_stats(engine);
    return count;
}

int engine_get_transactions_in_range(FinanceEngine* engine, const char* start,
                                     const char* end, Transaction* out, int max_count) {
    if (!engine || !start || !end || !out) return 0;
    int count = bst_range_query(engine->transaction_bst, start, end, out, max_count);
    update_stats(engine);
    return count;
}

int engine_get_recent_transactions(FinanceEngine* engine, int count, Transaction* out) {
    if (!engine || !out) return 0;
    int result = trans_stack_get_top_n(engine->recent_stack, count, out);
    update_stats(engine);
    return result;
}

bool engine_find_transaction(FinanceEngine* engine, const char* id, Transaction* out) {
    if (!engine || !id) return false;
    bool result = bst_find_by_id(engine->transaction_bst, id, out);
    update_stats(engine);
    return result;
}

// ==================== BUDGET OPERATIONS ====================

bool engine_set_budget(FinanceEngine* engine, const char* category, double limit) {
    if (!engine || !category) return false;
    
    Budget budget;
    safe_strcpy(budget.category, category, MAX_STRING_LEN);
    budget.limit = limit;
    
    // Get current spending
    Budget expense;
    if (hashmap_search(engine->expense_map, category, &expense)) {
        budget.spent = expense.spent;
    } else {
        budget.spent = 0;
    }
    
    // Check if updating existing
    Budget existing;
    if (hashmap_search(engine->budget_map, category, &existing)) {
        UndoAction action;
        action.type = ACTION_UPDATE_BUDGET;
        snprintf(action.data, sizeof(action.data), "%s|%.2f", category, existing.limit);
        undo_stack_push(engine->undo_stack, &action);
    } else {
        UndoAction action;
        action.type = ACTION_ADD_BUDGET;
        snprintf(action.data, sizeof(action.data), "%s|%.2f", category, limit);
        undo_stack_push(engine->undo_stack, &action);
    }
    
    hashmap_insert(engine->budget_map, category, &budget);
    trie_insert(engine->category_trie, category);
    
    update_stats(engine);
    return true;
}

bool engine_get_budget(FinanceEngine* engine, const char* category, Budget* out) {
    if (!engine || !category) return false;
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
    
    Budget all_budgets[MAX_BUDGETS];
    int total = hashmap_get_all(engine->budget_map, all_budgets, MAX_BUDGETS);
    
    int alert_count = 0;
    for (int i = 0; i < total && alert_count < max_count; i++) {
        double percent = all_budgets[i].limit > 0 ? 
                        (all_budgets[i].spent / all_budgets[i].limit) * 100 : 0;
        if (percent >= 50) {  // 50% threshold for alerts
            memcpy(&out[alert_count], &all_budgets[i], sizeof(Budget));
            alert_count++;
        }
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
    
    queue_enqueue(engine->bill_queue, &bill);  // Queue: O(1)
    
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
    
    bool result = queue_remove_by_id(engine->bill_queue, id);
    update_stats(engine);
    return result;
}

// ==================== ANALYTICS ====================

int engine_get_top_expenses(FinanceEngine* engine, int k, Transaction* out) {
    if (!engine || !out || k <= 0) return 0;
    
    // Rebuild heap with current expenses
    Transaction expenses[MAX_TRANSACTIONS];
    int expense_count = dll_filter_by_type(engine->transactions, "expense", expenses, MAX_TRANSACTIONS);
    
    maxheap_clear(engine->expense_heap);
    maxheap_build(engine->expense_heap, expenses, expense_count);
    
    int count = maxheap_get_top_k(engine->expense_heap, k, out);
    update_stats(engine);
    return count;
}

int engine_get_top_categories(FinanceEngine* engine, int k, CategoryAmount* out) {
    if (!engine || !out || k <= 0) return 0;
    
    // Build category amounts from expense map
    Budget expenses[MAX_CATEGORIES];
    int expense_count = hashmap_get_all(engine->expense_map, expenses, MAX_CATEGORIES);
    
    CategoryAmount categories[MAX_CATEGORIES];
    int cat_count = 0;
    
    for (int i = 0; i < expense_count; i++) {
        if (expenses[i].spent > 0) {
            safe_strcpy(categories[cat_count].category, expenses[i].category, MAX_STRING_LEN);
            categories[cat_count].total_amount = expenses[i].spent;
            cat_count++;
        }
    }
    
    cat_maxheap_clear(engine->category_heap);
    cat_maxheap_build(engine->category_heap, categories, cat_count);
    
    int count = cat_maxheap_get_top_k(engine->category_heap, k, out);
    update_stats(engine);
    return count;
}

// ==================== AUTOCOMPLETE ====================

int engine_get_category_suggestions(FinanceEngine* engine, const char* prefix,
                                    char** out, int max_count) {
    if (!engine || !out) return 0;
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

// ==================== UNDO ====================

bool engine_undo(FinanceEngine* engine) {
    if (!engine) return false;
    
    UndoAction action;
    if (!undo_stack_pop(engine->undo_stack, &action)) {
        return false;
    }
    
    char parts[6][MAX_STRING_LEN];
    int part_count = 0;
    char* token = strtok(action.data, "|");
    while (token && part_count < 6) {
        safe_strcpy(parts[part_count], token, MAX_STRING_LEN);
        part_count++;
        token = strtok(NULL, "|");
    }
    
    switch (action.type) {
        case ACTION_ADD_TRANSACTION: {
            // Undo add = delete (without adding to undo stack again)
            dll_delete_by_id(engine->transactions, parts[0]);
            bst_delete_by_id(engine->transaction_bst, parts[0]);
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
            
            dll_add_front(engine->transactions, &t);
            bst_insert(engine->transaction_bst, &t);
            update_expense_tracking(engine, &t, true);
            break;
        }
        case ACTION_ADD_BUDGET: {
            hashmap_remove(engine->budget_map, parts[0]);
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
    int count = dll_traverse_forward(engine->transactions, transactions, MAX_TRANSACTIONS);
    
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
    int count = dll_filter_by_type(engine->transactions, "income", transactions, MAX_TRANSACTIONS);
    
    double total = 0;
    for (int i = 0; i < count; i++) {
        total += transactions[i].amount;
    }
    return total;
}

double engine_get_total_expenses(FinanceEngine* engine) {
    if (!engine) return 0;
    
    Transaction transactions[MAX_TRANSACTIONS];
    int count = dll_filter_by_type(engine->transactions, "expense", transactions, MAX_TRANSACTIONS);
    
    double total = 0;
    for (int i = 0; i < count; i++) {
        total += transactions[i].amount;
    }
    return total;
}

int engine_get_transaction_count(FinanceEngine* engine) {
    return engine ? dll_size(engine->transactions) : 0;
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
    dll_add_back(engine->transactions, &t);
    bst_insert(engine->transaction_bst, &t);
    trans_stack_push(engine->recent_stack, &t);
    update_expense_tracking(engine, &t, true);
    
    if (strcmp(type, "expense") == 0) {
        maxheap_insert(engine->expense_heap, &t);
    }
    
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
    
    hashmap_insert(engine->budget_map, category, &budget);
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
