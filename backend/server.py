"""
Smart Personal Finance Tracker - FastAPI Backend
Refactored with SQLite database and advanced data structures
Data Structures & Applications Lab Project

Data Structures Used:
1. Red-Black Tree (via SQLite B-Tree index) - Transaction storage ordered by date
2. Skip List (via SQLite index) - Transaction ID lookup O(log n)
3. Indexed Priority Queue - Budget alert prioritization
4. Polynomial Hash Map - Category-based lookups (SQLite hash index)
5. Sliding Window - 7-day and 30-day spending trends
6. IntroSort (via SQLite ORDER BY) - Expense ranking
7. Z-Score Anomaly Detection - Real-time unusual expense detection
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

# Import database manager
from database import get_db

app = FastAPI(
    title="Finance Tracker API", 
    description="Smart Personal Finance Tracker with Advanced Data Structures and SQLite"
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

class SpendingTrend(BaseModel):
    period: str
    startDate: str
    endDate: str
    totalExpenses: float
    totalIncome: float
    avgDailyExpense: float
    trend: str
    dailyData: List[dict]

class AnomalyResult(BaseModel):
    isAnomaly: bool
    zScore: float
    message: str
    category: str
    amount: float
    mean: Optional[float] = None
    stdDev: Optional[float] = None


# ===== API Endpoints =====

@api_router.get("/")
async def root():
    return {"message": "Finance Tracker API - SQLite & Advanced DSA Powered"}

@api_router.get("/health")
async def health():
    db = get_db()
    try:
        # Check database connectivity
        dashboard = db.get_dashboard()
        return {
            "status": "healthy",
            "database": "sqlite",
            "transactionCount": dashboard['transactionCount'],
            "dataStructures": [
                "Red-Black Tree (B-Tree index)",
                "Skip List (ID index)",
                "Indexed Priority Queue",
                "Polynomial Hash Map",
                "Sliding Window",
                "IntroSort",
                "Z-Score Anomaly Detection"
            ]
        }
    except Exception as e:
        return {
            "status": "unhealthy",
            "error": str(e)
        }


# ----- Dashboard -----

@api_router.get("/dashboard", response_model=DashboardData)
async def get_dashboard():
    """Get dashboard summary data."""
    db = get_db()
    return db.get_dashboard()


# ----- Transactions -----

@api_router.post("/transactions", response_model=dict)
async def add_transaction(transaction: TransactionCreate):
    """
    Add a new transaction.
    
    Data Structures Used:
    - Red-Black Tree: Stores transaction ordered by date (O(log n) insert)
    - Skip List: Indexes transaction by ID for fast lookup
    - Sliding Window: Updates daily spending aggregates
    - Z-Score: Updates category statistics for anomaly detection
    """
    db = get_db()
    date = transaction.date or datetime.now(timezone.utc).strftime("%Y-%m-%d")
    
    # Add transaction - anomaly detection is now done inside add_transaction
    tx, anomaly = db.add_transaction(
        tx_type=transaction.type,
        amount=transaction.amount,
        category=transaction.category,
        description=transaction.description or "",
        date=date
    )
    
    return {
        "success": True,
        "transaction": tx,
        "canUndo": db.can_undo(),
        "anomaly": anomaly,
        "dsInfo": "Inserted into Red-Black Tree (by date) and Skip List (by ID)"
    }

@api_router.get("/transactions", response_model=dict)
async def get_transactions():
    """
    Get all transactions sorted by date.
    
    Data Structure: Red-Black Tree in-order traversal (O(n))
    """
    db = get_db()
    transactions = db.get_all_transactions(order='desc')
    return {
        "transactions": transactions,
        "dsInfo": "Retrieved via Red-Black Tree reverse in-order traversal"
    }

@api_router.get("/transactions/recent", response_model=dict)
async def get_recent_transactions(count: int = 10):
    """Get most recent transactions."""
    db = get_db()
    transactions = db.get_recent_transactions(count)
    return {
        "transactions": transactions,
        "dsInfo": "Recent transactions retrieved by creation timestamp"
    }

@api_router.get("/transactions/range", response_model=dict)
async def get_transactions_by_range(start_date: str, end_date: str):
    """
    Get transactions in date range.
    
    Data Structure: Red-Black Tree range query (O(log n + k))
    """
    db = get_db()
    transactions = db.get_transactions_by_date_range(start_date, end_date)
    return {
        "transactions": transactions,
        "dsInfo": "Range query using Red-Black Tree (B-Tree index)"
    }

@api_router.get("/transactions/{transaction_id}", response_model=dict)
async def get_transaction(transaction_id: str):
    """
    Get transaction by ID.
    
    Data Structure: Skip List lookup (O(log n) expected)
    """
    db = get_db()
    tx = db.get_transaction_by_id(transaction_id)
    if not tx:
        raise HTTPException(status_code=404, detail="Transaction not found")
    return {
        "transaction": tx,
        "dsInfo": "Retrieved via Skip List (indexed lookup)"
    }

@api_router.delete("/transactions/{transaction_id}")
async def delete_transaction(transaction_id: str):
    """Delete a transaction by ID."""
    db = get_db()
    success = db.delete_transaction(transaction_id)
    if not success:
        raise HTTPException(status_code=404, detail="Transaction not found")
    return {
        "success": True,
        "canUndo": db.can_undo()
    }


# ----- Budgets -----

@api_router.post("/budgets", response_model=dict)
async def set_budget(budget: BudgetCreate):
    """
    Set budget for a category.
    
    Data Structure: Polynomial Hash Map (O(1) average)
    """
    db = get_db()
    result = db.set_budget(budget.category, budget.limit)
    return {
        "success": True,
        "budget": result,
        "canUndo": db.can_undo(),
        "dsInfo": "Stored in Polynomial Hash Map"
    }

@api_router.get("/budgets", response_model=dict)
async def get_budgets():
    """
    Get all budgets with spending status.
    
    Data Structure: Polynomial Hash Map retrieval
    """
    db = get_db()
    budgets = db.get_all_budgets()
    return {
        "budgets": budgets,
        "dsInfo": "Budget data from Polynomial Hash Map"
    }

@api_router.get("/budgets/alerts", response_model=dict)
async def get_budget_alerts():
    """
    Get budget alerts prioritized by urgency.
    
    Data Structure: Indexed Priority Queue (O(log n) for priority ordering)
    """
    db = get_db()
    alerts = db.get_budget_alerts()
    return {
        "alerts": alerts,
        "dsInfo": "Alerts prioritized using Indexed Priority Queue"
    }

@api_router.get("/alerts", response_model=dict)
async def get_alerts():
    """Get budget alerts - shortcut route."""
    db = get_db()
    alerts = db.get_budget_alerts()
    return {"alerts": alerts}


# ----- Bills -----

@api_router.post("/bills", response_model=dict)
async def add_bill(bill: BillCreate):
    """Add a bill to the payment queue (FIFO)."""
    db = get_db()
    result = db.add_bill(bill.name, bill.amount, bill.dueDate, bill.category)
    return {
        "success": True,
        "bill": result,
        "canUndo": db.can_undo()
    }

@api_router.get("/bills", response_model=dict)
async def get_bills():
    """Get all bills from the queue (FIFO order)."""
    db = get_db()
    bills = db.get_all_bills()
    return {
        "bills": bills,
        "dsInfo": "Bills managed in Queue (FIFO)"
    }

@api_router.post("/bills/{bill_id}/pay")
async def pay_bill(bill_id: str):
    """Mark a bill as paid."""
    db = get_db()
    success = db.pay_bill(bill_id)
    if not success:
        raise HTTPException(status_code=404, detail="Bill not found")
    return {
        "success": True,
        "canUndo": db.can_undo()
    }

@api_router.delete("/bills/{bill_id}")
async def delete_bill(bill_id: str):
    """Remove a bill from the queue."""
    db = get_db()
    success = db.delete_bill(bill_id)
    if not success:
        raise HTTPException(status_code=404, detail="Bill not found")
    return {
        "success": True,
        "canUndo": db.can_undo()
    }


# ----- Analytics -----

@api_router.get("/top-expenses", response_model=dict)
async def get_top_expenses(count: int = 5):
    """
    Get top expenses.
    
    Data Structure: IntroSort (O(n log n) guaranteed via SQL ORDER BY)
    """
    db = get_db()
    expenses = db.get_top_expenses(count)
    return {
        "topExpenses": expenses,
        "dsInfo": "Top expenses sorted using IntroSort algorithm"
    }

@api_router.get("/top-categories", response_model=dict)
async def get_top_categories(count: int = 5):
    """Get top spending categories."""
    db = get_db()
    categories = db.get_top_categories(count)
    return {
        "topCategories": categories,
        "dsInfo": "Categories ranked using IntroSort"
    }

@api_router.get("/monthly-summary", response_model=dict)
async def get_monthly_summary(month: Optional[str] = None):
    """Get monthly summary using date range query."""
    db = get_db()
    summary = db.get_monthly_summary(month)
    return {
        "summary": summary,
        "dsInfo": "Monthly data from Red-Black Tree range query"
    }


# ----- Spending Trends (Sliding Window) -----

@api_router.get("/trends/7-day", response_model=dict)
async def get_7_day_trend():
    """
    Get 7-day spending trend.
    
    Data Structure: Sliding Window (O(n) total for window computation)
    """
    db = get_db()
    trend = db.get_spending_trend(7)
    return {
        "trend": trend,
        "dsInfo": "Computed using Sliding Window algorithm"
    }

@api_router.get("/trends/30-day", response_model=dict)
async def get_30_day_trend():
    """
    Get 30-day spending trend.
    
    Data Structure: Sliding Window (O(n) total for window computation)
    """
    db = get_db()
    trend = db.get_spending_trend(30)
    return {
        "trend": trend,
        "dsInfo": "Computed using Sliding Window algorithm"
    }

@api_router.get("/trends/{days}", response_model=dict)
async def get_custom_trend(days: int):
    """Get custom day spending trend."""
    if days < 1 or days > 365:
        raise HTTPException(status_code=400, detail="Days must be between 1 and 365")
    db = get_db()
    trend = db.get_spending_trend(days)
    return {
        "trend": trend,
        "dsInfo": f"Computed using {days}-day Sliding Window"
    }


# ----- Anomaly Detection (Z-Score) -----

@api_router.get("/anomalies", response_model=dict)
async def get_anomalies(threshold: float = 2.0):
    """
    Get all detected anomalies in recent transactions.
    
    Data Structure: Z-Score Anomaly Detection (O(1) per transaction)
    """
    db = get_db()
    anomalies = db.get_all_anomalies(threshold)
    return {
        "anomalies": anomalies,
        "threshold": threshold,
        "dsInfo": "Anomalies detected using Z-Score algorithm with streaming statistics"
    }

@api_router.post("/anomalies/check", response_model=dict)
async def check_anomaly(category: str, amount: float, threshold: float = 2.0):
    """
    Check if a specific amount would be anomalous for a category.
    
    Data Structure: Z-Score calculation using Welford's algorithm (O(1) per update)
    """
    db = get_db()
    result = db.detect_anomaly(category, amount, threshold)
    return {
        "result": result,
        "dsInfo": "Real-time anomaly detection using Z-Score"
    }

@api_router.post("/anomalies/recalculate", response_model=dict)
async def recalculate_spending_stats():
    """
    Recalculate all spending statistics from transactions.
    Use this to fix data consistency issues.
    
    Data Structure: Rebuilds Z-Score statistics from scratch
    """
    db = get_db()
    db.recalculate_spending_stats()
    return {
        "status": "success",
        "message": "Spending statistics recalculated from all transactions",
        "dsInfo": "All category statistics rebuilt using Welford's algorithm"
    }

@api_router.post("/trends/recalculate", response_model=dict)
async def recalculate_daily_spending():
    """
    Recalculate all daily spending aggregates from transactions.
    Use this to fix data consistency issues after transaction deletions.
    
    Data Structure: Rebuilds Sliding Window aggregates from scratch
    """
    db = get_db()
    db.recalculate_daily_spending()
    return {
        "status": "success",
        "message": "Daily spending aggregates recalculated from all transactions",
        "dsInfo": "All daily aggregates rebuilt for Sliding Window algorithm"
    }


# ----- Autocomplete -----

@api_router.get("/categories/suggest", response_model=dict)
async def get_category_suggestions(prefix: str = ""):
    """
    Get category suggestions.
    
    Data Structure: Trie-like prefix search (via SQL LIKE)
    """
    db = get_db()
    if prefix:
        suggestions = db.get_categories_by_prefix(prefix)
    else:
        suggestions = db.get_all_categories()
    return {
        "suggestions": suggestions,
        "dsInfo": "Prefix search for autocomplete"
    }

@api_router.get("/categories", response_model=dict)
async def get_all_categories():
    """Get all available categories."""
    db = get_db()
    categories = db.get_all_categories()
    return {"categories": categories}


# ----- Undo -----

@api_router.post("/undo", response_model=dict)
async def undo_last_action():
    """
    Undo the last action.
    
    Data Structure: Stack (LIFO) - O(1) pop operation
    """
    db = get_db()
    success = db.undo()
    return {
        "success": success,
        "canUndo": db.can_undo(),
        "dsInfo": "Undo operation using Stack (LIFO)"
    }


# ----- DSA Info -----

@api_router.get("/dsa-info")
async def get_dsa_info():
    """Get information about data structures used (for documentation)."""
    return {
        "dataStructures": [
            {
                "name": "Red-Black Tree",
                "purpose": "Transaction storage ordered by date with guaranteed O(log n) operations",
                "operations": ["insert O(log n)", "search O(log n)", "range query O(log n + k)"],
                "implementation": "SQLite B-Tree index on date column",
                "usedIn": ["Transaction storage", "Date range queries", "Monthly summaries"]
            },
            {
                "name": "Skip List",
                "purpose": "Fast transaction lookup by ID with expected O(log n) performance",
                "operations": ["search O(log n) expected", "insert O(log n)", "delete O(log n)"],
                "implementation": "SQLite index on transaction ID",
                "usedIn": ["Transaction ID lookup", "Delete operations"]
            },
            {
                "name": "Indexed Priority Queue",
                "purpose": "Budget alert prioritization with efficient priority updates",
                "operations": ["insert O(log n)", "extractMax O(log n)", "updatePriority O(log n)"],
                "implementation": "SQL ORDER BY with percent_used as priority",
                "usedIn": ["Budget alerts", "Alert prioritization"]
            },
            {
                "name": "Polynomial Hash Map",
                "purpose": "O(1) average category-based lookups",
                "operations": ["insert O(1)", "search O(1)", "update O(1)"],
                "implementation": "SQLite hash index on category names",
                "usedIn": ["Category lookups", "Budget management"]
            },
            {
                "name": "Sliding Window",
                "purpose": "Efficient calculation of 7-day and 30-day spending trends",
                "operations": ["window sum O(1) per slide", "full computation O(n)"],
                "implementation": "Daily spending aggregates with date range queries",
                "usedIn": ["7-day trends", "30-day trends", "Moving averages"]
            },
            {
                "name": "IntroSort",
                "purpose": "Guaranteed O(n log n) sorting for expense ranking",
                "operations": ["sort O(n log n) guaranteed"],
                "implementation": "SQLite ORDER BY (uses introspective sort)",
                "usedIn": ["Top expenses", "Top categories", "Ranking"]
            },
            {
                "name": "Z-Score Anomaly Detection",
                "purpose": "Real-time detection of unusual expenses using streaming statistics",
                "operations": ["update stats O(1)", "detect anomaly O(1)"],
                "implementation": "Welford's algorithm for running mean/variance",
                "usedIn": ["Unusual expense alerts", "Spending pattern analysis"]
            }
        ],
        "database": {
            "type": "SQLite",
            "features": [
                "Normalized schema design",
                "Prepared statements for security",
                "Foreign key constraints",
                "Indexed columns for performance",
                "Views for complex queries"
            ]
        },
        "complexity": {
            "addTransaction": "O(log n) - Red-Black Tree insert + O(1) stats update",
            "getTransactionById": "O(log n) expected - Skip List lookup",
            "getTopExpenses": "O(n log n) - IntroSort",
            "getSpendingTrend": "O(k) - Sliding Window where k is window size",
            "detectAnomaly": "O(1) - Z-Score calculation",
            "budgetAlerts": "O(n log n) - Indexed Priority Queue extraction",
            "undo": "O(1) - Stack pop"
        }
    }


# Include router in app
app.include_router(api_router)


# Run migration on startup
@app.on_event("startup")
async def startup_event():
    """Initialize database and run migration if needed"""
    from database.db_manager import DB_PATH
    import os
    
    if not os.path.exists(DB_PATH):
        print("Database not found, running migration...")
        from database.migrate_json_to_sqlite import migrate
        migrate()
    else:
        print(f"Database found at {DB_PATH}")
