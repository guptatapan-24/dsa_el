"""
SQLite Database Manager for Smart Personal Finance Tracker
Handles all database operations with prepared statements
"""

import sqlite3
import os
import uuid
from datetime import datetime, timedelta
from typing import List, Dict, Optional, Tuple, Any
from pathlib import Path
import math

DB_PATH = Path(__file__).parent.parent / "data" / "finance.db"
SCHEMA_PATH = Path(__file__).parent / "schema.sql"


class DatabaseManager:
    """Manages SQLite database operations with connection pooling"""
    
    def __init__(self, db_path: str = None):
        self.db_path = db_path or str(DB_PATH)
        self._ensure_db_exists()
    
    def _ensure_db_exists(self):
        """Create database and schema if not exists"""
        os.makedirs(os.path.dirname(self.db_path), exist_ok=True)
        
        if not os.path.exists(self.db_path):
            conn = sqlite3.connect(self.db_path)
            with open(SCHEMA_PATH, 'r') as f:
                conn.executescript(f.read())
            conn.commit()
            conn.close()
    
    def get_connection(self) -> sqlite3.Connection:
        """Get a database connection"""
        conn = sqlite3.connect(self.db_path)
        conn.row_factory = sqlite3.Row
        conn.execute("PRAGMA foreign_keys = ON")
        return conn
    
    def execute(self, query: str, params: tuple = ()) -> sqlite3.Cursor:
        """Execute a query with parameters"""
        conn = self.get_connection()
        try:
            cursor = conn.execute(query, params)
            conn.commit()
            return cursor
        finally:
            conn.close()
    
    def fetch_one(self, query: str, params: tuple = ()) -> Optional[Dict]:
        """Fetch single row"""
        conn = self.get_connection()
        try:
            cursor = conn.execute(query, params)
            row = cursor.fetchone()
            return dict(row) if row else None
        finally:
            conn.close()
    
    def fetch_all(self, query: str, params: tuple = ()) -> List[Dict]:
        """Fetch all rows"""
        conn = self.get_connection()
        try:
            cursor = conn.execute(query, params)
            return [dict(row) for row in cursor.fetchall()]
        finally:
            conn.close()
    
    # ==================== CATEGORY OPERATIONS ====================
    
    def get_or_create_category(self, name: str, cat_type: str = 'both') -> str:
        """Get existing category or create new one, return ID"""
        existing = self.fetch_one(
            "SELECT id FROM categories WHERE name = ?", (name,)
        )
        if existing:
            return existing['id']
        
        cat_id = f"cat_{uuid.uuid4().hex[:8]}"
        self.execute(
            "INSERT INTO categories (id, name, type) VALUES (?, ?, ?)",
            (cat_id, name, cat_type)
        )
        return cat_id
    
    def get_all_categories(self) -> List[str]:
        """Get all category names"""
        rows = self.fetch_all("SELECT name FROM categories ORDER BY name")
        return [row['name'] for row in rows]
    
    def get_categories_by_prefix(self, prefix: str, limit: int = 10) -> List[str]:
        """Get categories matching prefix (Trie-like behavior via SQL)"""
        rows = self.fetch_all(
            "SELECT name FROM categories WHERE LOWER(name) LIKE LOWER(?) || '%' LIMIT ?",
            (prefix, limit)
        )
        return [row['name'] for row in rows]
    
    # ==================== TRANSACTION OPERATIONS ====================
    
    def add_transaction(self, tx_type: str, amount: float, category: str, 
                        description: str = '', date: str = None) -> Tuple[Dict, Optional[Dict]]:
        """Add a new transaction and return anomaly info"""
        tx_id = f"txn_{uuid.uuid4().hex[:12]}"
        date = date or datetime.now().strftime('%Y-%m-%d')
        cat_id = self.get_or_create_category(category)
        
        # Detect anomaly BEFORE updating stats (for expenses)
        anomaly_info = None
        is_anomaly = 0
        z_score = 0.0
        if tx_type == 'expense':
            anomaly_info = self._detect_anomaly_internal(cat_id, category, amount)
            is_anomaly = 1 if anomaly_info.get('isAnomaly', False) else 0
            z_score = anomaly_info.get('zScore', 0.0)
        
        # Insert transaction with anomaly info
        self.execute(
            """INSERT INTO transactions (id, type, amount, category_id, description, date, is_anomaly, z_score)
               VALUES (?, ?, ?, ?, ?, ?, ?, ?)""",
            (tx_id, tx_type, amount, cat_id, description, date, is_anomaly, z_score)
        )
        
        # Update daily spending aggregates
        self._update_daily_spending(date, tx_type, amount, 1)
        
        # Update spending stats for anomaly detection AFTER checking for anomaly
        if tx_type == 'expense':
            self._update_spending_stats(cat_id, amount)
        
        # Record undo action
        self._push_undo(0, f"{tx_id}|{tx_type}|{amount}|{category}|{description}|{date}")
        
        return {
            'id': tx_id, 'type': tx_type, 'amount': amount,
            'category': category, 'description': description, 'date': date
        }, anomaly_info
    
    def get_transaction_by_id(self, tx_id: str) -> Optional[Dict]:
        """Get transaction by ID (Skip List simulation via indexed lookup)"""
        row = self.fetch_one(
            """SELECT t.id, t.type, t.amount, c.name as category, t.description, t.date
               FROM transactions t
               JOIN categories c ON t.category_id = c.id
               WHERE t.id = ?""",
            (tx_id,)
        )
        return row
    
    def get_transactions_by_date_range(self, start_date: str, end_date: str) -> List[Dict]:
        """Get transactions in date range (Red-Black Tree range query simulation)"""
        return self.fetch_all(
            """SELECT t.id, t.type, t.amount, c.name as category, t.description, t.date
               FROM transactions t
               JOIN categories c ON t.category_id = c.id
               WHERE t.date BETWEEN ? AND ?
               ORDER BY t.date DESC, t.created_at DESC""",
            (start_date, end_date)
        )
    
    def get_all_transactions(self, order: str = 'desc') -> List[Dict]:
        """Get all transactions ordered by date"""
        order_clause = 'DESC' if order == 'desc' else 'ASC'
        return self.fetch_all(
            f"""SELECT t.id, t.type, t.amount, c.name as category, t.description, t.date
                FROM transactions t
                JOIN categories c ON t.category_id = c.id
                ORDER BY t.date {order_clause}, t.created_at {order_clause}"""
        )
    
    def get_recent_transactions(self, count: int = 10) -> List[Dict]:
        """Get most recent transactions"""
        return self.fetch_all(
            """SELECT t.id, t.type, t.amount, c.name as category, t.description, t.date
               FROM transactions t
               JOIN categories c ON t.category_id = c.id
               ORDER BY t.created_at DESC
               LIMIT ?""",
            (count,)
        )
    
    def delete_transaction(self, tx_id: str) -> bool:
        """Delete a transaction by ID"""
        tx = self.get_transaction_by_id(tx_id)
        if not tx:
            return False
        
        # Record for undo
        self._push_undo(1, f"{tx['id']}|{tx['type']}|{tx['amount']}|{tx['category']}|{tx['description']}|{tx['date']}")
        
        # IMPORTANT: Delete transaction FIRST before updating stats
        # This ensures _remove_from_spending_stats calculates correctly without the deleted transaction
        self.execute("DELETE FROM transactions WHERE id = ?", (tx_id,))
        
        # Update daily spending
        self._update_daily_spending(tx['date'], tx['type'], -tx['amount'], -1)
        
        # Update spending stats for anomaly detection (for expenses)
        if tx['type'] == 'expense':
            self._remove_from_spending_stats(tx['category'], tx['amount'])
        
        return True
    
    # ==================== BUDGET OPERATIONS ====================
    
    def set_budget(self, category: str, limit: float) -> Dict:
        """Set or update budget for a category"""
        cat_id = self.get_or_create_category(category)
        budget_id = f"budget_{uuid.uuid4().hex[:8]}"
        
        existing = self.fetch_one(
            "SELECT id, budget_limit FROM budgets WHERE category_id = ?", (cat_id,)
        )
        
        if existing:
            # Record old value for undo
            self._push_undo(3, f"{category}|{existing['budget_limit']}")
            self.execute(
                "UPDATE budgets SET budget_limit = ?, updated_at = CURRENT_TIMESTAMP WHERE category_id = ?",
                (limit, cat_id)
            )
            budget_id = existing['id']
        else:
            self._push_undo(2, f"{category}|{limit}")
            self.execute(
                "INSERT INTO budgets (id, category_id, budget_limit) VALUES (?, ?, ?)",
                (budget_id, cat_id, limit)
            )
        
        return self.get_budget(category)
    
    def get_budget(self, category: str) -> Optional[Dict]:
        """Get budget status for a category"""
        row = self.fetch_one(
            """SELECT b.id, c.name as category, b.budget_limit as budget_limit,
                      COALESCE(SUM(CASE WHEN t.type = 'expense' THEN t.amount ELSE 0 END), 0) as spent
               FROM budgets b
               JOIN categories c ON b.category_id = c.id
               LEFT JOIN transactions t ON t.category_id = c.id AND t.type = 'expense'
               WHERE c.name = ?
               GROUP BY b.id, c.name, b.budget_limit""",
            (category,)
        )
        if row:
            percent = (row['spent'] / row['budget_limit'] * 100) if row['budget_limit'] > 0 else 0
            alert_level = 'exceeded' if percent >= 100 else 'warning' if percent >= 80 else 'caution' if percent >= 50 else 'normal'
            return {
                'category': row['category'],
                'limit': row['budget_limit'],
                'spent': row['spent'],
                'percentUsed': round(percent, 2),
                'alertLevel': alert_level
            }
        return None
    
    def get_all_budgets(self) -> List[Dict]:
        """Get all budgets with spending status"""
        rows = self.fetch_all(
            """SELECT c.name as category, b.budget_limit as budget_limit,
                      COALESCE(SUM(CASE WHEN t.type = 'expense' THEN t.amount ELSE 0 END), 0) as spent
               FROM budgets b
               JOIN categories c ON b.category_id = c.id
               LEFT JOIN transactions t ON t.category_id = c.id AND t.type = 'expense'
               GROUP BY b.id, c.name, b.budget_limit"""
        )
        
        budgets = []
        for row in rows:
            percent = (row['spent'] / row['budget_limit'] * 100) if row['budget_limit'] > 0 else 0
            alert_level = 'exceeded' if percent >= 100 else 'warning' if percent >= 80 else 'caution' if percent >= 50 else 'normal'
            budgets.append({
                'category': row['category'],
                'limit': row['budget_limit'],
                'spent': row['spent'],
                'percentUsed': round(percent, 2),
                'alertLevel': alert_level
            })
        return budgets
    
    def get_budget_alerts(self) -> List[Dict]:
        """Get budgets that need attention (using Indexed Priority Queue concept)"""
        budgets = self.get_all_budgets()
        # Sort by percentUsed descending (priority queue behavior)
        alerts = [b for b in budgets if b['alertLevel'] != 'normal']
        alerts.sort(key=lambda x: x['percentUsed'], reverse=True)
        
        result = []
        for b in alerts:
            msg = f"Budget exceeded! Spent ${b['spent']:.0f} of ${b['limit']:.0f}" if b['alertLevel'] == 'exceeded' else \
                  f"Warning: {b['percentUsed']:.0f}% of budget used" if b['alertLevel'] == 'warning' else \
                  f"Caution: {b['percentUsed']:.0f}% of budget used"
            result.append({
                'category': b['category'],
                'level': b['alertLevel'],
                'percentUsed': b['percentUsed'],
                'spent': b['spent'],
                'limit': b['limit'],
                'message': msg,
                'priority': b['percentUsed']  # For priority queue ordering
            })
        return result
    
    # ==================== BILL OPERATIONS ====================
    
    def add_bill(self, name: str, amount: float, due_date: str, category: str) -> Dict:
        """Add a bill to the queue"""
        bill_id = f"bill_{uuid.uuid4().hex[:8]}"
        cat_id = self.get_or_create_category(category)
        
        self.execute(
            """INSERT INTO bills (id, name, amount, due_date, category_id)
               VALUES (?, ?, ?, ?, ?)""",
            (bill_id, name, amount, due_date, cat_id)
        )
        
        self._push_undo(4, f"{bill_id}|{name}|{amount}|{due_date}|{category}")
        
        return {
            'id': bill_id, 'name': name, 'amount': amount,
            'dueDate': due_date, 'category': category, 'isPaid': False
        }
    
    def get_all_bills(self) -> List[Dict]:
        """Get all bills (FIFO order by creation)"""
        rows = self.fetch_all(
            """SELECT b.id, b.name, b.amount, b.due_date as dueDate, 
                      c.name as category, b.is_paid as isPaid
               FROM bills b
               JOIN categories c ON b.category_id = c.id
               ORDER BY b.created_at ASC"""
        )
        return [dict(row, isPaid=bool(row['isPaid'])) for row in rows]
    
    def pay_bill(self, bill_id: str) -> bool:
        """Mark a bill as paid"""
        self._push_undo(6, bill_id)
        result = self.execute(
            "UPDATE bills SET is_paid = 1, paid_at = CURRENT_TIMESTAMP WHERE id = ?",
            (bill_id,)
        )
        return result.rowcount > 0
    
    def delete_bill(self, bill_id: str) -> bool:
        """Delete a bill"""
        bill = self.fetch_one(
            """SELECT b.*, c.name as category FROM bills b 
               JOIN categories c ON b.category_id = c.id WHERE b.id = ?""",
            (bill_id,)
        )
        if bill:
            self._push_undo(5, f"{bill['id']}|{bill['name']}|{bill['amount']}|{bill['due_date']}|{bill['category']}")
            self.execute("DELETE FROM bills WHERE id = ?", (bill_id,))
            return True
        return False
    
    # ==================== ANALYTICS ====================
    
    def get_top_expenses(self, count: int = 5) -> List[Dict]:
        """Get top expenses (IntroSort simulation via SQL ORDER BY)"""
        return self.fetch_all(
            """SELECT t.id, t.type, t.amount, c.name as category, t.description, t.date
               FROM transactions t
               JOIN categories c ON t.category_id = c.id
               WHERE t.type = 'expense'
               ORDER BY t.amount DESC
               LIMIT ?""",
            (count,)
        )
    
    def get_top_categories(self, count: int = 5) -> List[Dict]:
        """Get top spending categories"""
        return self.fetch_all(
            """SELECT c.name as category, SUM(t.amount) as totalAmount
               FROM transactions t
               JOIN categories c ON t.category_id = c.id
               WHERE t.type = 'expense'
               GROUP BY c.name
               ORDER BY totalAmount DESC
               LIMIT ?""",
            (count,)
        )
    
    def get_monthly_summary(self, year_month: str = None) -> Dict:
        """Get monthly summary"""
        if not year_month:
            year_month = datetime.now().strftime('%Y-%m')
        
        start_date = f"{year_month}-01"
        end_date = f"{year_month}-31"
        
        summary = self.fetch_one(
            """SELECT 
                  COALESCE(SUM(CASE WHEN type = 'income' THEN amount ELSE 0 END), 0) as totalIncome,
                  COALESCE(SUM(CASE WHEN type = 'expense' THEN amount ELSE 0 END), 0) as totalExpenses,
                  COUNT(*) as transactionCount
               FROM transactions
               WHERE date BETWEEN ? AND ?""",
            (start_date, end_date)
        )
        
        category_breakdown = self.fetch_all(
            """SELECT c.name as category, SUM(t.amount) as amount
               FROM transactions t
               JOIN categories c ON t.category_id = c.id
               WHERE t.type = 'expense' AND t.date BETWEEN ? AND ?
               GROUP BY c.name
               ORDER BY amount DESC""",
            (start_date, end_date)
        )
        
        return {
            'month': year_month,
            'totalIncome': summary['totalIncome'],
            'totalExpenses': summary['totalExpenses'],
            'netSavings': summary['totalIncome'] - summary['totalExpenses'],
            'transactionCount': summary['transactionCount'],
            'categoryBreakdown': category_breakdown
        }
    
    # ==================== SLIDING WINDOW ANALYTICS ====================
    
    def _update_daily_spending(self, date: str, tx_type: str, amount: float, count_delta: int):
        """Update daily spending aggregates"""
        existing = self.fetch_one("SELECT * FROM daily_spending WHERE date = ?", (date,))
        
        if existing:
            income_delta = amount if tx_type == 'income' else 0
            expense_delta = amount if tx_type == 'expense' else 0
            
            new_count = existing['transaction_count'] + count_delta
            new_income = existing['total_income'] + income_delta
            new_expense = existing['total_expenses'] + expense_delta
            
            # If no transactions left for this date, remove the entry
            if new_count <= 0:
                self.execute("DELETE FROM daily_spending WHERE date = ?", (date,))
            else:
                # Ensure values don't go negative
                self.execute(
                    """UPDATE daily_spending 
                       SET total_income = MAX(0, total_income + ?),
                           total_expenses = MAX(0, total_expenses + ?),
                           transaction_count = MAX(0, transaction_count + ?)
                       WHERE date = ?""",
                    (income_delta, expense_delta, count_delta, date)
                )
        else:
            # Only insert if adding (positive amount)
            if amount > 0:
                income = amount if tx_type == 'income' else 0
                expense = amount if tx_type == 'expense' else 0
                self.execute(
                    """INSERT INTO daily_spending (date, total_income, total_expenses, transaction_count)
                       VALUES (?, ?, ?, ?)""",
                    (date, income, expense, max(0, count_delta))
                )
    
    def get_spending_trend(self, days: int = 7) -> Dict:
        """Get spending trend using sliding window concept"""
        end_date = datetime.now().strftime('%Y-%m-%d')
        start_date = (datetime.now() - timedelta(days=days-1)).strftime('%Y-%m-%d')
        
        # Get daily data directly from transactions (more reliable than aggregates)
        daily_data = self.fetch_all(
            """SELECT date,
                      SUM(CASE WHEN type = 'income' THEN amount ELSE 0 END) as total_income,
                      SUM(CASE WHEN type = 'expense' THEN amount ELSE 0 END) as total_expenses,
                      COUNT(*) as transaction_count
               FROM transactions
               WHERE date BETWEEN ? AND ?
               GROUP BY date
               ORDER BY date ASC""",
            (start_date, end_date)
        )
        
        # Calculate totals and moving average
        total_expenses = sum(d['total_expenses'] for d in daily_data)
        total_income = sum(d['total_income'] for d in daily_data)
        avg_daily_expense = total_expenses / days if days > 0 else 0
        
        # Calculate trend (positive = increasing spending)
        if len(daily_data) >= 2:
            first_half = sum(d['total_expenses'] for d in daily_data[:len(daily_data)//2])
            second_half = sum(d['total_expenses'] for d in daily_data[len(daily_data)//2:])
            trend = 'increasing' if second_half > first_half * 1.1 else 'decreasing' if second_half < first_half * 0.9 else 'stable'
        else:
            trend = 'stable'
        
        return {
            'period': f'{days}-day',
            'startDate': start_date,
            'endDate': end_date,
            'totalExpenses': round(total_expenses, 2),
            'totalIncome': round(total_income, 2),
            'avgDailyExpense': round(avg_daily_expense, 2),
            'trend': trend,
            'dailyData': daily_data
        }
    
    # ==================== Z-SCORE ANOMALY DETECTION ====================
    
    def _update_spending_stats(self, category_id: str, amount: float):
        """Update running statistics for Z-Score calculation (Welford's algorithm)"""
        existing = self.fetch_one(
            "SELECT * FROM spending_stats WHERE category_id = ?", (category_id,)
        )
        
        if existing:
            n = existing['transaction_count'] + 1
            old_mean = existing['mean_amount']
            new_mean = old_mean + (amount - old_mean) / n
            
            old_sum_sq = existing['sum_squared']
            new_sum_sq = old_sum_sq + (amount - old_mean) * (amount - new_mean)
            
            std_dev = math.sqrt(new_sum_sq / n) if n > 0 else 0
            
            self.execute(
                """UPDATE spending_stats 
                   SET mean_amount = ?, std_dev = ?, transaction_count = ?,
                       sum_amount = sum_amount + ?, sum_squared = ?,
                       updated_at = CURRENT_TIMESTAMP
                   WHERE category_id = ?""",
                (new_mean, std_dev, n, amount, new_sum_sq, category_id)
            )
        else:
            self.execute(
                """INSERT INTO spending_stats 
                   (category_id, mean_amount, std_dev, transaction_count, sum_amount, sum_squared)
                   VALUES (?, ?, 0, 1, ?, 0)""",
                (category_id, amount, amount)
            )
    
    def _remove_from_spending_stats(self, category: str, amount: float):
        """Remove transaction from spending stats when deleted - recalculates for accuracy"""
        cat_id = self.get_or_create_category(category)
        
        # Get all remaining expense transactions for this category
        transactions = self.fetch_all(
            """SELECT t.amount
               FROM transactions t
               WHERE t.category_id = ? AND t.type = 'expense'""",
            (cat_id,)
        )
        
        if len(transactions) == 0:
            # No transactions left, delete stats
            self.execute("DELETE FROM spending_stats WHERE category_id = ?", (cat_id,))
            return
        
        # Recalculate stats from scratch for this category using Welford's algorithm
        n = 0
        mean = 0.0
        sum_sq = 0.0
        total_sum = 0.0
        
        for tx in transactions:
            n += 1
            total_sum += tx['amount']
            old_mean = mean
            mean = old_mean + (tx['amount'] - old_mean) / n
            sum_sq = sum_sq + (tx['amount'] - old_mean) * (tx['amount'] - mean)
        
        std_dev = math.sqrt(sum_sq / n) if n > 0 and sum_sq > 0 else 0
        
        existing = self.fetch_one(
            "SELECT * FROM spending_stats WHERE category_id = ?", (cat_id,)
        )
        
        if existing:
            self.execute(
                """UPDATE spending_stats 
                   SET mean_amount = ?, std_dev = ?, transaction_count = ?,
                       sum_amount = ?, sum_squared = ?,
                       updated_at = CURRENT_TIMESTAMP
                   WHERE category_id = ?""",
                (mean, std_dev, n, total_sum, sum_sq, cat_id)
            )
        else:
            self.execute(
                """INSERT INTO spending_stats 
                   (category_id, mean_amount, std_dev, transaction_count, sum_amount, sum_squared)
                   VALUES (?, ?, ?, ?, ?, ?)""",
                (cat_id, mean, std_dev, n, total_sum, sum_sq)
            )
    
    def recalculate_spending_stats(self):
        """Recalculate all spending stats from transactions (for data consistency)"""
        # Clear all stats
        self.execute("DELETE FROM spending_stats")
        
        # Recalculate from transactions
        transactions = self.fetch_all(
            """SELECT t.amount, c.id as category_id
               FROM transactions t
               JOIN categories c ON t.category_id = c.id
               WHERE t.type = 'expense'
               ORDER BY t.created_at ASC"""
        )
        
        for tx in transactions:
            self._update_spending_stats(tx['category_id'], tx['amount'])
    
    def recalculate_daily_spending(self):
        """Recalculate all daily spending aggregates from transactions (for data consistency)"""
        # Clear all daily spending
        self.execute("DELETE FROM daily_spending")
        
        # Recalculate from transactions grouped by date
        daily_data = self.fetch_all(
            """SELECT date,
                      SUM(CASE WHEN type = 'income' THEN amount ELSE 0 END) as total_income,
                      SUM(CASE WHEN type = 'expense' THEN amount ELSE 0 END) as total_expenses,
                      COUNT(*) as transaction_count
               FROM transactions
               GROUP BY date
               ORDER BY date ASC"""
        )
        
        for day in daily_data:
            self.execute(
                """INSERT INTO daily_spending (date, total_income, total_expenses, transaction_count)
                   VALUES (?, ?, ?, ?)""",
                (day['date'], day['total_income'], day['total_expenses'], day['transaction_count'])
            )
    
    def _detect_anomaly_internal(self, cat_id: str, category: str, amount: float, threshold: float = 2.0) -> Dict:
        """Internal anomaly detection before stats are updated"""
        stats = self.fetch_one(
            "SELECT * FROM spending_stats WHERE category_id = ?", (cat_id,)
        )
        
        if not stats or stats['transaction_count'] < 3 or stats['std_dev'] == 0:
            return {
                'isAnomaly': False,
                'zScore': 0,
                'message': 'Insufficient data for anomaly detection',
                'category': category,
                'amount': amount
            }
        
        z_score = (amount - stats['mean_amount']) / stats['std_dev']
        is_anomaly = abs(z_score) > threshold
        
        if is_anomaly:
            if z_score > 0:
                msg = f"Unusually HIGH expense: ${amount:.2f} is {z_score:.1f} std devs above average ${stats['mean_amount']:.2f}"
            else:
                msg = f"Unusually LOW expense: ${amount:.2f} is {abs(z_score):.1f} std devs below average ${stats['mean_amount']:.2f}"
        else:
            msg = f"Normal expense within expected range for {category}"
        
        return {
            'isAnomaly': is_anomaly,
            'zScore': round(z_score, 2),
            'message': msg,
            'category': category,
            'amount': amount,
            'mean': round(stats['mean_amount'], 2),
            'stdDev': round(stats['std_dev'], 2)
        }
    
    def detect_anomaly(self, category: str, amount: float, threshold: float = 2.0) -> Dict:
        """Detect if a transaction amount is anomalous using Z-Score"""
        cat_id = self.get_or_create_category(category)
        stats = self.fetch_one(
            "SELECT * FROM spending_stats WHERE category_id = ?", (cat_id,)
        )
        
        if not stats or stats['transaction_count'] < 3 or stats['std_dev'] == 0:
            return {
                'isAnomaly': False,
                'zScore': 0,
                'message': 'Insufficient data for anomaly detection',
                'category': category,
                'amount': amount
            }
        
        z_score = (amount - stats['mean_amount']) / stats['std_dev']
        is_anomaly = abs(z_score) > threshold
        
        if is_anomaly:
            if z_score > 0:
                msg = f"Unusually HIGH expense: ${amount:.2f} is {z_score:.1f} std devs above average ${stats['mean_amount']:.2f}"
            else:
                msg = f"Unusually LOW expense: ${amount:.2f} is {abs(z_score):.1f} std devs below average ${stats['mean_amount']:.2f}"
        else:
            msg = f"Normal expense within expected range for {category}"
        
        return {
            'isAnomaly': is_anomaly,
            'zScore': round(z_score, 2),
            'message': msg,
            'category': category,
            'amount': amount,
            'mean': round(stats['mean_amount'], 2),
            'stdDev': round(stats['std_dev'], 2)
        }
    
    def get_all_anomalies(self, threshold: float = 2.0) -> List[Dict]:
        """Get all transactions flagged as anomalies at creation time"""
        # Get transactions that were flagged as anomalies when added
        transactions = self.fetch_all(
            """SELECT t.id, t.amount, c.name as category, t.date, t.description,
                      t.is_anomaly, t.z_score
               FROM transactions t
               JOIN categories c ON t.category_id = c.id
               WHERE t.type = 'expense' AND t.is_anomaly = 1
               ORDER BY t.created_at DESC
               LIMIT 100"""
        )
        
        anomalies = []
        for tx in transactions:
            anomalies.append({
                'id': tx['id'],
                'amount': tx['amount'],
                'category': tx['category'],
                'date': tx['date'],
                'description': tx['description'],
                'zScore': round(tx['z_score'], 2),
                'isHigh': tx['z_score'] > 0
            })
        
        return anomalies
    
    # ==================== UNDO OPERATIONS ====================
    
    def _push_undo(self, action_type: int, data: str):
        """Push action to undo stack"""
        self.execute(
            "INSERT INTO undo_actions (action_type, action_data) VALUES (?, ?)",
            (action_type, data)
        )
        # Keep only last 50 actions
        self.execute(
            """DELETE FROM undo_actions WHERE id NOT IN (
                SELECT id FROM undo_actions ORDER BY id DESC LIMIT 50
            )"""
        )
    
    def undo(self) -> bool:
        """Undo last action"""
        action = self.fetch_one(
            "SELECT * FROM undo_actions ORDER BY id DESC LIMIT 1"
        )
        if not action:
            return False
        
        self.execute("DELETE FROM undo_actions WHERE id = ?", (action['id'],))
        
        parts = action['action_data'].split('|')
        action_type = action['action_type']
        
        if action_type == 0:  # ADD_TRANSACTION - undo by deleting
            tx_id = parts[0]
            self.execute("DELETE FROM transactions WHERE id = ?", (tx_id,))
        elif action_type == 1:  # DELETE_TRANSACTION - undo by re-adding
            tx_id, tx_type, amount, category, description, date = parts
            cat_id = self.get_or_create_category(category)
            self.execute(
                "INSERT INTO transactions (id, type, amount, category_id, description, date) VALUES (?, ?, ?, ?, ?, ?)",
                (tx_id, tx_type, float(amount), cat_id, description, date)
            )
        elif action_type == 2:  # ADD_BUDGET - undo by deleting
            category = parts[0]
            cat_id = self.get_or_create_category(category)
            self.execute("DELETE FROM budgets WHERE category_id = ?", (cat_id,))
        elif action_type == 3:  # UPDATE_BUDGET - restore old value
            category, old_limit = parts
            cat_id = self.get_or_create_category(category)
            self.execute(
                "UPDATE budgets SET budget_limit = ? WHERE category_id = ?",
                (float(old_limit), cat_id)
            )
        
        return True
    
    def can_undo(self) -> bool:
        """Check if undo is available"""
        count = self.fetch_one("SELECT COUNT(*) as cnt FROM undo_actions")
        return count['cnt'] > 0
    
    # ==================== DASHBOARD ====================
    
    def get_dashboard(self) -> Dict:
        """Get dashboard summary"""
        totals = self.fetch_one(
            """SELECT 
                  COALESCE(SUM(CASE WHEN type = 'income' THEN amount ELSE 0 END), 0) as totalIncome,
                  COALESCE(SUM(CASE WHEN type = 'expense' THEN amount ELSE 0 END), 0) as totalExpenses,
                  COUNT(*) as transactionCount
               FROM transactions"""
        )
        
        budget_count = self.fetch_one("SELECT COUNT(*) as cnt FROM budgets")['cnt']
        bill_count = self.fetch_one("SELECT COUNT(*) as cnt FROM bills WHERE is_paid = 0")['cnt']
        
        return {
            'balance': totals['totalIncome'] - totals['totalExpenses'],
            'totalIncome': totals['totalIncome'],
            'totalExpenses': totals['totalExpenses'],
            'transactionCount': totals['transactionCount'],
            'budgetCount': budget_count,
            'billCount': bill_count,
            'canUndo': self.can_undo()
        }


# Singleton instance
_db_instance = None

def get_db() -> DatabaseManager:
    """Get database manager instance"""
    global _db_instance
    if _db_instance is None:
        _db_instance = DatabaseManager()
    return _db_instance
