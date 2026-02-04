# Makefile for Pure C Finance DSA Library
# Builds a shared library (.so) for Python ctypes integration

CC = gcc
CFLAGS = -Wall -Wextra -fPIC -O2 -std=c99
LDFLAGS = -shared
TARGET = libfinance_dsa.so

# Source files
SRCS = hashmap.c linkedlist.c bst.c heap.c queue.c stack.c trie.c finance_engine.c
OBJS = $(SRCS:.c=.o)

# Headers
HDRS = common.h hashmap.h linkedlist.h bst.h heap.h queue.h stack.h trie.h finance_engine.h

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) -lm
	@echo "Built shared library: $(TARGET)"
	@echo "Data Structures included:"
	@echo "  1. HashMap (hashmap.c) - O(1) category-budget mapping"
	@echo "  2. DoublyLinkedList (linkedlist.c) - O(1) transaction insertion"
	@echo "  3. BST (bst.c) - O(log n) date-based queries"
	@echo "  4. MaxHeap (heap.c) - O(log n) top expenses"
	@echo "  5. Queue (queue.c) - O(1) FIFO bill management"
	@echo "  6. Stack (stack.c) - O(1) undo operations"
	@echo "  7. Trie (trie.c) - O(m) autocomplete"

%.o: %.c $(HDRS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

# Test program
test: $(TARGET)
	$(CC) -o test_engine test_engine.c -L. -lfinance_dsa -lm -Wl,-rpath,.
	./test_engine
