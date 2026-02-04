/**
 * Trie Implementation - Pure C
 * Used for autocomplete functionality
 */

#include "trie.h"
#include <ctype.h>

// Create a new trie node
static TrieNode* create_node(void) {
    TrieNode* node = (TrieNode*)malloc(sizeof(TrieNode));
    if (node) {
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            node->children[i] = NULL;
        }
        node->is_end_of_word = false;
        node->word[0] = '\0';
    }
    return node;
}

// Recursively free nodes
static void free_node(TrieNode* node) {
    if (!node) return;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) {
            free_node(node->children[i]);
        }
    }
    free(node);
}

// Convert to lowercase
static char to_lower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 'a';
    }
    return c;
}

// Create new trie
Trie* trie_create(void) {
    Trie* trie = (Trie*)malloc(sizeof(Trie));
    if (trie) {
        trie->root = create_node();
        trie->word_count = 0;
        trie->operations_count = 0;
    }
    return trie;
}

// Destroy trie
void trie_destroy(Trie* trie) {
    if (!trie) return;
    free_node(trie->root);
    free(trie);
}

// Insert a word
// Time Complexity: O(m) where m is word length
bool trie_insert(Trie* trie, const char* word) {
    if (!trie || !word || word[0] == '\0') return false;
    
    trie->operations_count++;
    TrieNode* current = trie->root;
    
    for (int i = 0; word[i] != '\0'; i++) {
        int index = (unsigned char)to_lower(word[i]);
        if (index < 0 || index >= ALPHABET_SIZE) {
            continue;  // Skip invalid characters
        }
        
        if (!current->children[index]) {
            current->children[index] = create_node();
            if (!current->children[index]) return false;
        }
        current = current->children[index];
    }
    
    if (!current->is_end_of_word) {
        current->is_end_of_word = true;
        safe_strcpy(current->word, word, MAX_STRING_LEN);  // Store original case
        trie->word_count++;
    }
    
    return true;
}

// Search for exact word
// Time Complexity: O(m)
bool trie_search(Trie* trie, const char* word) {
    if (!trie || !word) return false;
    
    trie->operations_count++;
    TrieNode* current = trie->root;
    
    for (int i = 0; word[i] != '\0'; i++) {
        int index = (unsigned char)to_lower(word[i]);
        if (index < 0 || index >= ALPHABET_SIZE) {
            continue;
        }
        
        if (!current->children[index]) {
            return false;
        }
        current = current->children[index];
    }
    
    return current->is_end_of_word;
}

// Check if any word starts with prefix
// Time Complexity: O(m)
bool trie_starts_with(Trie* trie, const char* prefix) {
    if (!trie || !prefix) return false;
    
    trie->operations_count++;
    TrieNode* current = trie->root;
    
    for (int i = 0; prefix[i] != '\0'; i++) {
        int index = (unsigned char)to_lower(prefix[i]);
        if (index < 0 || index >= ALPHABET_SIZE) {
            continue;
        }
        
        if (!current->children[index]) {
            return false;
        }
        current = current->children[index];
    }
    
    return true;
}

// Collect all words from a node
static void collect_words(TrieNode* node, char** words, int* count, int max_results) {
    if (!node || *count >= max_results) return;
    
    if (node->is_end_of_word) {
        words[*count] = (char*)malloc(MAX_STRING_LEN);
        if (words[*count]) {
            safe_strcpy(words[*count], node->word, MAX_STRING_LEN);
            (*count)++;
        }
    }
    
    for (int i = 0; i < ALPHABET_SIZE && *count < max_results; i++) {
        if (node->children[i]) {
            collect_words(node->children[i], words, count, max_results);
        }
    }
}

// Get words with prefix (autocomplete)
// Time Complexity: O(m + k) where m is prefix length, k is number of results
int trie_get_words_with_prefix(Trie* trie, const char* prefix, char** out_words, int max_results) {
    if (!trie || !out_words) return 0;
    
    trie->operations_count++;
    
    // If prefix is empty, get all words
    if (!prefix || prefix[0] == '\0') {
        int count = 0;
        collect_words(trie->root, out_words, &count, max_results);
        return count;
    }
    
    // Navigate to end of prefix
    TrieNode* current = trie->root;
    for (int i = 0; prefix[i] != '\0'; i++) {
        int index = (unsigned char)to_lower(prefix[i]);
        if (index < 0 || index >= ALPHABET_SIZE) {
            continue;
        }
        
        if (!current->children[index]) {
            return 0;  // No words with this prefix
        }
        current = current->children[index];
    }
    
    // Collect all words from this point
    int count = 0;
    collect_words(current, out_words, &count, max_results);
    return count;
}

// Helper to check if node has children
static bool has_children(TrieNode* node) {
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (node->children[i]) return true;
    }
    return false;
}

// Remove a word from trie
// Time Complexity: O(m)
bool trie_remove(Trie* trie, const char* word) {
    if (!trie || !word) return false;
    
    trie->operations_count++;
    TrieNode* current = trie->root;
    
    for (int i = 0; word[i] != '\0'; i++) {
        int index = (unsigned char)to_lower(word[i]);
        if (index < 0 || index >= ALPHABET_SIZE) {
            continue;
        }
        
        if (!current->children[index]) {
            return false;
        }
        current = current->children[index];
    }
    
    if (current->is_end_of_word) {
        current->is_end_of_word = false;
        current->word[0] = '\0';
        trie->word_count--;
        return true;
    }
    
    return false;
}

// Get all words
int trie_get_all_words(Trie* trie, char** out_words, int max_results) {
    if (!trie || !out_words) return 0;
    
    int count = 0;
    collect_words(trie->root, out_words, &count, max_results);
    return count;
}

int trie_size(Trie* trie) {
    return trie ? trie->word_count : 0;
}

bool trie_is_empty(Trie* trie) {
    return trie ? trie->word_count == 0 : true;
}

void trie_clear(Trie* trie) {
    if (!trie) return;
    
    free_node(trie->root);
    trie->root = create_node();
    trie->word_count = 0;
}

int trie_get_operations_count(Trie* trie) {
    return trie ? trie->operations_count : 0;
}
