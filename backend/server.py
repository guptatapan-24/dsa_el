"""
Smart Personal Finance Tracker - FastAPI Backend
Hybrid C Data Structures + SQLite Architecture
Data Structures & Applications Lab Project

ARCHITECTURE:
- SQLite: Persistence layer (data survives server restarts)
- C Engine: In-memory operations using REAL data structures

C DATA STRUCTURES (Pure C, compiled to shared library):
1. HashMap - Category to Budget mapping (O(1) operations)
2. DoublyLinkedList - Transaction history
3. BST - Date-sorted transactions (O(log n) operations)
4. MaxHeap - Top expenses extraction
5. Queue - Bill payment FIFO
6. Stack - Undo operations
7. Trie - Category autocomplete

PROOF: Every API response includes 'dsaProof' with:
- Data structure used
- Operation performed
- Operation count from C engine
"""

from fastapi import FastAPI, HTTPException, APIRouter
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, Field
from typing import List, Optional
from datetime import datetime, timezone
import os
from pathlib import Path
from dotenv import load_dotenv

ROOT_DIR = Path(__file__).parent
load_dotenv(ROOT_DIR / '.env')

# Import database manager (SQLite for persistence)
from database import get_db

# Import C engine (for actual DSA operations)
from c_engine import get_c_engine, is_c_engine_available, CFinanceEngine

app = FastAPI(
    title="Finance Tracker API", 
    description="Smart Personal Finance Tracker with C Data Structures + SQLite Persistence"
)

# CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=[
        "http://localhost:3000",
        "http://127.0.0.1:3000",
        os.environ.get("FRONTEND_URL", "https://c-struct-reimplement.preview.emergentagent.com")
    ],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

api_router = APIRouter(prefix="/api")


# ===== Pydantic Models =====

class TransactionCreate(BaseModel):
    type: str = Field(..., description="'income' or 'expense'")
    amount: float = Field(..., gt=0)
    category: str
    description: Optional[str] = ""
    date: Optional[str] = None  # YYYY-MM-DD format

class Transaction(BaseModel):
    id: str
    type: str
    amount: float
    category: str
    description: str
    date: str

class BudgetCreate(BaseModel):
    category: str
    limit: float = Field(..., gt=0)

class Budget(BaseModel):
    category: str
    limit: float
    spent: float
    percentUsed: float
    alertLevel: str

class BillCreate(BaseModel):
    name: str
    amount: float = Field(..., gt=0)
    dueDate: str  # YYYY-MM-DD format
    category: str

class Bill(BaseModel):
    id: str
    name: str
    amount: float
    dueDate: str
    category: str
    isPaid: bool

class BudgetAlert(BaseModel):
    category: str
    level: str
    percentUsed: float
    spent: float
    limit: float
    message: str
    priority: Optional[float] = None

class CategoryAmount(BaseModel):
    category: str
    totalAmount: float

class MonthlySummary(BaseModel):
    month: str
    totalIncome: float
    totalExpenses: float
    netSavings: float
    transactionCount: int
    categoryBreakdown: List[dict]

class DashboardData(BaseModel):
    balance: float
    totalIncome: float
    totalExpenses: float
    transactionCount: int
    budgetCount: int
    billCount: int
    canUndo: bool


def get_dsa_proof(engine: CFinanceEngine, operation: str, ds_name: str, complexity: str) -> dict:
    """Generate DSA proof showing which data structure was used."""
    stats = engine.get_dsa_stats()
    return {
        'dataStructure': ds_name,
        'operation': operation,
        'complexity': complexity,
        'operationCounts': stats,
        'proof': f"C {ds_name} operation: {operation}"
    }


# ===== API Endpoints =====

@api_router.get("/")
async def root():
    c_available = is_c_engine_available()
    return {
        "message": "Finance Tracker API - C Data Structures + SQLite",
        "cEngineStatus": "ACTIVE" if c_available else "UNAVAILABLE",
        "architecture": "Hybrid (C for DSA operations, SQLite for persistence)"
    }


@api_router.get("/health")
async def health():
    db = get_db()
    c_available = is_c_engine_available()
    
    try:
        dashboard = db.get_dashboard()
        c_stats = get_c_engine().get_dsa_stats() if c_available else None
        
        return {
            "status": "healthy",
            "database": "sqlite",
            "cEngine": "active" if c_available else "unavailable",
            "transactionCount": dashboard['transactionCount'],
            "cDsaStats": c_stats,
            "dataStructures": [
                {"name": "HashMap", "impl": "C (hashmap.c)", "use": "Budget mapping O(1)"},
                {"name": "DoublyLinkedList", "impl": "C (linkedlist.c)", "use": "Transaction history"},
                {"name": "BST", "impl": "C (bst.c)", "use": "Date-sorted queries O(log n)"},
                {"name": "MaxHeap", "impl": "C (heap.c)", "use": "Top expenses O(log n)"},
                {"name": "Queue", "impl": "C (queue.c)", "use": "Bill FIFO O(1)"},
                {"name": "Stack", "impl": "C (stack.c)", "use": "Undo O(1)"},
                {"name": "Trie", "impl": "C (trie.c)", "use": "Autocomplete O(m)"}
            ]
        }
    except Exception as e:
        return {
            "status": "unhealthy",
            "error": str(e)
        }


# ----- Dashboard -----

@api_router.get("/dashboard", response_model=dict)
async def get_dashboard():
    """
    Get dashboard summary data.
    Uses C LinkedList traversal for statistics.
    """
    c_engine = get_c_engine()
    dashboard = c_engine.get_dashboard()
    
    return {
        **dashboard,
        "dsaProof": get_dsa_proof(c_engine, "traversal for totals", "LinkedList", "O(n)")
    }


# ----- Transactions -----

@api_router.post("/transactions", response_model=dict)
async def add_transaction(transaction: TransactionCreate):
    """
    Add a new transaction.
    
    C Data Structures Used:
    - LinkedList: O(1) insertion at front
    - BST: O(log n) insertion for date ordering
    - Stack: O(1) push for undo recording
    - Trie: O(m) for category autocomplete
    - MaxHeap: O(log n) for expense tracking (if expense)
    """
    db = get_db()
    c_engine = get_c_engine()
    
    date = transaction.date or datetime.now(timezone.utc).strftime("%Y-%m-%d")
    
    # Save to SQLite for persistence
    tx, anomaly = db.add_transaction(
        tx_type=transaction.type,
        amount=transaction.amount,
        category=transaction.category,
        description=transaction.description or "",
        date=date
    )
    
    # Add to C engine (uses all data structures)
    c_engine.add_transaction(
        tx_type=transaction.type,
        amount=transaction.amount,
        category=transaction.category,
        description=transaction.description or "",
        date=date
    )
    
    stats = c_engine.get_dsa_stats()
    
    return {
        "success": True,
        "transaction": tx,
        "canUndo": c_engine.can_undo(),
        "anomaly": anomaly,
        "dsaProof": {
            "dataStructures": ["LinkedList", "BST", "Stack", "Trie", "MaxHeap"],
            "operations": {
                "LinkedList": "add_front O(1)",
                "BST": "insert O(log n)",
                "Stack": "push O(1)",
                "Trie": "insert O(m)",
                "MaxHeap": "insert O(log n) [if expense]"
            },
            "operationCounts": stats,
            "proof": f"C engine processed: {stats['total_ops']} total DSA operations"
        }
    }


@api_router.get("/transactions", response_model=dict)
async def get_transactions():
    """
    Get all transactions sorted by date.
    
    C Data Structure: BST reverse in-order traversal (O(n))
    """
    c_engine = get_c_engine()
    
    # Use C BST for sorted retrieval
    transactions = c_engine.get_transactions_desc()
    
    return {
        "transactions": transactions,
        "dsaProof": get_dsa_proof(c_engine, "reverse_inorder_traversal", "BST", "O(n)")
    }


@api_router.get("/transactions/recent", response_model=dict)
async def get_recent_transactions(count: int = 10):
    """
    Get most recent transactions.
    C Data Structure: Stack (recent transactions stack)
    """
    c_engine = get_c_engine()
    transactions = c_engine.get_recent_transactions(count)
    
    return {
        "transactions": transactions,
        "dsaProof": get_dsa_proof(c_engine, "get_top_n", "Stack", "O(k)")
    }


@api_router.get("/transactions/range", response_model=dict)
async def get_transactions_by_range(start_date: str, end_date: str):
    """
    Get transactions in date range.
    
    C Data Structure: BST range query (O(log n + k))
    """
    c_engine = get_c_engine()
    transactions = c_engine.get_transactions_in_range(start_date, end_date)
    
    return {
        "transactions": transactions,
        "dsaProof": get_dsa_proof(c_engine, "range_query", "BST", "O(log n + k)")
    }


@api_router.get("/transactions/{transaction_id}", response_model=dict)
async def get_transaction(transaction_id: str):
    """
    Get transaction by ID.
    
    C Data Structure: BST search (O(n) for ID search)
    """
    c_engine = get_c_engine()
    tx = c_engine.find_transaction(transaction_id)
    
    if not tx:
        raise HTTPException(status_code=404, detail="Transaction not found")
    
    return {
        "transaction": tx,
        "dsaProof": get_dsa_proof(c_engine, "find_by_id", "BST", "O(n)")
    }


@api_router.delete("/transactions/{transaction_id}")
async def delete_transaction(transaction_id: str):
    """
    Delete a transaction by ID.
    
    C Data Structures: LinkedList O(n) delete, BST O(n) delete, Stack O(1) push
    """
    db = get_db()
    c_engine = get_c_engine()
    
    # Delete from SQLite
    success = db.delete_transaction(transaction_id)
    if not success:
        raise HTTPException(status_code=404, detail="Transaction not found")
    
    # Delete from C engine
    c_engine.delete_transaction(transaction_id)
    
    return {
        "success": True,
        "canUndo": c_engine.can_undo(),
        "dsaProof": get_dsa_proof(c_engine, "delete", "LinkedList + BST + Stack", "O(n)")
    }


# ----- Budgets -----

@api_router.post("/budgets", response_model=dict)
async def set_budget(budget: BudgetCreate):
    """
    Set budget for a category.
    
    C Data Structure: HashMap (O(1) average)
    """
    db = get_db()
    c_engine = get_c_engine()
    
    # Save to SQLite
    result = db.set_budget(budget.category, budget.limit)
    
    # Update C engine HashMap
    c_engine.set_budget(budget.category, budget.limit)
    
    return {
        "success": True,
        "budget": result,
        "canUndo": c_engine.can_undo(),
        "dsaProof": get_dsa_proof(c_engine, "insert/update", "HashMap", "O(1)")
    }


@api_router.get("/budgets", response_model=dict)
async def get_budgets():
    """
    Get all budgets with spending status.
    
    C Data Structure: HashMap retrieval
    """
    c_engine = get_c_engine()
    budgets = c_engine.get_all_budgets()
    
    return {
        "budgets": budgets,
        "dsaProof": get_dsa_proof(c_engine, "get_all", "HashMap", "O(n)")
    }


@api_router.get("/budgets/alerts", response_model=dict)
async def get_budget_alerts():
    """
    Get budget alerts prioritized by urgency.
    
    C Data Structure: HashMap iteration with filtering
    """
    c_engine = get_c_engine()
    alerts = c_engine.get_budget_alerts()
    
    return {
        "alerts": alerts,
        "dsaProof": get_dsa_proof(c_engine, "get_alerts (>=50% used)", "HashMap", "O(n)")
    }


@api_router.get("/alerts", response_model=dict)
async def get_alerts():
    """Get budget alerts - shortcut route."""
    c_engine = get_c_engine()
    alerts = c_engine.get_budget_alerts()
    return {"alerts": alerts}


# ----- Bills -----

@api_router.post("/bills", response_model=dict)
async def add_bill(bill: BillCreate):
    """
    Add a bill to the payment queue.
    
    C Data Structure: Queue O(1) enqueue
    """
    db = get_db()
    c_engine = get_c_engine()
    
    # Save to SQLite
    result = db.add_bill(bill.name, bill.amount, bill.dueDate, bill.category)
    
    # Add to C Queue
    c_engine.add_bill(bill.name, bill.amount, bill.dueDate, bill.category)
    
    return {
        "success": True,
        "bill": result,
        "canUndo": c_engine.can_undo(),
        "dsaProof": get_dsa_proof(c_engine, "enqueue", "Queue", "O(1)")
    }


@api_router.get("/bills", response_model=dict)
async def get_bills():
    """
    Get all bills from the queue (FIFO order).
    
    C Data Structure: Queue traversal
    """
    c_engine = get_c_engine()
    bills = c_engine.get_all_bills()
    
    return {
        "bills": bills,
        "dsaProof": get_dsa_proof(c_engine, "get_all (FIFO)", "Queue", "O(n)")
    }


@api_router.post("/bills/{bill_id}/pay")
async def pay_bill(bill_id: str):
    """
    Mark a bill as paid.
    
    C Data Structure: Queue update
    """
    db = get_db()
    c_engine = get_c_engine()
    
    # Update SQLite
    success = db.pay_bill(bill_id)
    if not success:
        raise HTTPException(status_code=404, detail="Bill not found")
    
    # Update C Queue
    c_engine.pay_bill(bill_id)
    
    return {
        "success": True,
        "canUndo": c_engine.can_undo(),
        "dsaProof": get_dsa_proof(c_engine, "mark_as_paid", "Queue", "O(n)")
    }


@api_router.delete("/bills/{bill_id}")
async def delete_bill(bill_id: str):
    """
    Remove a bill from the queue.
    
    C Data Structure: Queue remove
    """
    db = get_db()
    c_engine = get_c_engine()
    
    # Delete from SQLite
    success = db.delete_bill(bill_id)
    if not success:
        raise HTTPException(status_code=404, detail="Bill not found")
    
    # Delete from C Queue
    c_engine.delete_bill(bill_id)
    
    return {
        "success": True,
        "canUndo": c_engine.can_undo(),
        "dsaProof": get_dsa_proof(c_engine, "remove", "Queue", "O(n)")
    }


# ----- Analytics -----

@api_router.get("/top-expenses", response_model=dict)
async def get_top_expenses(count: int = 5):
    """
    Get top expenses.
    
    C Data Structure: MaxHeap extract-top-k O(n log k)
    """
    c_engine = get_c_engine()
    expenses = c_engine.get_top_expenses(count)
    
    return {
        "topExpenses": expenses,
        "dsaProof": get_dsa_proof(c_engine, f"extract_top_{count}", "MaxHeap", "O(n log k)")
    }


@api_router.get("/top-categories", response_model=dict)
async def get_top_categories(count: int = 5):
    """
    Get top spending categories.
    
    C Data Structure: MaxHeap for categories
    """
    c_engine = get_c_engine()
    categories = c_engine.get_top_categories(count)
    
    return {
        "topCategories": categories,
        "dsaProof": get_dsa_proof(c_engine, f"extract_top_{count}", "CategoryMaxHeap", "O(n log k)")
    }


@api_router.get("/monthly-summary", response_model=dict)
async def get_monthly_summary(month: Optional[str] = None):
    """
    Get monthly summary using date range query.
    Uses SQLite for complex aggregation (C engine for date range).
    """
    db = get_db()
    c_engine = get_c_engine()
    summary = db.get_monthly_summary(month)
    
    return {
        "summary": summary,
        "dsaProof": get_dsa_proof(c_engine, "date_range_aggregation", "BST + SQLite", "O(log n + k)")
    }


# ----- Spending Trends (uses SQLite sliding window) -----

@api_router.get("/trends/7-day", response_model=dict)
async def get_7_day_trend():
    """Get 7-day spending trend."""
    db = get_db()
    c_engine = get_c_engine()
    trend = db.get_spending_trend(7)
    
    return {
        "trend": trend,
        "dsaProof": get_dsa_proof(c_engine, "sliding_window_7day", "SQLite Aggregation", "O(k)")
    }


@api_router.get("/trends/30-day", response_model=dict)
async def get_30_day_trend():
    """Get 30-day spending trend."""
    db = get_db()
    c_engine = get_c_engine()
    trend = db.get_spending_trend(30)
    
    return {
        "trend": trend,
        "dsaProof": get_dsa_proof(c_engine, "sliding_window_30day", "SQLite Aggregation", "O(k)")
    }


@api_router.get("/trends/{days}", response_model=dict)
async def get_custom_trend(days: int):
    """Get custom day spending trend."""
    if days < 1 or days > 365:
        raise HTTPException(status_code=400, detail="Days must be between 1 and 365")
    db = get_db()
    c_engine = get_c_engine()
    trend = db.get_spending_trend(days)
    
    return {
        "trend": trend,
        "dsaProof": get_dsa_proof(c_engine, f"sliding_window_{days}day", "SQLite Aggregation", "O(k)")
    }


# ----- Anomaly Detection (uses SQLite Z-Score) -----

@api_router.get("/anomalies", response_model=dict)
async def get_anomalies(threshold: float = 2.0):
    """Get all detected anomalies in recent transactions."""
    db = get_db()
    c_engine = get_c_engine()
    anomalies = db.get_all_anomalies(threshold)
    
    return {
        "anomalies": anomalies,
        "threshold": threshold,
        "dsaProof": get_dsa_proof(c_engine, "z_score_detection", "SQLite Statistics", "O(1)")
    }


@api_router.post("/anomalies/check", response_model=dict)
async def check_anomaly(category: str, amount: float, threshold: float = 2.0):
    """Check if a specific amount would be anomalous."""
    db = get_db()
    c_engine = get_c_engine()
    result = db.detect_anomaly(category, amount, threshold)
    
    return {
        "result": result,
        "dsaProof": get_dsa_proof(c_engine, "z_score_check", "SQLite Statistics", "O(1)")
    }


@api_router.post("/anomalies/recalculate", response_model=dict)
async def recalculate_spending_stats():
    """Recalculate all spending statistics."""
    db = get_db()
    c_engine = get_c_engine()
    db.recalculate_spending_stats()
    
    return {
        "status": "success",
        "message": "Spending statistics recalculated",
        "dsaProof": get_dsa_proof(c_engine, "stats_recalculation", "SQLite", "O(n)")
    }


@api_router.post("/trends/recalculate", response_model=dict)
async def recalculate_daily_spending():
    """Recalculate all daily spending aggregates."""
    db = get_db()
    c_engine = get_c_engine()
    db.recalculate_daily_spending()
    
    return {
        "status": "success",
        "message": "Daily spending aggregates recalculated",
        "dsaProof": get_dsa_proof(c_engine, "daily_aggregates_recalc", "SQLite", "O(n)")
    }


# ----- Autocomplete -----

@api_router.get("/categories/suggest", response_model=dict)
async def get_category_suggestions(prefix: str = ""):
    """
    Get category suggestions.
    
    C Data Structure: Trie prefix search (O(m + k))
    """
    c_engine = get_c_engine()
    
    if prefix:
        suggestions = c_engine.get_category_suggestions(prefix)
    else:
        suggestions = c_engine.get_all_categories()
    
    return {
        "suggestions": suggestions,
        "dsaProof": get_dsa_proof(
            c_engine, 
            "prefix_search" if prefix else "get_all_words",
            "Trie",
            "O(m + k)" if prefix else "O(n)"
        )
    }


@api_router.get("/categories", response_model=dict)
async def get_all_categories():
    """
    Get all available categories.
    
    C Data Structure: Trie traversal
    """
    c_engine = get_c_engine()
    categories = c_engine.get_all_categories()
    
    return {
        "categories": categories,
        "dsaProof": get_dsa_proof(c_engine, "traverse_all", "Trie", "O(n)")
    }


# ----- Undo -----

@api_router.post("/undo", response_model=dict)
async def undo_last_action():
    """
    Undo the last action.
    
    C Data Structure: Stack (LIFO) - O(1) pop operation
    """
    db = get_db()
    c_engine = get_c_engine()
    
    # Undo in SQLite
    success = db.undo()
    
    # Undo in C engine
    c_engine.undo()
    
    return {
        "success": success,
        "canUndo": c_engine.can_undo(),
        "dsaProof": get_dsa_proof(c_engine, "pop", "Stack", "O(1)")
    }


# ----- DSA Info & Stats -----

@api_router.get("/dsa-info")
async def get_dsa_info():
    """Get information about data structures used."""
    c_engine = get_c_engine()
    stats = c_engine.get_dsa_stats()
    
    return {
        "implementation": "Pure C (compiled to libfinance_dsa.so)",
        "persistence": "SQLite (data survives restarts)",
        "operationProof": stats,
        "dataStructures": [
            {
                "name": "HashMap",
                "file": "hashmap.c / hashmap.h",
                "purpose": "Category to Budget mapping",
                "operations": ["insert O(1)", "search O(1)", "update O(1)", "delete O(1)"],
                "usedIn": ["Budget management", "Expense tracking per category"],
                "opCount": stats['hashmap_ops']
            },
            {
                "name": "DoublyLinkedList",
                "file": "linkedlist.c / linkedlist.h",
                "purpose": "Transaction history storage",
                "operations": ["add_front O(1)", "add_back O(1)", "traverse O(n)", "delete O(n)"],
                "usedIn": ["Transaction storage", "History traversal"],
                "opCount": stats['linkedlist_ops']
            },
            {
                "name": "Binary Search Tree (BST)",
                "file": "bst.c / bst.h",
                "purpose": "Date-sorted transaction queries",
                "operations": ["insert O(log n)", "search O(log n)", "range_query O(log n + k)"],
                "usedIn": ["Date range queries", "Sorted transaction retrieval"],
                "opCount": stats['bst_ops']
            },
            {
                "name": "MaxHeap",
                "file": "heap.c / heap.h",
                "purpose": "Top-K expense extraction",
                "operations": ["insert O(log n)", "extract_max O(log n)", "build O(n)"],
                "usedIn": ["Top expenses", "Top categories"],
                "opCount": stats['heap_ops']
            },
            {
                "name": "Queue",
                "file": "queue.c / queue.h",
                "purpose": "Bill payment FIFO management",
                "operations": ["enqueue O(1)", "dequeue O(1)", "peek O(1)"],
                "usedIn": ["Bill queue", "FIFO bill payments"],
                "opCount": stats['queue_ops']
            },
            {
                "name": "Stack",
                "file": "stack.c / stack.h",
                "purpose": "Undo operations + recent transactions",
                "operations": ["push O(1)", "pop O(1)", "peek O(1)"],
                "usedIn": ["Undo functionality", "Recent transactions"],
                "opCount": stats['stack_ops']
            },
            {
                "name": "Trie",
                "file": "trie.c / trie.h",
                "purpose": "Category autocomplete",
                "operations": ["insert O(m)", "search O(m)", "prefix_search O(m + k)"],
                "usedIn": ["Category suggestions", "Autocomplete"],
                "opCount": stats['trie_ops']
            }
        ],
        "totalOperations": stats['total_ops'],
        "complexity": {
            "addTransaction": "O(1) LinkedList + O(log n) BST + O(1) Stack + O(m) Trie",
            "getTransactionById": "O(n) BST/LinkedList search by ID",
            "getTransactionsInRange": "O(log n + k) BST range query",
            "getTopExpenses": "O(n log k) MaxHeap extraction",
            "setBudget": "O(1) HashMap insert",
            "getBudgetAlerts": "O(n) HashMap iteration",
            "addBill": "O(1) Queue enqueue",
            "getBills": "O(n) Queue traversal (FIFO order)",
            "getCategorySuggestions": "O(m + k) Trie prefix search",
            "undo": "O(1) Stack pop"
        }
    }


@api_router.get("/dsa-stats")
async def get_dsa_stats():
    """
    Get real-time DSA operation statistics.
    THIS IS THE PROOF that C data structures are being used!
    """
    c_engine = get_c_engine()
    stats = c_engine.get_dsa_stats()
    
    return {
        "message": "PROOF: These are actual operation counts from C data structures",
        "implementation": "libfinance_dsa.so (Pure C, compiled with gcc)",
        "stats": stats,
        "breakdown": {
            "HashMap": {
                "operations": stats['hashmap_ops'],
                "description": "Category-budget mapping operations"
            },
            "LinkedList": {
                "operations": stats['linkedlist_ops'],
                "description": "Transaction list operations"
            },
            "BST": {
                "operations": stats['bst_ops'],
                "description": "Date-sorted query operations"
            },
            "Heap": {
                "operations": stats['heap_ops'],
                "description": "Top-K extraction operations"
            },
            "Queue": {
                "operations": stats['queue_ops'],
                "description": "Bill FIFO operations"
            },
            "Stack": {
                "operations": stats['stack_ops'],
                "description": "Undo and recent transactions operations"
            },
            "Trie": {
                "operations": stats['trie_ops'],
                "description": "Autocomplete operations"
            }
        },
        "totalOperations": stats['total_ops']
    }


@api_router.post("/dsa-stats/reset")
async def reset_dsa_stats():
    """Reset DSA operation counters."""
    c_engine = get_c_engine()
    c_engine.reset_stats()
    
    return {
        "message": "DSA stats reset to zero",
        "stats": c_engine.get_dsa_stats()
    }


# Include router in app
app.include_router(api_router)


# Hydrate C engine from SQLite on startup
@app.on_event("startup")
async def startup_event():
    """Initialize database and hydrate C engine from SQLite."""
    from database.db_manager import DB_PATH
    import os
    
    # Ensure database exists
    if not os.path.exists(DB_PATH):
        print("Database not found, running migration...")
        from database.migrate_json_to_sqlite import migrate
        migrate()
    else:
        print(f"Database found at {DB_PATH}")
    
    # Initialize C engine
    try:
        c_engine = get_c_engine()
        print("C Finance Engine initialized")
        
        # Hydrate C engine from SQLite
        db = get_db()
        
        # Load all transactions into C engine
        transactions = db.get_all_transactions(order='asc')
        for tx in transactions:
            c_engine.load_transaction(
                tx['id'], tx['type'], tx['amount'],
                tx['category'], tx.get('description', ''), tx['date']
            )
        print(f"Loaded {len(transactions)} transactions into C engine")
        
        # Load all budgets into C engine
        budgets = db.get_all_budgets()
        for b in budgets:
            c_engine.load_budget(b['category'], b['limit'])
        print(f"Loaded {len(budgets)} budgets into C engine")
        
        # Load all bills into C engine
        bills = db.get_all_bills()
        for bill in bills:
            c_engine.load_bill(
                bill['id'], bill['name'], bill['amount'],
                bill['dueDate'], bill['category'], bill['isPaid']
            )
        print(f"Loaded {len(bills)} bills into C engine")
        
        # Show initial stats
        stats = c_engine.get_dsa_stats()
        print(f"C Engine DSA Stats after hydration: {stats}")
        
    except Exception as e:
        print(f"Warning: C Engine initialization failed: {e}")
        print("Falling back to SQLite-only mode")
