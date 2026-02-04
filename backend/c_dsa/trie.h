/**
 * Trie Implementation for Category/Payee Autocomplete
 * Data Structures & Applications Lab Project - Pure C Implementation
 * Operations: insert O(m), search O(m), prefix search O(m + k)
 */

#ifndef TRIE_H
#define TRIE_H

#include "common.h"

#define ALPHABET_SIZE 128  // ASCII characters
#define MAX_WORD_LENGTH 256

// Trie Node
typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool is_end_of_word;
    char word[MAX_STRING_LEN];  // Store complete word at end node
} TrieNode;

// Trie structure
typedef struct {
    TrieNode* root;
    int word_count;
    int operations_count;
} Trie;

// Function declarations
Trie* trie_create(void);
void trie_destroy(Trie* trie);
bool trie_insert(Trie* trie, const char* word);
bool trie_search(Trie* trie, const char* word);
bool trie_starts_with(Trie* trie, const char* prefix);
int trie_get_words_with_prefix(Trie* trie, const char* prefix, char** out_words, int max_results);
bool trie_remove(Trie* trie, const char* word);
int trie_get_all_words(Trie* trie, char** out_words, int max_results);
int trie_size(Trie* trie);
bool trie_is_empty(Trie* trie);
void trie_clear(Trie* trie);
int trie_get_operations_count(Trie* trie);

#endif // TRIE_H
