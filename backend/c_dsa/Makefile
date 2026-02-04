# Makefile for Pure C Finance DSA Library
# Builds a shared library (.so) for Python ctypes integration

CC = gcc
CFLAGS = -Wall -Wextra -fPIC -O2 -std=c99
LDFLAGS = -shared
TARGET = libfinance_dsa.so

# Source files - All data structures
SRCS = hashmap.c linkedlist.c bst.c heap.c queue.c stack.c trie.c \
       rbtree.c skiplist.c introsort.c indexed_pq.c sliding_window.c zscore.c \
       finance_engine.c
OBJS = $(SRCS:.c=.o)

# Headers
HDRS = common.h hashmap.h linkedlist.h bst.h heap.h queue.h stack.h trie.h \
       rbtree.h skiplist.h introsort.h indexed_pq.h sliding_window.h zscore.h \
       finance_engine.h

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) -lm
	@echo "Built shared library: $(TARGET)"
	@echo "Data Structures included:"
	@echo "  1. Red-Black Tree (rbtree.c) - O(log n) transaction storage"
	@echo "  2. Skip List (skiplist.c) - O(log n) ID lookups"
	@echo "  3. IntroSort (introsort.c) - O(n log n) sorting"
	@echo "  4. Polynomial HashMap (hashmap.c) - O(1) budget storage"
	@echo "  5. Indexed Priority Queue (indexed_pq.c) - O(log n) alert sorting"
	@echo "  6. Sliding Window (sliding_window.c) - O(k) trend calculation"
	@echo "  7. Z-Score Welford (zscore.c) - O(1) anomaly detection"
	@echo "  8. Queue (queue.c) - O(1) bill FIFO"
	@echo "  9. Stack (stack.c) - O(1) undo operations"
	@echo " 10. Trie (trie.c) - O(m) autocomplete"

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Test program
test: $(TARGET)
	$(CC) -o test_engine test_engine.c -L. -lfinance_dsa -lm -Wl,-rpath,.
	./test_engine
