/**
 * Hash Map Implementation - Pure C
 * Uses polynomial rolling hash with chaining for collision resolution
 */

#include "hashmap.h"

// Create new hash map
HashMap* hashmap_create(void) {
    HashMap* map = (HashMap*)malloc(sizeof(HashMap));
    if (map) {
        for (int i = 0; i < TABLE_SIZE; i++) {
            map->table[i] = NULL;
        }
        map->count = 0;
        map->operations_count = 0;
    }
    return map;
}

// Destroy hash map and free memory
void hashmap_destroy(HashMap* map) {
    if (!map) return;
    
    for (int i = 0; i < TABLE_SIZE; i++) {
        HashNode* current = map->table[i];
        while (current) {
            HashNode* temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(map);
}

// Polynomial rolling hash function
// Time Complexity: O(m) where m is key length
unsigned int hashmap_hash(const char* key) {
    unsigned long hash_val = 0;
    int p = 31;
    unsigned long p_pow = 1;
    
    for (int i = 0; key[i] != '\0'; i++) {
        char c = key[i];
        // Convert to lowercase for case-insensitive hashing
        if (c >= 'A' && c <= 'Z') c = c - 'A' + 'a';
        hash_val = (hash_val + (c - 'a' + 1) * p_pow) % TABLE_SIZE;
        p_pow = (p_pow * p) % TABLE_SIZE;
    }
    return (unsigned int)hash_val;
}

// Insert or update key-value pair
// Time Complexity: O(1) average, O(n) worst case
bool hashmap_insert(HashMap* map, const char* key, const Budget* value) {
    if (!map || !key || !value) return false;
    
    map->operations_count++;
    unsigned int index = hashmap_hash(key);
    
    // Check if key exists, update if so
    HashNode* current = map->table[index];
    while (current) {
        if (strcmp(current->key, key) == 0) {
            memcpy(&current->value, value, sizeof(Budget));
            return true;
        }
        current = current->next;
    }
    
    // Key doesn't exist, insert new node at front
    HashNode* new_node = (HashNode*)malloc(sizeof(HashNode));
    if (!new_node) return false;
    
    safe_strcpy(new_node->key, key, MAX_STRING_LEN);
    memcpy(&new_node->value, value, sizeof(Budget));
    new_node->next = map->table[index];
    map->table[index] = new_node;
    map->count++;
    
    return true;
}

// Search for a key
// Time Complexity: O(1) average, O(n) worst case
bool hashmap_search(HashMap* map, const char* key, Budget* out_value) {
    if (!map || !key) return false;
    
    map->operations_count++;
    unsigned int index = hashmap_hash(key);
    HashNode* current = map->table[index];
    
    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (out_value) {
                memcpy(out_value, &current->value, sizeof(Budget));
            }
            return true;
        }
        current = current->next;
    }
    return false;
}

// Update value for existing key
// Time Complexity: O(1) average
bool hashmap_update(HashMap* map, const char* key, const Budget* value) {
    if (!map || !key || !value) return false;
    
    map->operations_count++;
    unsigned int index = hashmap_hash(key);
    HashNode* current = map->table[index];
    
    while (current) {
        if (strcmp(current->key, key) == 0) {
            memcpy(&current->value, value, sizeof(Budget));
            return true;
        }
        current = current->next;
    }
    return false;
}

// Remove a key
// Time Complexity: O(1) average, O(n) worst case
bool hashmap_remove(HashMap* map, const char* key) {
    if (!map || !key) return false;
    
    map->operations_count++;
    unsigned int index = hashmap_hash(key);
    HashNode* current = map->table[index];
    HashNode* prev = NULL;
    
    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                map->table[index] = current->next;
            }
            free(current);
            map->count--;
            return true;
        }
        prev = current;
        current = current->next;
    }
    return false;
}

// Check if key exists
bool hashmap_contains(HashMap* map, const char* key) {
    return hashmap_search(map, key, NULL);
}

// Get map size
int hashmap_size(HashMap* map) {
    return map ? map->count : 0;
}

// Get all budgets
int hashmap_get_all(HashMap* map, Budget* out_budgets, int max_count) {
    if (!map || !out_budgets) return 0;
    
    int count = 0;
    for (int i = 0; i < TABLE_SIZE && count < max_count; i++) {
        HashNode* current = map->table[i];
        while (current && count < max_count) {
            memcpy(&out_budgets[count], &current->value, sizeof(Budget));
            count++;
            current = current->next;
        }
    }
    return count;
}

// Get operations count for proof
int hashmap_get_operations_count(HashMap* map) {
    return map ? map->operations_count : 0;
}
