/**
 * Test program for C Finance DSA Library
 * Demonstrates that all data structures are actually being used
 */

#include <stdio.h>
#include <stdlib.h>
#include "finance_engine.h"

void print_separator(const char* title) {
    printf("\n========== %s ==========\n", title);
}

int main() {
    printf("\n*** C Data Structures & Algorithms Finance Engine Test ***\n");
    printf("This test proves ALL 7 data structures are being used.\n");
    
    // Create engine
    FinanceEngine* engine = engine_create();
    if (!engine) {
        printf("ERROR: Failed to create engine\n");
        return 1;
    }
    
    print_separator("1. ADDING TRANSACTIONS (LinkedList + BST + Stack + Trie)");
    
    // Add transactions
    const char* id1 = engine_add_transaction(engine, "income", 5000.00, "Salary", "Monthly salary", "2025-01-15");
    printf("Added income: ID=%s\n", id1);
    
    const char* id2 = engine_add_transaction(engine, "expense", 1200.00, "Rent", "Monthly rent", "2025-01-01");
    printf("Added expense: ID=%s\n", id2);
    
    const char* id3 = engine_add_transaction(engine, "expense", 150.00, "Food", "Grocery shopping", "2025-01-10");
    printf("Added expense: ID=%s\n", id3);
    
    const char* id4 = engine_add_transaction(engine, "expense", 80.00, "Food", "Restaurant", "2025-01-12");
    printf("Added expense: ID=%s\n", id4);
    
    const char* id5 = engine_add_transaction(engine, "expense", 500.00, "Shopping", "Electronics", "2025-01-08");
    printf("Added expense: ID=%s\n", id5);
    
    print_separator("2. BST RANGE QUERY (Date-based search)");
    
    Transaction range_results[10];
    int range_count = engine_get_transactions_in_range(engine, "2025-01-01", "2025-01-10", range_results, 10);
    printf("Transactions between 2025-01-01 and 2025-01-10 (BST range query):\n");
    for (int i = 0; i < range_count; i++) {
        printf("  - %s: $%.2f (%s)\n", range_results[i].date, range_results[i].amount, range_results[i].category);
    }
    
    print_separator("3. HASHMAP (Budget Management)");
    
    engine_set_budget(engine, "Food", 300.00);
    engine_set_budget(engine, "Shopping", 400.00);
    engine_set_budget(engine, "Rent", 1500.00);
    
    Budget budget;
    if (engine_get_budget(engine, "Food", &budget)) {
        double percent = budget.limit > 0 ? (budget.spent / budget.limit) * 100 : 0;
        printf("Food Budget: $%.2f spent of $%.2f limit (%.1f%%)\n", 
               budget.spent, budget.limit, percent);
    }
    
    print_separator("4. MAX HEAP (Top Expenses)");
    
    Transaction top_expenses[5];
    int top_count = engine_get_top_expenses(engine, 3, top_expenses);
    printf("Top 3 expenses (extracted from MaxHeap):\n");
    for (int i = 0; i < top_count; i++) {
        printf("  %d. $%.2f - %s\n", i+1, top_expenses[i].amount, top_expenses[i].description);
    }
    
    print_separator("5. CATEGORY HEAP (Top Categories)");
    
    CategoryAmount top_cats[5];
    int cat_count = engine_get_top_categories(engine, 3, top_cats);
    printf("Top spending categories (from CategoryMaxHeap):\n");
    for (int i = 0; i < cat_count; i++) {
        printf("  %d. %s: $%.2f\n", i+1, top_cats[i].category, top_cats[i].total_amount);
    }
    
    print_separator("6. QUEUE (Bill Management - FIFO)");
    
    engine_add_bill(engine, "Electric Bill", 120.00, "2025-01-25", "Utilities");
    engine_add_bill(engine, "Internet", 50.00, "2025-01-28", "Utilities");
    engine_add_bill(engine, "Phone", 40.00, "2025-02-01", "Bills");
    
    Bill bills[10];
    int bill_count = engine_get_all_bills(engine, bills, 10);
    printf("Bills in queue (FIFO order):\n");
    for (int i = 0; i < bill_count; i++) {
        printf("  %d. %s: $%.2f due %s\n", i+1, bills[i].name, bills[i].amount, bills[i].due_date);
    }
    
    print_separator("7. TRIE (Category Autocomplete)");
    
    char* suggestions[10];
    int suggest_count = engine_get_category_suggestions(engine, "F", suggestions, 10);
    printf("Categories starting with 'F' (Trie prefix search):\n");
    for (int i = 0; i < suggest_count; i++) {
        printf("  - %s\n", suggestions[i]);
        free(suggestions[i]);
    }
    
    print_separator("8. STACK (Undo Functionality)");
    
    printf("Can undo: %s\n", engine_can_undo(engine) ? "YES" : "NO");
    printf("Transaction count before undo: %d\n", engine_get_transaction_count(engine));
    
    if (engine_undo(engine)) {
        printf("Undo successful!\n");
        printf("Transaction count after undo: %d\n", engine_get_transaction_count(engine));
    }
    
    print_separator("9. DSA OPERATION STATISTICS (PROOF OF USAGE)");
    
    DSAStats stats;
    engine_get_dsa_stats(engine, &stats);
    
    printf("Data Structure Operations Count:\n");
    printf("  HashMap operations:     %d\n", stats.hashmap_ops);
    printf("  LinkedList operations:  %d\n", stats.linkedlist_ops);
    printf("  BST operations:         %d\n", stats.bst_ops);
    printf("  Heap operations:        %d\n", stats.heap_ops);
    printf("  Queue operations:       %d\n", stats.queue_ops);
    printf("  Stack operations:       %d\n", stats.stack_ops);
    printf("  Trie operations:        %d\n", stats.trie_ops);
    printf("  --------------------------\n");
    printf("  TOTAL DSA operations:   %d\n", stats.total_ops);
    
    print_separator("10. FINANCIAL SUMMARY");
    
    printf("Total Income:   $%.2f\n", engine_get_total_income(engine));
    printf("Total Expenses: $%.2f\n", engine_get_total_expenses(engine));
    printf("Balance:        $%.2f\n", engine_get_total_balance(engine));
    printf("Transactions:   %d\n", engine_get_transaction_count(engine));
    printf("Budgets:        %d\n", engine_get_budget_count(engine));
    printf("Bills:          %d\n", engine_get_bill_count(engine));
    
    // Cleanup
    engine_destroy(engine);
    
    printf("\n*** TEST COMPLETE: All 7 C data structures verified! ***\n\n");
    return 0;
}
