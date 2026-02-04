"""
C Data Structures Engine - Python ctypes Wrapper
Provides Python interface to the C finance engine library
All 7 data structures are implemented in pure C and called via ctypes

Data Structures:
1. HashMap - Category to Budget mapping (O(1) operations)
2. DoublyLinkedList - Transaction history
3. BST - Date-sorted transactions (O(log n) operations)
4. MaxHeap - Top expenses (O(log n) operations)
5. Queue - Bill payment FIFO (O(1) operations)
6. Stack - Undo operations (O(1) operations)
7. Trie - Category autocomplete (O(m) operations)
"""

import ctypes
from ctypes import Structure, c_char, c_double, c_int, c_bool, POINTER, byref, create_string_buffer
from pathlib import Path
from typing import List, Dict, Optional, Tuple
import os

# Path to the shared library
LIB_PATH = Path(__file__).parent / "c_dsa" / "libfinance_dsa.so"

# C Structure definitions (must match common.h)
MAX_STRING_LEN = 256


class CTransaction(Structure):
    """Matches Transaction struct in C"""
    _fields_ = [
        ("id", c_char * 64),
        ("type", c_char * 16),
        ("amount", c_double),
        ("category", c_char * MAX_STRING_LEN),
        ("description", c_char * MAX_STRING_LEN),
        ("date", c_char * 16),
    ]
    
    def to_dict(self) -> dict:
        return {
            'id': self.id.decode('utf-8', errors='replace'),
            'type': self.type.decode('utf-8', errors='replace'),
            'amount': self.amount,
            'category': self.category.decode('utf-8', errors='replace'),
            'description': self.description.decode('utf-8', errors='replace'),
            'date': self.date.decode('utf-8', errors='replace'),
        }


class CBudget(Structure):
    """Matches Budget struct in C"""
    _fields_ = [
        ("category", c_char * MAX_STRING_LEN),
        ("limit", c_double),
        ("spent", c_double),
    ]
    
    def to_dict(self) -> dict:
        percent = (self.spent / self.limit * 100) if self.limit > 0 else 0
        alert_level = 'exceeded' if percent >= 100 else 'warning' if percent >= 80 else 'caution' if percent >= 50 else 'normal'
        return {
            'category': self.category.decode('utf-8', errors='replace'),
            'limit': self.limit,
            'spent': self.spent,
            'percentUsed': round(percent, 2),
            'alertLevel': alert_level,
        }


class CBill(Structure):
    """Matches Bill struct in C"""
    _fields_ = [
        ("id", c_char * 64),
        ("name", c_char * MAX_STRING_LEN),
        ("amount", c_double),
        ("due_date", c_char * 16),
        ("category", c_char * MAX_STRING_LEN),
        ("is_paid", c_bool),
    ]
    
    def to_dict(self) -> dict:
        return {
            'id': self.id.decode('utf-8', errors='replace'),
            'name': self.name.decode('utf-8', errors='replace'),
            'amount': self.amount,
            'dueDate': self.due_date.decode('utf-8', errors='replace'),
            'category': self.category.decode('utf-8', errors='replace'),
            'isPaid': bool(self.is_paid),
        }


class CCategoryAmount(Structure):
    """Matches CategoryAmount struct in C"""
    _fields_ = [
        ("category", c_char * MAX_STRING_LEN),
        ("total_amount", c_double),
    ]
    
    def to_dict(self) -> dict:
        return {
            'category': self.category.decode('utf-8', errors='replace'),
            'totalAmount': self.total_amount,
        }


class CDSAStats(Structure):
    """Matches DSAStats struct in C - tracks operations per data structure"""
    _fields_ = [
        ("hashmap_ops", c_int),
        ("linkedlist_ops", c_int),
        ("bst_ops", c_int),
        ("heap_ops", c_int),
        ("queue_ops", c_int),
        ("stack_ops", c_int),
        ("trie_ops", c_int),
        ("total_ops", c_int),
    ]
    
    def to_dict(self) -> dict:
        return {
            'hashmap_ops': self.hashmap_ops,
            'linkedlist_ops': self.linkedlist_ops,
            'bst_ops': self.bst_ops,
            'heap_ops': self.heap_ops,
            'queue_ops': self.queue_ops,
            'stack_ops': self.stack_ops,
            'trie_ops': self.trie_ops,
            'total_ops': self.total_ops,
        }


class CFinanceEngine:
    """
    Python wrapper for the C Finance Engine.
    Provides all operations using actual C data structures.
    
    This is the PROOF that real C data structures are being used:
    - Every operation increments the corresponding DSA counter
    - DSA stats can be retrieved to verify data structure usage
    """
    
    def __init__(self):
        self._lib = None
        self._engine = None
        self._is_initialized = False
        self._load_library()
    
    def _load_library(self):
        """Load the C shared library"""
        if not os.path.exists(LIB_PATH):
            raise FileNotFoundError(f"C library not found at {LIB_PATH}. Run 'make' in c_dsa directory.")
        
        self._lib = ctypes.CDLL(str(LIB_PATH))
        self._setup_function_signatures()
        
        # Create the engine
        self._engine = self._lib.engine_create()
        if not self._engine:
            raise RuntimeError("Failed to create C finance engine")
        
        self._is_initialized = True
        print(f"C Finance Engine initialized with 7 data structures")
    
    def _setup_function_signatures(self):
        """Define C function signatures for ctypes"""
        lib = self._lib
        
        # Engine lifecycle
        lib.engine_create.restype = ctypes.c_void_p
        lib.engine_create.argtypes = []
        
        lib.engine_destroy.restype = None
        lib.engine_destroy.argtypes = [ctypes.c_void_p]
        
        # Transaction operations
        lib.engine_add_transaction.restype = ctypes.c_char_p
        lib.engine_add_transaction.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_double,
            ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p
        ]
        
        lib.engine_delete_transaction.restype = c_bool
        lib.engine_delete_transaction.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        
        lib.engine_get_all_transactions.restype = c_int
        lib.engine_get_all_transactions.argtypes = [ctypes.c_void_p, POINTER(CTransaction), c_int]
        
        lib.engine_get_transactions_desc.restype = c_int
        lib.engine_get_transactions_desc.argtypes = [ctypes.c_void_p, POINTER(CTransaction), c_int]
        
        lib.engine_get_transactions_in_range.restype = c_int
        lib.engine_get_transactions_in_range.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, POINTER(CTransaction), c_int
        ]
        
        lib.engine_get_recent_transactions.restype = c_int
        lib.engine_get_recent_transactions.argtypes = [ctypes.c_void_p, c_int, POINTER(CTransaction)]
        
        lib.engine_find_transaction.restype = c_bool
        lib.engine_find_transaction.argtypes = [ctypes.c_void_p, ctypes.c_char_p, POINTER(CTransaction)]
        
        # Budget operations
        lib.engine_set_budget.restype = c_bool
        lib.engine_set_budget.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_double]
        
        lib.engine_get_budget.restype = c_bool
        lib.engine_get_budget.argtypes = [ctypes.c_void_p, ctypes.c_char_p, POINTER(CBudget)]
        
        lib.engine_get_all_budgets.restype = c_int
        lib.engine_get_all_budgets.argtypes = [ctypes.c_void_p, POINTER(CBudget), c_int]
        
        lib.engine_get_budget_alerts.restype = c_int
        lib.engine_get_budget_alerts.argtypes = [ctypes.c_void_p, POINTER(CBudget), c_int]
        
        # Bill operations
        lib.engine_add_bill.restype = ctypes.c_char_p
        lib.engine_add_bill.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_double,
            ctypes.c_char_p, ctypes.c_char_p
        ]
        
        lib.engine_get_all_bills.restype = c_int
        lib.engine_get_all_bills.argtypes = [ctypes.c_void_p, POINTER(CBill), c_int]
        
        lib.engine_pay_bill.restype = c_bool
        lib.engine_pay_bill.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        
        lib.engine_delete_bill.restype = c_bool
        lib.engine_delete_bill.argtypes = [ctypes.c_void_p, ctypes.c_char_p]
        
        # Analytics
        lib.engine_get_top_expenses.restype = c_int
        lib.engine_get_top_expenses.argtypes = [ctypes.c_void_p, c_int, POINTER(CTransaction)]
        
        lib.engine_get_top_categories.restype = c_int
        lib.engine_get_top_categories.argtypes = [ctypes.c_void_p, c_int, POINTER(CCategoryAmount)]
        
        # Autocomplete
        lib.engine_get_category_suggestions.restype = c_int
        lib.engine_get_category_suggestions.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, POINTER(ctypes.c_char_p), c_int
        ]
        
        lib.engine_get_all_categories.restype = c_int
        lib.engine_get_all_categories.argtypes = [ctypes.c_void_p, POINTER(ctypes.c_char_p), c_int]
        
        # Undo
        lib.engine_undo.restype = c_bool
        lib.engine_undo.argtypes = [ctypes.c_void_p]
        
        lib.engine_can_undo.restype = c_bool
        lib.engine_can_undo.argtypes = [ctypes.c_void_p]
        
        # Statistics
        lib.engine_get_total_balance.restype = c_double
        lib.engine_get_total_balance.argtypes = [ctypes.c_void_p]
        
        lib.engine_get_total_income.restype = c_double
        lib.engine_get_total_income.argtypes = [ctypes.c_void_p]
        
        lib.engine_get_total_expenses.restype = c_double
        lib.engine_get_total_expenses.argtypes = [ctypes.c_void_p]
        
        lib.engine_get_transaction_count.restype = c_int
        lib.engine_get_transaction_count.argtypes = [ctypes.c_void_p]
        
        lib.engine_get_budget_count.restype = c_int
        lib.engine_get_budget_count.argtypes = [ctypes.c_void_p]
        
        lib.engine_get_bill_count.restype = c_int
        lib.engine_get_bill_count.argtypes = [ctypes.c_void_p]
        
        # DSA Stats
        lib.engine_get_dsa_stats.restype = None
        lib.engine_get_dsa_stats.argtypes = [ctypes.c_void_p, POINTER(CDSAStats)]
        
        lib.engine_reset_stats.restype = None
        lib.engine_reset_stats.argtypes = [ctypes.c_void_p]
        
        # Data loading
        lib.engine_load_transaction.restype = None
        lib.engine_load_transaction.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p,
            ctypes.c_double, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_char_p
        ]
        
        lib.engine_load_budget.restype = None
        lib.engine_load_budget.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_double]
        
        lib.engine_load_bill.restype = None
        lib.engine_load_bill.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p,
            ctypes.c_double, ctypes.c_char_p, ctypes.c_char_p, c_bool
        ]
    
    def _encode(self, s: str) -> bytes:
        """Encode string for C"""
        return s.encode('utf-8') if s else b''
    
    # ==================== TRANSACTION OPERATIONS ====================
    
    def add_transaction(self, tx_type: str, amount: float, category: str,
                       description: str = '', date: str = '') -> Optional[str]:
        """
        Add a transaction.
        C Data Structures Used:
        - LinkedList: O(1) insertion
        - BST: O(log n) insertion for date ordering
        - Stack: O(1) for undo recording
        - Trie: O(m) for category addition
        - MaxHeap: O(log n) for expense tracking
        """
        result = self._lib.engine_add_transaction(
            self._engine,
            self._encode(tx_type),
            c_double(amount),
            self._encode(category),
            self._encode(description),
            self._encode(date)
        )
        return result.decode('utf-8') if result else None
    
    def delete_transaction(self, tx_id: str) -> bool:
        """
        Delete a transaction.
        C Data Structures Used:
        - LinkedList: O(n) deletion by ID
        - BST: O(n) deletion by ID
        - Stack: O(1) for undo recording
        """
        return self._lib.engine_delete_transaction(self._engine, self._encode(tx_id))
    
    def get_all_transactions(self) -> List[dict]:
        """
        Get all transactions.
        C Data Structure Used: LinkedList traversal O(n)
        """
        buffer = (CTransaction * 10000)()
        count = self._lib.engine_get_all_transactions(self._engine, buffer, 10000)
        return [buffer[i].to_dict() for i in range(count)]
    
    def get_transactions_desc(self) -> List[dict]:
        """
        Get transactions in descending date order.
        C Data Structure Used: BST reverse in-order traversal O(n)
        """
        buffer = (CTransaction * 10000)()
        count = self._lib.engine_get_transactions_desc(self._engine, buffer, 10000)
        return [buffer[i].to_dict() for i in range(count)]
    
    def get_transactions_in_range(self, start_date: str, end_date: str) -> List[dict]:
        """
        Get transactions in date range.
        C Data Structure Used: BST range query O(log n + k)
        """
        buffer = (CTransaction * 10000)()
        count = self._lib.engine_get_transactions_in_range(
            self._engine,
            self._encode(start_date),
            self._encode(end_date),
            buffer,
            10000
        )
        return [buffer[i].to_dict() for i in range(count)]
    
    def get_recent_transactions(self, count: int) -> List[dict]:
        """
        Get most recent transactions.
        C Data Structure Used: Stack (recent transactions stack)
        """
        buffer = (CTransaction * count)()
        result_count = self._lib.engine_get_recent_transactions(self._engine, count, buffer)
        return [buffer[i].to_dict() for i in range(result_count)]
    
    def find_transaction(self, tx_id: str) -> Optional[dict]:
        """
        Find transaction by ID.
        C Data Structure Used: BST search O(n) for ID
        """
        tx = CTransaction()
        if self._lib.engine_find_transaction(self._engine, self._encode(tx_id), byref(tx)):
            return tx.to_dict()
        return None
    
    # ==================== BUDGET OPERATIONS ====================
    
    def set_budget(self, category: str, limit: float) -> bool:
        """
        Set budget for category.
        C Data Structures Used:
        - HashMap: O(1) insertion/update
        - Stack: O(1) for undo recording
        - Trie: O(m) for category addition
        """
        return self._lib.engine_set_budget(self._engine, self._encode(category), c_double(limit))
    
    def get_budget(self, category: str) -> Optional[dict]:
        """
        Get budget for category.
        C Data Structure Used: HashMap O(1) lookup
        """
        budget = CBudget()
        if self._lib.engine_get_budget(self._engine, self._encode(category), byref(budget)):
            return budget.to_dict()
        return None
    
    def get_all_budgets(self) -> List[dict]:
        """
        Get all budgets.
        C Data Structure Used: HashMap iteration O(n)
        """
        buffer = (CBudget * 100)()
        count = self._lib.engine_get_all_budgets(self._engine, buffer, 100)
        return [buffer[i].to_dict() for i in range(count)]
    
    def get_budget_alerts(self) -> List[dict]:
        """
        Get budget alerts (>=50% used).
        C Data Structure Used: HashMap iteration + filtering
        """
        buffer = (CBudget * 100)()
        count = self._lib.engine_get_budget_alerts(self._engine, buffer, 100)
        alerts = []
        for i in range(count):
            b = buffer[i].to_dict()
            msg = f"Budget exceeded! Spent ${b['spent']:.0f} of ${b['limit']:.0f}" if b['alertLevel'] == 'exceeded' else \
                  f"Warning: {b['percentUsed']:.0f}% of budget used" if b['alertLevel'] == 'warning' else \
                  f"Caution: {b['percentUsed']:.0f}% of budget used"
            alerts.append({
                'category': b['category'],
                'level': b['alertLevel'],
                'percentUsed': b['percentUsed'],
                'spent': b['spent'],
                'limit': b['limit'],
                'message': msg,
                'priority': b['percentUsed']
            })
        return alerts
    
    # ==================== BILL OPERATIONS ====================
    
    def add_bill(self, name: str, amount: float, due_date: str, category: str) -> Optional[str]:
        """
        Add a bill to the queue.
        C Data Structure Used: Queue O(1) enqueue
        """
        result = self._lib.engine_add_bill(
            self._engine,
            self._encode(name),
            c_double(amount),
            self._encode(due_date),
            self._encode(category)
        )
        return result.decode('utf-8') if result else None
    
    def get_all_bills(self) -> List[dict]:
        """
        Get all bills (FIFO order).
        C Data Structure Used: Queue traversal O(n)
        """
        buffer = (CBill * 100)()
        count = self._lib.engine_get_all_bills(self._engine, buffer, 100)
        return [buffer[i].to_dict() for i in range(count)]
    
    def pay_bill(self, bill_id: str) -> bool:
        """
        Mark bill as paid.
        C Data Structure Used: Queue update O(n)
        """
        return self._lib.engine_pay_bill(self._engine, self._encode(bill_id))
    
    def delete_bill(self, bill_id: str) -> bool:
        """
        Delete a bill.
        C Data Structure Used: Queue removal O(n)
        """
        return self._lib.engine_delete_bill(self._engine, self._encode(bill_id))
    
    # ==================== ANALYTICS ====================
    
    def get_top_expenses(self, k: int) -> List[dict]:
        """
        Get top k expenses.
        C Data Structure Used: MaxHeap extract-top-k O(n log k)
        """
        buffer = (CTransaction * k)()
        count = self._lib.engine_get_top_expenses(self._engine, k, buffer)
        return [buffer[i].to_dict() for i in range(count)]
    
    def get_top_categories(self, k: int) -> List[dict]:
        """
        Get top k spending categories.
        C Data Structure Used: MaxHeap for categories O(n log k)
        """
        buffer = (CCategoryAmount * k)()
        count = self._lib.engine_get_top_categories(self._engine, k, buffer)
        return [buffer[i].to_dict() for i in range(count)]
    
    # ==================== AUTOCOMPLETE ====================
    
    def get_category_suggestions(self, prefix: str, max_count: int = 10) -> List[str]:
        """
        Get category suggestions by prefix.
        C Data Structure Used: Trie prefix search O(m + k)
        """
        # Allocate buffer of char pointers
        buffer = (ctypes.c_char_p * max_count)()
        for i in range(max_count):
            buffer[i] = ctypes.create_string_buffer(256)
        
        count = self._lib.engine_get_category_suggestions(
            self._engine,
            self._encode(prefix),
            buffer,
            max_count
        )
        
        results = []
        for i in range(count):
            if buffer[i]:
                results.append(buffer[i].decode('utf-8', errors='replace'))
        return results
    
    def get_all_categories(self) -> List[str]:
        """
        Get all categories.
        C Data Structure Used: Trie traversal O(n)
        """
        max_count = 200
        buffer = (ctypes.c_char_p * max_count)()
        for i in range(max_count):
            buffer[i] = ctypes.create_string_buffer(256)
        
        count = self._lib.engine_get_all_categories(self._engine, buffer, max_count)
        
        results = []
        for i in range(count):
            if buffer[i]:
                results.append(buffer[i].decode('utf-8', errors='replace'))
        return results
    
    # ==================== UNDO ====================
    
    def undo(self) -> bool:
        """
        Undo last action.
        C Data Structure Used: Stack O(1) pop
        """
        return self._lib.engine_undo(self._engine)
    
    def can_undo(self) -> bool:
        """Check if undo is available."""
        return self._lib.engine_can_undo(self._engine)
    
    # ==================== STATISTICS ====================
    
    def get_balance(self) -> float:
        """Get total balance."""
        return self._lib.engine_get_total_balance(self._engine)
    
    def get_total_income(self) -> float:
        """Get total income."""
        return self._lib.engine_get_total_income(self._engine)
    
    def get_total_expenses(self) -> float:
        """Get total expenses."""
        return self._lib.engine_get_total_expenses(self._engine)
    
    def get_transaction_count(self) -> int:
        """Get transaction count."""
        return self._lib.engine_get_transaction_count(self._engine)
    
    def get_budget_count(self) -> int:
        """Get budget count."""
        return self._lib.engine_get_budget_count(self._engine)
    
    def get_bill_count(self) -> int:
        """Get bill count."""
        return self._lib.engine_get_bill_count(self._engine)
    
    # ==================== DSA STATS (PROOF) ====================
    
    def get_dsa_stats(self) -> dict:
        """
        Get data structure operation statistics.
        THIS IS THE PROOF that real C data structures are being used!
        """
        stats = CDSAStats()
        self._lib.engine_get_dsa_stats(self._engine, byref(stats))
        return stats.to_dict()
    
    def reset_stats(self):
        """Reset DSA operation counters."""
        self._lib.engine_reset_stats(self._engine)
    
    # ==================== DATA LOADING ====================
    
    def load_transaction(self, tx_id: str, tx_type: str, amount: float,
                        category: str, description: str, date: str):
        """Load transaction from SQLite (for hydration)."""
        self._lib.engine_load_transaction(
            self._engine,
            self._encode(tx_id),
            self._encode(tx_type),
            c_double(amount),
            self._encode(category),
            self._encode(description),
            self._encode(date)
        )
    
    def load_budget(self, category: str, limit: float):
        """Load budget from SQLite (for hydration)."""
        self._lib.engine_load_budget(self._engine, self._encode(category), c_double(limit))
    
    def load_bill(self, bill_id: str, name: str, amount: float,
                 due_date: str, category: str, is_paid: bool):
        """Load bill from SQLite (for hydration)."""
        self._lib.engine_load_bill(
            self._engine,
            self._encode(bill_id),
            self._encode(name),
            c_double(amount),
            self._encode(due_date),
            self._encode(category),
            c_bool(is_paid)
        )
    
    def get_dashboard(self) -> dict:
        """Get dashboard summary."""
        return {
            'balance': self.get_balance(),
            'totalIncome': self.get_total_income(),
            'totalExpenses': self.get_total_expenses(),
            'transactionCount': self.get_transaction_count(),
            'budgetCount': self.get_budget_count(),
            'billCount': self.get_bill_count(),
            'canUndo': self.can_undo()
        }
    
    def __del__(self):
        """Clean up C engine."""
        if self._engine and self._lib:
            self._lib.engine_destroy(self._engine)


# Singleton instance
_c_engine: Optional[CFinanceEngine] = None


def get_c_engine() -> CFinanceEngine:
    """Get or create the C finance engine singleton."""
    global _c_engine
    if _c_engine is None:
        _c_engine = CFinanceEngine()
    return _c_engine


def is_c_engine_available() -> bool:
    """Check if C engine is available."""
    try:
        get_c_engine()
        return True
    except Exception as e:
        print(f"C Engine not available: {e}")
        return False
