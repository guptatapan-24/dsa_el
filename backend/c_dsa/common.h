/**
 * Common definitions and structures for Finance Tracker DSA
 * Data Structures & Applications Lab Project - Pure C Implementation
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define MAX_STRING_LEN 256
#define TABLE_SIZE 100
#define MAX_TRIE_CHILDREN 128

// Transaction structure
typedef struct {
    char id[64];
    char type[16];        // "income" or "expense"
    double amount;
    char category[MAX_STRING_LEN];
    char description[MAX_STRING_LEN];
    char date[16];        // YYYY-MM-DD
} Transaction;

// Budget structure
typedef struct {
    char category[MAX_STRING_LEN];
    double limit;
    double spent;
} Budget;

// Bill structure
typedef struct {
    char id[64];
    char name[MAX_STRING_LEN];
    double amount;
    char due_date[16];
    char category[MAX_STRING_LEN];
    bool is_paid;
} Bill;

// Category amount for analytics
typedef struct {
    char category[MAX_STRING_LEN];
    double total_amount;
} CategoryAmount;

// Undo action types
typedef enum {
    ACTION_ADD_TRANSACTION = 0,
    ACTION_DELETE_TRANSACTION = 1,
    ACTION_ADD_BUDGET = 2,
    ACTION_UPDATE_BUDGET = 3,
    ACTION_ADD_BILL = 4,
    ACTION_DELETE_BILL = 5,
    ACTION_PAY_BILL = 6
} ActionType;

// Undo action structure
typedef struct {
    ActionType type;
    char data[512];
} UndoAction;

// Helper function to copy strings safely
static inline void safe_strcpy(char* dest, const char* src, size_t dest_size) {
    if (dest && src && dest_size > 0) {
        strncpy(dest, src, dest_size - 1);
        dest[dest_size - 1] = '\0';
    }
}

// Helper to create a copy of transaction
static inline Transaction* transaction_copy(const Transaction* t) {
    Transaction* copy = (Transaction*)malloc(sizeof(Transaction));
    if (copy && t) {
        memcpy(copy, t, sizeof(Transaction));
    }
    return copy;
}

#endif // COMMON_H
