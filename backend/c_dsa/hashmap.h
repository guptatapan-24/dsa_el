/**
 * Hash Map Implementation for Category → Budget Mapping
 * Data Structures & Applications Lab Project - Pure C Implementation
 * Operations: insert, update, search, remove - O(1) average
 */

#ifndef HASHMAP_H
#define HASHMAP_H

#include "common.h"

// Hash node for chaining
typedef struct HashNode {
    char key[MAX_STRING_LEN];
    Budget value;
    struct HashNode* next;
} HashNode;

// HashMap structure
typedef struct {
    HashNode* table[TABLE_SIZE];
    int count;
    int operations_count;  // Track operations for proof
} HashMap;

// Function declarations
HashMap* hashmap_create(void);
void hashmap_destroy(HashMap* map);
unsigned int hashmap_hash(const char* key);
bool hashmap_insert(HashMap* map, const char* key, const Budget* value);
bool hashmap_search(HashMap* map, const char* key, Budget* out_value);
bool hashmap_update(HashMap* map, const char* key, const Budget* value);
bool hashmap_remove(HashMap* map, const char* key);
bool hashmap_contains(HashMap* map, const char* key);
int hashmap_size(HashMap* map);
int hashmap_get_all(HashMap* map, Budget* out_budgets, int max_count);
int hashmap_get_operations_count(HashMap* map);

#endif // HASHMAP_H
