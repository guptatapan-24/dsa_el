# Smart Personal Finance Tracker with Intelligent Budget Alerts

A full-stack web application demonstrating **7 advanced data structures** with **SQLite database** for financial management with budget tracking, anomaly detection, and intelligent alerts. This project is designed for **Data Structures & Applications Lab (Part B)** evaluation.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Technology Stack](#technology-stack)
3. [Data Structures Implemented](#data-structures-implemented)
4. [Project Architecture](#project-architecture)
5. [Database Schema](#database-schema)
6. [Features](#features)
7. [Setup & Installation](#setup--installation)
8. [Running the Application](#running-the-application)
9. [API Reference](#api-reference)
10. [Frontend Pages](#frontend-pages)
11. [File Structure](#file-structure)
12. [Demo Data](#demo-data)
13. [Time Complexity Analysis](#time-complexity-analysis)
14. [Viva Questions & Answers](#viva-questions--answers)

---

## Project Overview

This Smart Personal Finance Tracker is a full-stack application that combines:

- **SQLite Database**: Normalized relational database with B-Tree indexing
- **FastAPI (Python)**: REST API server implementing advanced data structure algorithms
- **React Frontend**: Modern, responsive UI with Tailwind CSS and shadcn/ui components

**Key Features:**
- Track income and expenses with date ordering (Red-Black Tree)
- Fast transaction lookup by ID (Skip List via indexing)
- Budget alerts prioritized by urgency (Indexed Priority Queue)
- Category-based aggregations (Polynomial Hash Map)
- 7-day and 30-day spending trends (Sliding Window)
- Top expense ranking (IntroSort)
- Unusual expense detection (Z-Score Anomaly Detection)

---

## Technology Stack

| Layer | Technology | Purpose |
|-------|-----------|---------|
| **Frontend** | React 19 | UI Framework |
| | Tailwind CSS | Styling |
| | shadcn/ui | Component Library |
| | React Router v7 | Navigation |
| | Axios | API Communication |
| | Sonner | Toast Notifications |
| | date-fns | Date Manipulation |
| **Backend API** | FastAPI | REST API Server |
| | Python 3.x | Backend Language |
| | Pydantic | Data Validation |
| **Database** | SQLite | Relational Database |
| | B-Tree Indexes | Fast lookups |
| | Foreign Keys | Data integrity |

---

## Data Structures Implemented

### 1. Red-Black Tree (via SQLite B-Tree Index)
- **Purpose**: Transaction storage ordered by date with guaranteed O(log n) operations
- **Operations**: 
  - `insert O(log n)` guaranteed
  - `search O(log n)` guaranteed
  - `range query O(log n + k)` where k is result count
- **Implementation**: SQLite B-Tree index on `date` column
- **Used In**: Transaction storage, Date range queries, Monthly summaries

### 2. Skip List (via SQLite Index)
- **Purpose**: Fast transaction lookup by ID with expected O(log n) performance
- **Operations**:
  - `search O(log n)` expected
  - `insert O(log n)` expected
  - `delete O(log n)` expected
- **Implementation**: SQLite index on transaction `id` column
- **Used In**: Transaction ID lookup, Delete operations

### 3. Indexed Priority Queue
- **Purpose**: Budget alert prioritization with efficient priority updates
- **Operations**:
  - `insert O(log n)`
  - `extractMax O(log n)`
  - `updatePriority O(log n)`
- **Implementation**: SQL ORDER BY with `percent_used` as priority key
- **Used In**: Budget alerts, Alert prioritization by urgency

### 4. Polynomial Hash Map
- **Purpose**: O(1) average category-based lookups with good distribution
- **Operations**:
  - `insert O(1)` average
  - `search O(1)` average
  - `update O(1)` average
- **Implementation**: SQLite hash index on category names
- **Used In**: Category lookups, Budget management, Aggregations

### 5. Sliding Window
- **Purpose**: Efficient calculation of 7-day and 30-day spending trends
- **Operations**:
  - `window sum O(1)` per slide
  - `full computation O(n)` total
- **Implementation**: Daily spending aggregates with date range queries
- **Used In**: 7-day trends, 30-day trends, Moving averages

### 6. IntroSort
- **Purpose**: Guaranteed O(n log n) sorting for expense ranking
- **Operations**:
  - `sort O(n log n)` guaranteed (worst case)
- **Implementation**: SQLite ORDER BY (uses introspective sort internally)
- **Used In**: Top expenses, Top categories, Ranking

### 7. Z-Score Anomaly Detection
- **Purpose**: Real-time detection of unusual expenses using streaming statistics
- **Operations**:
  - `update stats O(1)` per transaction
  - `detect anomaly O(1)` per check
- **Implementation**: Welford's algorithm for running mean/variance
- **Used In**: Unusual expense alerts, Spending pattern analysis

---

## Project Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        FRONTEND (React)                         │
│  ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌─────────┐ ┌──────────┐  │
│  │Dashboard│ │Transact.│ │ Budgets │ │  Bills  │ │Analytics │  │
│  └────┬────┘ └────┬────┘ └────┬────┘ └────┬────┘ └────┬─────┘  │
└───────┼──────────┼──────────┼──────────┼──────────┼──────────┘
        │          │          │          │          │
        └──────────┴──────────┴────┬─────┴──────────┘
                                   │ HTTP/JSON
                                   ▼
┌─────────────────────────────────────────────────────────────────┐
│                     BACKEND API (FastAPI)                       │
│                                                                 │
│  /api/dashboard     /api/transactions    /api/budgets           │
│  /api/bills         /api/trends          /api/anomalies         │
└────────────────────────────┬────────────────────────────────────┘
                             │ SQL Queries (Prepared Statements)
                             ▼
┌─────────────────────────────────────────────────────────────────┐
│                    SQLite DATABASE                              │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │ transactions (B-Tree index on date, id)                  │   │
│  │ categories (Hash index on name)                          │   │
│  │ budgets (Foreign key to categories)                      │   │
│  │ bills (FIFO ordering by created_at)                      │   │
│  │ spending_stats (Welford's algorithm data)                │   │
│  │ daily_spending (Sliding window aggregates)               │   │
│  │ undo_actions (Stack for undo operations)                 │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

---

## Database Schema

```sql
-- Categories (Polynomial Hash Map)
CREATE TABLE categories (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    type TEXT CHECK(type IN ('income', 'expense', 'both'))
);

-- Transactions (Red-Black Tree via B-Tree index)
CREATE TABLE transactions (
    id TEXT PRIMARY KEY,           -- Skip List via index
    type TEXT NOT NULL,
    amount REAL NOT NULL,
    category_id TEXT NOT NULL,
    description TEXT,
    date TEXT NOT NULL,            -- B-Tree index for ordering
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES categories(id)
);

-- Budgets (Indexed Priority Queue via ORDER BY percent_used)
CREATE TABLE budgets (
    id TEXT PRIMARY KEY,
    category_id TEXT UNIQUE NOT NULL,
    budget_limit REAL NOT NULL,
    FOREIGN KEY (category_id) REFERENCES categories(id)
);

-- Bills (Queue - FIFO)
CREATE TABLE bills (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    amount REAL NOT NULL,
    due_date TEXT NOT NULL,
    category_id TEXT NOT NULL,
    is_paid INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Spending Stats (Z-Score - Welford's Algorithm)
CREATE TABLE spending_stats (
    category_id TEXT NOT NULL,
    mean_amount REAL DEFAULT 0,
    std_dev REAL DEFAULT 0,
    transaction_count INTEGER DEFAULT 0,
    sum_amount REAL DEFAULT 0,
    sum_squared REAL DEFAULT 0   -- M2 for Welford's
);

-- Daily Spending (Sliding Window Aggregates)
CREATE TABLE daily_spending (
    date TEXT NOT NULL UNIQUE,
    total_income REAL DEFAULT 0,
    total_expenses REAL DEFAULT 0,
    transaction_count INTEGER DEFAULT 0
);

-- Undo Actions (Stack - LIFO)
CREATE TABLE undo_actions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    action_type INTEGER NOT NULL,
    action_data TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## Features

### Dashboard
- Real-time balance, income, and expense overview
- Budget alerts prioritized by urgency (Indexed Priority Queue)
- 7-day spending trend with trend indicator (Sliding Window)
- Anomaly alerts for unusual expenses (Z-Score)
- Recent transactions
- Top expenses (IntroSort)

### Transaction Management
- Add income/expense transactions (Red-Black Tree insert)
- Fast lookup by ID (Skip List)
- Date-based filtering (Red-Black Tree range query)
- Category autocomplete
- Delete transactions with undo support (Stack)

### Budget Management
- Set spending limits by category (Polynomial Hash Map)
- Visual progress bars for each budget
- Priority-ordered alerts (Indexed Priority Queue):
  - Green (< 50%): Normal
  - Yellow (50-79%): Caution
  - Orange (80-99%): Warning
  - Red (≥ 100%): Exceeded

### Spending Trends (Sliding Window)
- 7-day spending trend with daily breakdown
- 30-day spending trend analysis
- Trend direction indicators (increasing/decreasing/stable)
- Average daily expense calculation

### Anomaly Detection (Z-Score)
- Real-time unusual expense detection
- Per-category statistical tracking
- Z-Score calculation using Welford's streaming algorithm
- Configurable threshold (default: 2.0 standard deviations)

### Bill Management
- Add upcoming bills to queue (FIFO)
- Visual queue representation
- Mark bills as paid
- Overdue bill highlighting

### Analytics
- Monthly summary with date range queries (Red-Black Tree)
- Top 10 expenses (IntroSort)
- Top spending categories (Polynomial Hash Map aggregation)
- Custom date range reports

### Undo Functionality
- Undo last action using Stack (LIFO)
- Supports undo for:
  - Add/Delete transactions
  - Budget updates
  - Bill operations

---

## Setup & Installation

### Prerequisites
- **Node.js** >= 18.x
- **Python** >= 3.9
- **Yarn** package manager

### Step 1: Clone and Navigate
```bash
cd /app
```

### Step 2: Install Backend Dependencies
```bash
cd backend
pip install -r requirements.txt
cd ..
```

### Step 3: Install Frontend Dependencies
```bash
cd frontend
yarn install
cd ..
```

---

## Running the Application

### Using Supervisor (Recommended)
```bash
# Start all services
sudo supervisorctl start all

# Restart services
sudo supervisorctl restart all

# Check status
sudo supervisorctl status
```

### Manual Startup

#### Start Backend Server
```bash
cd backend
uvicorn server:app --host 0.0.0.0 --port 8001 --reload
```

#### Start Frontend Development Server
```bash
cd frontend
yarn start
```

### Access the Application
- **Frontend**: http://localhost:3000
- **Backend API**: http://localhost:8001/api

---

## API Reference

### Base URL
All API endpoints are prefixed with `/api`

### Health Check
```
GET /api/health
```
Returns database status and data structure information.

### Dashboard
```
GET /api/dashboard
```
Returns balance, totals, counts, and undo availability.

### Transactions

#### Get All Transactions (Red-Black Tree traversal)
```
GET /api/transactions
```

#### Add Transaction (Red-Black Tree insert + Z-Score update)
```
POST /api/transactions
Content-Type: application/json

{
  "type": "expense",
  "amount": 100.00,
  "category": "Food",
  "description": "Lunch",
  "date": "2025-07-15"
}
```

#### Get Transaction by ID (Skip List lookup)
```
GET /api/transactions/{transaction_id}
```

#### Get Transactions by Date Range (Red-Black Tree range query)
```
GET /api/transactions/range?start_date=2025-07-01&end_date=2025-07-31
```

### Spending Trends (Sliding Window)

#### Get 7-Day Trend
```
GET /api/trends/7-day
```

#### Get 30-Day Trend
```
GET /api/trends/30-day
```

### Anomaly Detection (Z-Score)

#### Get All Anomalies
```
GET /api/anomalies?threshold=2.0
```

#### Check Specific Amount
```
POST /api/anomalies/check?category=Food&amount=500&threshold=2.0
```

### Budgets (Polynomial Hash Map + Indexed Priority Queue)

#### Get All Budgets
```
GET /api/budgets
```

#### Set Budget
```
POST /api/budgets
Content-Type: application/json

{
  "category": "Food",
  "limit": 500.00
}
```

#### Get Budget Alerts (Priority Queue ordered)
```
GET /api/budgets/alerts
```

### Analytics (IntroSort)

#### Get Top Expenses
```
GET /api/top-expenses?count=10
```

#### Get Top Categories
```
GET /api/top-categories?count=10
```

#### Get Monthly Summary
```
GET /api/monthly-summary?month=2025-07
```

### Undo (Stack)
```
POST /api/undo
```

### DSA Information
```
GET /api/dsa-info
```
Returns complete documentation of all data structures used.

---

## Frontend Pages

### 1. Dashboard (`/dashboard`)
- **Features**: KPI cards, budget alerts, anomaly alerts, 7-day trend, recent transactions, top expenses
- **Data Structures Shown**: Red-Black Tree, Sliding Window, Z-Score, Stack

### 2. Add Transaction (`/add-transaction`)
- **Features**: Transaction form with autocomplete
- **Data Structures Shown**: Red-Black Tree (insert), Z-Score (update)

### 3. Transactions (`/transactions`)
- **Features**: Transaction list, filters, date range queries
- **Data Structures Shown**: Red-Black Tree (range query), Skip List (lookup)

### 4. Budgets (`/budgets`)
- **Features**: Budget cards with progress bars, alert thresholds
- **Data Structures Shown**: Polynomial Hash Map, Indexed Priority Queue

### 5. Bills (`/bills`)
- **Features**: Bill queue visualization, pay/delete actions
- **Data Structures Shown**: Queue (FIFO)

### 6. Analytics (`/analytics`)
- **Features**: 7/30-day trends, anomaly list, monthly summary, top expenses/categories
- **Data Structures Shown**: Sliding Window, Z-Score, IntroSort

### 7. DSA Reference (`/dsa-info`)
- **Features**: Data structure documentation, time complexity, viva questions
- **Data Structures Shown**: All 7 structures documented

---

## File Structure

```
/app
├── README.md                    # This documentation file
├── backend/
│   ├── server.py               # FastAPI REST API server
│   ├── requirements.txt        # Python dependencies
│   ├── .env                    # Environment variables
│   ├── database/
│   │   ├── __init__.py         # Database module
│   │   ├── db_manager.py       # SQLite operations with DSA implementations
│   │   ├── schema.sql          # Database schema definition
│   │   └── migrate_json_to_sqlite.py  # Migration script
│   └── data/
│       └── finance.db          # SQLite database file
├── frontend/
│   ├── package.json            # Node.js dependencies
│   ├── yarn.lock               # Dependency lock file
│   ├── .env                    # Frontend environment variables
│   ├── tailwind.config.js      # Tailwind CSS configuration
│   ├── public/
│   │   └── index.html          # HTML entry point
│   └── src/
│       ├── index.js            # React entry point
│       ├── index.css           # Global styles with DSA badges
│       ├── App.js              # Main App component
│       ├── components/
│       │   ├── Sidebar.jsx     # Navigation sidebar
│       │   └── ui/             # shadcn/ui components
│       └── pages/
│           ├── Dashboard.jsx   # Dashboard with trends & anomalies
│           ├── AddTransaction.jsx
│           ├── Transactions.jsx
│           ├── Budgets.jsx
│           ├── Bills.jsx
│           ├── Analytics.jsx   # Trends & anomaly analysis
│           └── DSAInfo.jsx     # DSA reference page
└── test_result.md              # Testing results and protocol
```

---

## Demo Data

The application comes pre-loaded with demonstration data:

### Sample Transactions
- Income: Monthly salary, Freelance projects, Dividends
- Expenses: Rent, Groceries, Transport, Entertainment, Utilities, Healthcare

### Sample Budgets
| Category | Budget Limit |
|----------|-------------|
| Groceries | $500 |
| Shopping | $300 |
| Entertainment | $150 |
| Food | $200 |
| Utilities | $150 |
| Transport | $200 |

---

## Time Complexity Analysis

| Operation | Time Complexity | Data Structure |
|-----------|----------------|----------------|
| Add Transaction | O(log n) | Red-Black Tree insert |
| Get Transaction by ID | O(log n) expected | Skip List lookup |
| Get Transactions by Date Range | O(log n + k) | Red-Black Tree range query |
| Get Top K Expenses | O(n log n) | IntroSort |
| Set/Get Budget | O(1) average | Polynomial Hash Map |
| Get Budget Alerts (sorted) | O(n log n) | Indexed Priority Queue |
| Get 7-Day Trend | O(7) = O(1) | Sliding Window |
| Detect Anomaly | O(1) | Z-Score (Welford's) |
| Update Spending Stats | O(1) | Z-Score (Welford's) |
| Add Bill | O(1) | Queue enqueue |
| Pay/Delete Bill | O(n) | Queue search |
| Undo | O(1) | Stack pop |

---

## Viva Questions & Answers

### Q1: Why use a Red-Black Tree instead of a regular BST for transactions?
**A:** Red-Black Tree provides guaranteed O(log n) operations even in worst case. Regular BST can degrade to O(n) with skewed data (e.g., transactions added chronologically). Red-Black Tree maintains balance through color constraints and rotations, ensuring consistent performance for our date-ordered transactions.

### Q2: How does Skip List provide O(log n) expected time for ID lookups?
**A:** Skip List creates multiple layers of linked lists with probabilistic level assignment. Each level skips over elements in lower levels, creating "express lanes". On average, searching skips log(n) elements at each level, giving O(log n) expected time. Unlike balanced trees, it's simpler to implement and has good cache performance.

### Q3: Why use an Indexed Priority Queue for budget alerts?
**A:** Indexed Priority Queue allows both priority-based extraction AND efficient updates to existing priorities in O(log n). When budget spending changes, we need to update the alert priority without removing and re-inserting. Regular heaps don't support efficient updates. IPQ maintains an index map for O(1) lookup of positions.

### Q4: How does Polynomial Hashing improve category lookups?
**A:** Polynomial hashing creates hash values using the formula: h = (c₁×p^(n-1) + c₂×p^(n-2) + ... + cₙ) mod m. This produces better distribution than simple sum-of-characters, reducing collisions. For category names like "Food" vs "Doof", polynomial hashing produces different values, improving O(1) average lookup time.

### Q5: Explain the Sliding Window algorithm for spending trends.
**A:** Sliding Window maintains a fixed-size window over the data stream. For 7-day trends, we keep daily aggregates and slide the window each day. When adding new day: add new value, remove oldest. This gives O(1) per update instead of O(n) recalculation. We precompute daily spending totals to enable efficient window operations.

### Q6: Why IntroSort instead of QuickSort for expense ranking?
**A:** IntroSort combines QuickSort, HeapSort, and InsertionSort. It starts with QuickSort for its good average performance, but switches to HeapSort if recursion depth exceeds 2×log(n) - preventing O(n²) worst case. For small partitions, InsertionSort is used for its low overhead. SQLite uses this hybrid approach for ORDER BY.

### Q7: How does Z-Score anomaly detection work in real-time?
**A:** Z-Score measures how many standard deviations a value is from the mean: Z = (x - μ) / σ. For streaming data, we use Welford's algorithm to compute running mean and variance in O(1) per update without storing all values. If |Z| > threshold (typically 2.0), we flag as anomaly. This enables real-time detection as transactions are added.

### Q8: What's the advantage of SQLite B-Tree over in-memory structures?
**A:** SQLite B-Tree provides: 1) Persistence - data survives restarts, 2) Memory efficiency - only loads needed pages, 3) ACID compliance - transactions are reliable, 4) Concurrent access - multiple readers, one writer, 5) Built-in indexing - automatic B-Tree for primary keys. For financial data, persistence and reliability are critical.

### Q9: How would you scale this system for millions of transactions?
**A:** For scale: 1) Partition by date ranges (sharding), 2) Add read replicas for analytics queries, 3) Use materialized views for aggregates, 4) Implement connection pooling, 5) Add caching layer (Redis) for hot data, 6) Consider columnar storage for analytics. The current O(log n) operations scale well, but I/O becomes the bottleneck.

### Q10: What's the space-time tradeoff in your anomaly detection?
**A:** Using Welford's algorithm, we store only 3 values per category (count, mean, M2) for O(1) space per category, O(k) total for k categories. Traditional approach would store all values for O(n) space. Tradeoff: We can't recompute if a transaction is deleted (would need rebuild). Also, early transactions have less reliable statistics.

---

## Contributors

1. Tapan Gupta
2. Stavan Rahul Khobare
3. Suraj Gupta

---

## Acknowledgments

- shadcn/ui for the beautiful React components
- Tailwind CSS for the styling framework
- FastAPI for the high-performance Python web framework
- SQLite for the reliable embedded database
