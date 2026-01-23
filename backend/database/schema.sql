-- SQLite Schema for Smart Personal Finance Tracker
-- Normalized database design with proper indexes

-- Enable foreign keys
PRAGMA foreign_keys = ON;

-- Categories table
CREATE TABLE IF NOT EXISTS categories (
    id TEXT PRIMARY KEY,
    name TEXT UNIQUE NOT NULL,
    type TEXT CHECK(type IN ('income', 'expense', 'both')) DEFAULT 'both',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Transactions table
CREATE TABLE IF NOT EXISTS transactions (
    id TEXT PRIMARY KEY,
    type TEXT NOT NULL CHECK(type IN ('income', 'expense')),
    amount REAL NOT NULL CHECK(amount > 0),
    category_id TEXT NOT NULL,
    description TEXT DEFAULT '',
    date TEXT NOT NULL,  -- YYYY-MM-DD format for string comparison
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES categories(id)
);

-- Budgets table
CREATE TABLE IF NOT EXISTS budgets (
    id TEXT PRIMARY KEY,
    category_id TEXT UNIQUE NOT NULL,
    budget_limit REAL NOT NULL CHECK(budget_limit > 0),
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES categories(id)
);

-- Bills table
CREATE TABLE IF NOT EXISTS bills (
    id TEXT PRIMARY KEY,
    name TEXT NOT NULL,
    amount REAL NOT NULL CHECK(amount > 0),
    due_date TEXT NOT NULL,  -- YYYY-MM-DD format
    category_id TEXT NOT NULL,
    is_paid INTEGER DEFAULT 0,
    paid_at TIMESTAMP,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES categories(id)
);

-- Undo actions table
CREATE TABLE IF NOT EXISTS undo_actions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    action_type INTEGER NOT NULL,
    action_data TEXT NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Anomaly detection statistics table (for Z-Score)
CREATE TABLE IF NOT EXISTS spending_stats (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    category_id TEXT NOT NULL,
    mean_amount REAL DEFAULT 0,
    std_dev REAL DEFAULT 0,
    transaction_count INTEGER DEFAULT 0,
    sum_amount REAL DEFAULT 0,
    sum_squared REAL DEFAULT 0,
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (category_id) REFERENCES categories(id)
);

-- Daily spending aggregates (for sliding window)
CREATE TABLE IF NOT EXISTS daily_spending (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    date TEXT NOT NULL UNIQUE,
    total_income REAL DEFAULT 0,
    total_expenses REAL DEFAULT 0,
    transaction_count INTEGER DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Indexes for performance
CREATE INDEX IF NOT EXISTS idx_transactions_date ON transactions(date);
CREATE INDEX IF NOT EXISTS idx_transactions_category ON transactions(category_id);
CREATE INDEX IF NOT EXISTS idx_transactions_type ON transactions(type);
CREATE INDEX IF NOT EXISTS idx_transactions_amount ON transactions(amount DESC);
CREATE INDEX IF NOT EXISTS idx_bills_due_date ON bills(due_date);
CREATE INDEX IF NOT EXISTS idx_bills_is_paid ON bills(is_paid);
CREATE INDEX IF NOT EXISTS idx_daily_spending_date ON daily_spending(date);
CREATE INDEX IF NOT EXISTS idx_categories_name ON categories(name);

-- Views for common queries
CREATE VIEW IF NOT EXISTS v_budget_status AS
SELECT 
    b.id,
    c.name as category,
    b.budget_limit as limit_amount,
    COALESCE(SUM(CASE WHEN t.type = 'expense' THEN t.amount ELSE 0 END), 0) as spent,
    CASE 
        WHEN b.budget_limit > 0 THEN 
            ROUND(COALESCE(SUM(CASE WHEN t.type = 'expense' THEN t.amount ELSE 0 END), 0) / b.budget_limit * 100, 2)
        ELSE 0 
    END as percent_used
FROM budgets b
JOIN categories c ON b.category_id = c.id
LEFT JOIN transactions t ON t.category_id = c.id AND t.type = 'expense'
GROUP BY b.id, c.name, b.budget_limit;

CREATE VIEW IF NOT EXISTS v_category_totals AS
SELECT 
    c.id,
    c.name as category,
    COALESCE(SUM(CASE WHEN t.type = 'expense' THEN t.amount ELSE 0 END), 0) as total_expenses,
    COALESCE(SUM(CASE WHEN t.type = 'income' THEN t.amount ELSE 0 END), 0) as total_income,
    COUNT(t.id) as transaction_count
FROM categories c
LEFT JOIN transactions t ON t.category_id = c.id
GROUP BY c.id, c.name;
