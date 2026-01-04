"""
Smart Personal Finance Tracker - FastAPI Backend
Calls C++ executables for DSA operations
Data Structures & Applications Lab Project
"""

from fastapi import FastAPI, HTTPException, APIRouter
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, Field
from typing import List, Optional
from datetime import datetime, timezone
import subprocess
import json
import os
from pathlib import Path
from dotenv import load_dotenv

ROOT_DIR = Path(__file__).parent
load_dotenv(ROOT_DIR / '.env')

app = FastAPI(title="Finance Tracker API", description="DSA-powered Finance Tracker")
# CORS middleware
app.add_middleware(
    CORSMiddleware,
    allow_origins=[
        "http://localhost:3000",
        "http://127.0.0.1:3000",
        "https://finance-tracker-1641.preview.emergentagent.com"
    ],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

api_router = APIRouter(prefix="/api")

# Paths
CPP_ENGINE = ROOT_DIR / "cpp" / "finance_engine.exe"
DATA_DIR = ROOT_DIR / "data"

# Ensure data directory exists
DATA_DIR.mkdir(exist_ok=True)

# Initialize empty data files if they don't exist
for filename in ["transactions.json", "budgets.json", "bills.json"]:
    filepath = DATA_DIR / filename
    if not filepath.exists():
        key = filename.replace(".json", "")
        filepath.write_text(json.dumps({key: []}))


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


# ===== C++ Engine Communication =====

def call_cpp_engine(command: str, params: dict = None) -> dict:
    """Call the C++ finance engine with a command and return the result."""
    
    # Build the input JSON
    input_data = {"command": command}
    if params:
        input_data["params"] = params
    
    input_json = json.dumps(input_data)
    
    try:
        # Run the C++ executable
        result = subprocess.run(
            [str(CPP_ENGINE)],
            input=input_json,
            capture_output=True,
            text=True,
            timeout=10,
            cwd=str(DATA_DIR)
        )
        
        if result.returncode != 0:
            raise HTTPException(
                status_code=500, 
                detail=f"C++ engine error: {result.stderr}"
            )
        
        # Parse output JSON
        output = result.stdout.strip()
        if not output:
            return {"error": "Empty response from engine"}
        
        return json.loads(output)
        
    except subprocess.TimeoutExpired:
        raise HTTPException(status_code=504, detail="Engine timeout")
    except json.JSONDecodeError as e:
        raise HTTPException(status_code=500, detail=f"Invalid JSON response: {e}")
    except FileNotFoundError:
        raise HTTPException(
            status_code=500, 
            detail="C++ engine not found. Please compile first."
        )


# ===== API Endpoints =====

@api_router.get("/")
async def root():
    return {"message": "Finance Tracker API - DSA Powered"}

@api_router.get("/health")
async def health():
    # Check if C++ engine exists
    engine_exists = CPP_ENGINE.exists()
    return {
        "status": "healthy" if engine_exists else "degraded",
        "engine_available": engine_exists,
        "data_dir": str(DATA_DIR)
    }


# ----- Dashboard -----

@api_router.get("/dashboard", response_model=DashboardData)
async def get_dashboard():
    """Get dashboard summary data."""
    result = call_cpp_engine("get_dashboard")
    return result


# ----- Transactions -----

@api_router.post("/transactions", response_model=dict)
async def add_transaction(transaction: TransactionCreate):
    """Add a new transaction (using Doubly Linked List + BST)."""
    params = {
        "type": transaction.type,
        "amount": transaction.amount,
        "category": transaction.category,
        "description": transaction.description or "",
        "date": transaction.date or datetime.now(timezone.utc).strftime("%Y-%m-%d")
    }
    result = call_cpp_engine("add_transaction", params)
    return result

@api_router.get("/transactions", response_model=dict)
async def get_transactions():
    """Get all transactions sorted by date (using BST in-order traversal)."""
    result = call_cpp_engine("get_transactions")
    return result

@api_router.get("/transactions/recent", response_model=dict)
async def get_recent_transactions(count: int = 10):
    """Get recent transactions (using Stack - LIFO)."""
    result = call_cpp_engine("get_recent_transactions", {"count": str(count)})
    return result

@api_router.get("/transactions/range", response_model=dict)
async def get_transactions_by_range(start_date: str, end_date: str):
    """Get transactions in date range (using BST range query)."""
    result = call_cpp_engine("get_transactions_by_date", {
        "startDate": start_date,
        "endDate": end_date
    })
    return result

@api_router.delete("/transactions/{transaction_id}")
async def delete_transaction(transaction_id: str):
    """Delete a transaction by ID."""
    result = call_cpp_engine("delete_transaction", {"id": transaction_id})
    return result


# ----- Budgets -----

@api_router.post("/budgets", response_model=dict)
async def set_budget(budget: BudgetCreate):
    """Set budget for a category (using HashMap)."""
    result = call_cpp_engine("set_budget", {
        "category": budget.category,
        "limit": budget.limit
    })
    return result

@api_router.get("/budgets", response_model=dict)
async def get_budgets():
    """Get all budgets with spending status (from HashMap)."""
    result = call_cpp_engine("get_budgets")
    return result

@api_router.get("/budgets/alerts", response_model=dict)
async def get_budget_alerts():
    """Get budget alerts (50%, 80%, 100% thresholds)."""
    result = call_cpp_engine("get_alerts")
    return result

@api_router.get("/alerts", response_model=dict)
async def get_alerts():
    """Get budget alerts - shortcut route (50%, 80%, 100% thresholds)."""
    result = call_cpp_engine("get_alerts")
    return result

# ----- Bills -----

@api_router.post("/bills", response_model=dict)
async def add_bill(bill: BillCreate):
    """Add a bill to the payment queue (using Queue - FIFO)."""
    result = call_cpp_engine("add_bill", {
        "name": bill.name,
        "amount": bill.amount,
        "dueDate": bill.dueDate,
        "category": bill.category
    })
    return result

@api_router.get("/bills", response_model=dict)
async def get_bills():
    """Get all bills from the queue."""
    result = call_cpp_engine("get_bills")
    return result

@api_router.post("/bills/{bill_id}/pay")
async def pay_bill(bill_id: str):
    """Mark a bill as paid."""
    result = call_cpp_engine("pay_bill", {"id": bill_id})
    return result

@api_router.delete("/bills/{bill_id}")
async def delete_bill(bill_id: str):
    """Remove a bill from the queue."""
    result = call_cpp_engine("delete_bill", {"id": bill_id})
    return result


# ----- Analytics -----

@api_router.get("/top-expenses", response_model=dict)
async def get_top_expenses(count: int = 5):
    """Get top expenses (using Max Heap - extract max)."""
    result = call_cpp_engine("get_top_expenses", {"count": str(count)})
    return result

@api_router.get("/top-categories", response_model=dict)
async def get_top_categories(count: int = 5):
    """Get top spending categories (using Category Max Heap)."""
    result = call_cpp_engine("get_top_categories", {"count": str(count)})
    return result

@api_router.get("/monthly-summary", response_model=dict)
async def get_monthly_summary(month: Optional[str] = None):
    """Get monthly summary (using BST month range query)."""
    params = {"month": month} if month else {}
    result = call_cpp_engine("get_monthly_summary", params)
    return result


# ----- Autocomplete -----

@api_router.get("/categories/suggest", response_model=dict)
async def get_category_suggestions(prefix: str = ""):
    """Get category suggestions (using Trie prefix search)."""
    result = call_cpp_engine("get_category_suggestions", {"prefix": prefix})
    return result

@api_router.get("/categories", response_model=dict)
async def get_all_categories():
    """Get all available categories."""
    result = call_cpp_engine("get_all_categories")
    return result


# ----- Undo -----

@api_router.post("/undo", response_model=dict)
async def undo_last_action():
    """Undo the last action (using Stack - pop)."""
    result = call_cpp_engine("undo")
    return result


# ----- DSA Info (for academic presentation) -----

@api_router.get("/dsa-info")
async def get_dsa_info():
    """Get information about data structures used (for viva)."""
    return {
        "dataStructures": [
            {
                "name": "Hash Map",
                "purpose": "Store category → budget mapping and expense totals",
                "operations": ["insert O(1)", "search O(1)", "update O(1)"],
                "usedIn": ["Budget management", "Category expense tracking"]
            },
            {
                "name": "Doubly Linked List",
                "purpose": "Maintain transaction history with bidirectional traversal",
                "operations": ["addFront O(1)", "delete O(n)", "traverse O(n)"],
                "usedIn": ["Transaction history", "Recent transactions"]
            },
            {
                "name": "Binary Search Tree (BST)",
                "purpose": "Store transactions sorted by date for efficient range queries",
                "operations": ["insert O(log n)", "range query O(log n + k)", "in-order O(n)"],
                "usedIn": ["Date-wise queries", "Monthly summaries"]
            },
            {
                "name": "Max Heap",
                "purpose": "Identify top spending transactions and categories",
                "operations": ["insert O(log n)", "extractMax O(log n)", "buildHeap O(n)"],
                "usedIn": ["Top expenses", "Top categories"]
            },
            {
                "name": "Queue (FIFO)",
                "purpose": "Manage upcoming bill payments in order",
                "operations": ["enqueue O(1)", "dequeue O(1)", "peek O(1)"],
                "usedIn": ["Bill management", "Payment scheduling"]
            },
            {
                "name": "Stack (LIFO)",
                "purpose": "Implement undo functionality and recent transactions",
                "operations": ["push O(1)", "pop O(1)", "peek O(1)"],
                "usedIn": ["Undo operations", "Recent transactions display"]
            },
            {
                "name": "Trie",
                "purpose": "Category and payee autocomplete functionality",
                "operations": ["insert O(m)", "prefixSearch O(m+k)"],
                "usedIn": ["Category suggestions", "Search autocomplete"]
            }
        ],
        "complexity": {
            "addTransaction": "O(log n) - BST insert + O(1) HashMap update",
            "getTopExpenses": "O(k log n) - Heap extract k times",
            "rangeQuery": "O(log n + k) - BST range traversal",
            "undo": "O(1) - Stack pop"
        }
    }


# Include router in app
app.include_router(api_router)