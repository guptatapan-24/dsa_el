"""
Migration script to convert JSON data to SQLite
Preserves existing demo data during migration
"""

import json
import os
from pathlib import Path
import sys

# Add parent to path for imports
sys.path.insert(0, str(Path(__file__).parent.parent))

from database.db_manager import DatabaseManager, DB_PATH, SCHEMA_PATH

DATA_DIR = Path(__file__).parent.parent / "data"


def migrate():
    """Migrate JSON data to SQLite database"""
    print("Starting migration from JSON to SQLite...")
    
    # Remove existing database to start fresh
    if os.path.exists(DB_PATH):
        os.remove(DB_PATH)
        print(f"Removed existing database: {DB_PATH}")
    
    # Create new database with schema
    db = DatabaseManager()
    print(f"Created new database: {DB_PATH}")
    
    # Load and migrate transactions
    transactions_file = DATA_DIR / "transactions.json"
    if transactions_file.exists():
        with open(transactions_file, 'r') as f:
            data = json.load(f)
            transactions = data.get('transactions', [])
            print(f"Migrating {len(transactions)} transactions...")
            
            for tx in transactions:
                # Get or create category
                cat_id = db.get_or_create_category(tx['category'])
                
                # Insert transaction directly (bypass undo stack for migration)
                db.execute(
                    """INSERT INTO transactions (id, type, amount, category_id, description, date)
                       VALUES (?, ?, ?, ?, ?, ?)""",
                    (tx['id'], tx['type'], tx['amount'], cat_id, 
                     tx.get('description', ''), tx['date'])
                )
                
                # Update daily spending
                db._update_daily_spending(tx['date'], tx['type'], tx['amount'], 1)
                
                # Update spending stats for expenses
                if tx['type'] == 'expense':
                    db._update_spending_stats(cat_id, tx['amount'])
            
            print(f"  ✓ Migrated {len(transactions)} transactions")
    
    # Load and migrate budgets
    budgets_file = DATA_DIR / "budgets.json"
    if budgets_file.exists():
        with open(budgets_file, 'r') as f:
            data = json.load(f)
            budgets = data.get('budgets', [])
            print(f"Migrating {len(budgets)} budgets...")
            
            for budget in budgets:
                cat_id = db.get_or_create_category(budget['category'])
                budget_id = f"budget_{budget['category'].lower()[:8]}"
                
                db.execute(
                    """INSERT OR REPLACE INTO budgets (id, category_id, budget_limit)
                       VALUES (?, ?, ?)""",
                    (budget_id, cat_id, budget['limit'])
                )
            
            print(f"  ✓ Migrated {len(budgets)} budgets")
    
    # Load and migrate bills
    bills_file = DATA_DIR / "bills.json"
    if bills_file.exists():
        with open(bills_file, 'r') as f:
            data = json.load(f)
            bills = data.get('bills', [])
            print(f"Migrating {len(bills)} bills...")
            
            for bill in bills:
                cat_id = db.get_or_create_category(bill['category'])
                
                db.execute(
                    """INSERT INTO bills (id, name, amount, due_date, category_id, is_paid)
                       VALUES (?, ?, ?, ?, ?, ?)""",
                    (bill['id'], bill['name'], bill['amount'], 
                     bill['dueDate'], cat_id, 1 if bill.get('isPaid', False) else 0)
                )
            
            print(f"  ✓ Migrated {len(bills)} bills")
    
    # Add default categories that may not be in transactions
    default_categories = [
        "Food", "Transport", "Shopping", "Entertainment", "Bills",
        "Healthcare", "Education", "Salary", "Freelance", "Investment",
        "Rent", "Utilities", "Groceries", "Dining", "Travel"
    ]
    
    for cat in default_categories:
        db.get_or_create_category(cat)
    
    print(f"  ✓ Added {len(default_categories)} default categories")
    
    # Verify migration
    print("\nVerification:")
    dashboard = db.get_dashboard()
    print(f"  Total transactions: {dashboard['transactionCount']}")
    print(f"  Total income: ${dashboard['totalIncome']:.2f}")
    print(f"  Total expenses: ${dashboard['totalExpenses']:.2f}")
    print(f"  Balance: ${dashboard['balance']:.2f}")
    print(f"  Budgets: {dashboard['budgetCount']}")
    print(f"  Unpaid bills: {dashboard['billCount']}")
    
    categories = db.get_all_categories()
    print(f"  Categories: {len(categories)}")
    
    print("\n✅ Migration completed successfully!")
    return True


if __name__ == "__main__":
    migrate()
