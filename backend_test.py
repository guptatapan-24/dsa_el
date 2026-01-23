#!/usr/bin/env python3
"""
Backend API Testing for Smart Personal Finance Tracker
Comprehensive testing of ALL FastAPI endpoints with SQLite & DSA integration
"""

import requests
import json
import sys
import os
from datetime import datetime, timedelta
from pathlib import Path

# Read backend URL from frontend/.env
def get_backend_url():
    env_path = Path(__file__).parent / "frontend" / ".env"
    if env_path.exists():
        with open(env_path, 'r') as f:
            for line in f:
                if line.startswith('REACT_APP_BACKEND_URL='):
                    return line.split('=', 1)[1].strip() + "/api"
    return "http://localhost:8001/api"

BACKEND_URL = get_backend_url()

class FinanceTrackerTester:
    def __init__(self):
        self.base_url = BACKEND_URL
        self.session = requests.Session()
        self.test_results = []
        self.created_transaction_id = None
        self.created_bill_id = None
        
    def log_test(self, test_name, success, details="", response_data=None):
        """Log test results"""
        status = "✅ PASS" if success else "❌ FAIL"
        print(f"{status} {test_name}")
        if details:
            print(f"   Details: {details}")
        if response_data and not success:
            print(f"   Response: {response_data}")
        print()
        
        self.test_results.append({
            "test": test_name,
            "success": success,
            "details": details,
            "response": response_data
        })
    
    def make_request(self, method, endpoint, data=None, params=None):
        """Make HTTP request with error handling"""
        url = f"{self.base_url}{endpoint}"
        try:
            if method.upper() == "GET":
                response = self.session.get(url, params=params, timeout=30)
            elif method.upper() == "POST":
                response = self.session.post(url, json=data, timeout=30)
            elif method.upper() == "DELETE":
                response = self.session.delete(url, timeout=30)
            else:
                raise ValueError(f"Unsupported method: {method}")
            
            return response
        except requests.exceptions.RequestException as e:
            return None, str(e)
    
    def test_root_and_health(self):
        """Test root and health endpoints"""
        print("🔍 Testing Root & Health Endpoints...")
        
        # Test GET /api/ (root)
        response = self.make_request("GET", "/")
        if isinstance(response, tuple):
            self.log_test("Root Endpoint", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            message = data.get("message", "")
            if "Finance Tracker API" in message:
                self.log_test("Root Endpoint", True, f"Message: {message}")
            else:
                self.log_test("Root Endpoint", False, f"Unexpected message: {message}", data)
        else:
            self.log_test("Root Endpoint", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/health
        response = self.make_request("GET", "/health")
        if isinstance(response, tuple):
            self.log_test("Health Check", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            status = data.get("status", "unknown")
            database = data.get("database", "unknown")
            data_structures = data.get("dataStructures", [])
            
            if status == "healthy" and database == "sqlite" and len(data_structures) >= 7:
                self.log_test("Health Check", True, f"Status: {status}, DB: {database}, DSA: {len(data_structures)} structures")
            else:
                self.log_test("Health Check", False, f"Status: {status}, DB: {database}, DSA count: {len(data_structures)}", data)
        else:
            self.log_test("Health Check", False, f"HTTP {response.status_code}", response.text)
    
    def test_dashboard_endpoint(self):
        """Test /api/dashboard endpoint"""
        print("🔍 Testing Dashboard Endpoint...")
        response = self.make_request("GET", "/dashboard")
        
        if isinstance(response, tuple):
            self.log_test("Dashboard Data", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            required_fields = ["balance", "totalIncome", "totalExpenses", "transactionCount", "budgetCount", "billCount", "canUndo"]
            
            missing_fields = [field for field in required_fields if field not in data]
            if missing_fields:
                self.log_test("Dashboard Data", False, f"Missing fields: {missing_fields}", data)
                return False
            
            # Check if values are reasonable (demo data should exist)
            balance = data.get("balance", 0)
            income = data.get("totalIncome", 0)
            expenses = data.get("totalExpenses", 0)
            tx_count = data.get("transactionCount", 0)
            
            self.log_test("Dashboard Data", True, 
                         f"Balance: ${balance:.2f}, Income: ${income:.2f}, Expenses: ${expenses:.2f}, Transactions: {tx_count}")
            return True
        else:
            self.log_test("Dashboard Data", False, f"HTTP {response.status_code}", response.text)
            return False
    
    def test_transactions_endpoints(self):
        """Test all transaction-related endpoints"""
        print("🔍 Testing Transaction Endpoints...")
        
        # Test GET /api/transactions
        response = self.make_request("GET", "/transactions")
        if isinstance(response, tuple):
            self.log_test("Get All Transactions", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            transactions = data.get("transactions", [])
            ds_info = data.get("dsInfo", "")
            self.log_test("Get All Transactions", True, f"Found {len(transactions)} transactions. DSA: {ds_info}")
        else:
            self.log_test("Get All Transactions", False, f"HTTP {response.status_code}", response.text)
            return False
        
        # Test POST /api/transactions (add new transaction)
        new_transaction = {
            "type": "expense",
            "amount": 125.50,
            "category": "Groceries",
            "description": "Weekly grocery shopping",
            "date": "2025-01-16"
        }
        
        response = self.make_request("POST", "/transactions", new_transaction)
        if isinstance(response, tuple):
            self.log_test("Add Transaction", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            if data.get("success") and "transaction" in data and "id" in data["transaction"]:
                self.created_transaction_id = data["transaction"]["id"]
                anomaly = data.get("anomaly")
                ds_info = data.get("dsInfo", "")
                self.log_test("Add Transaction", True, 
                             f"Created transaction ID: {self.created_transaction_id}. Anomaly: {anomaly is not None}. DSA: {ds_info}")
            else:
                self.log_test("Add Transaction", False, "Invalid response structure", data)
                return False
        else:
            self.log_test("Add Transaction", False, f"HTTP {response.status_code}", response.text)
            return False
        
        # Test GET /api/transactions/recent?count=5
        response = self.make_request("GET", "/transactions/recent", params={"count": 5})
        if isinstance(response, tuple):
            self.log_test("Get Recent Transactions", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            recent = data.get("transactions", [])
            ds_info = data.get("dsInfo", "")
            self.log_test("Get Recent Transactions", True, f"Found {len(recent)} recent transactions. DSA: {ds_info}")
        else:
            self.log_test("Get Recent Transactions", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/transactions/range?start_date=2025-01-01&end_date=2025-12-31
        start_date = "2025-01-01"
        end_date = "2025-12-31"
        response = self.make_request("GET", "/transactions/range", 
                                   params={"start_date": start_date, "end_date": end_date})
        if isinstance(response, tuple):
            self.log_test("Get Transactions by Range", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            range_transactions = data.get("transactions", [])
            ds_info = data.get("dsInfo", "")
            self.log_test("Get Transactions by Range", True, 
                         f"Found {len(range_transactions)} transactions in range {start_date} to {end_date}. DSA: {ds_info}")
        else:
            self.log_test("Get Transactions by Range", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/transactions/{transaction_id} (get by ID)
        if self.created_transaction_id:
            response = self.make_request("GET", f"/transactions/{self.created_transaction_id}")
            if isinstance(response, tuple):
                self.log_test("Get Transaction by ID", False, f"Request failed: {response[1]}")
            elif response.status_code == 200:
                data = response.json()
                transaction = data.get("transaction")
                ds_info = data.get("dsInfo", "")
                if transaction and transaction.get("id") == self.created_transaction_id:
                    self.log_test("Get Transaction by ID", True, f"Retrieved transaction {self.created_transaction_id}. DSA: {ds_info}")
                else:
                    self.log_test("Get Transaction by ID", False, "Transaction ID mismatch", data)
            else:
                self.log_test("Get Transaction by ID", False, f"HTTP {response.status_code}", response.text)
        
        # Test DELETE /api/transactions/{transaction_id}
        if self.created_transaction_id:
            response = self.make_request("DELETE", f"/transactions/{self.created_transaction_id}")
            if isinstance(response, tuple):
                self.log_test("Delete Transaction", False, f"Request failed: {response[1]}")
            elif response.status_code == 200:
                data = response.json()
                if data.get("success"):
                    self.log_test("Delete Transaction", True, f"Deleted transaction {self.created_transaction_id}")
                else:
                    self.log_test("Delete Transaction", False, "Delete operation failed", data)
            else:
                self.log_test("Delete Transaction", False, f"HTTP {response.status_code}", response.text)
        
        return True
    
    def test_budgets_endpoints(self):
        """Test all budget-related endpoints"""
        print("🔍 Testing Budget Endpoints...")
        
        # Test GET /api/budgets
        response = self.make_request("GET", "/budgets")
        if isinstance(response, tuple):
            self.log_test("Get All Budgets", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            budgets = data.get("budgets", [])
            ds_info = data.get("dsInfo", "")
            self.log_test("Get All Budgets", True, f"Found {len(budgets)} budgets. DSA: {ds_info}")
        else:
            self.log_test("Get All Budgets", False, f"HTTP {response.status_code}", response.text)
            return False
        
        # Test POST /api/budgets (set budget)
        new_budget = {
            "category": "Entertainment",
            "limit": 300.0
        }
        
        response = self.make_request("POST", "/budgets", new_budget)
        if isinstance(response, tuple):
            self.log_test("Set Budget", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            if data.get("success") and "budget" in data:
                budget = data["budget"]
                ds_info = data.get("dsInfo", "")
                self.log_test("Set Budget", True, 
                             f"Set budget for {new_budget['category']}: ${new_budget['limit']}. DSA: {ds_info}")
            else:
                self.log_test("Set Budget", False, "Invalid response structure", data)
        else:
            self.log_test("Set Budget", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/budgets/alerts
        response = self.make_request("GET", "/budgets/alerts")
        if isinstance(response, tuple):
            self.log_test("Get Budget Alerts", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            alerts = data.get("alerts", [])
            ds_info = data.get("dsInfo", "")
            self.log_test("Get Budget Alerts", True, f"Found {len(alerts)} budget alerts. DSA: {ds_info}")
        else:
            self.log_test("Get Budget Alerts", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/alerts (shortcut route)
        response = self.make_request("GET", "/alerts")
        if isinstance(response, tuple):
            self.log_test("Get Alerts (shortcut)", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            alerts = data.get("alerts", [])
            self.log_test("Get Alerts (shortcut)", True, f"Found {len(alerts)} alerts via shortcut route")
        else:
            self.log_test("Get Alerts (shortcut)", False, f"HTTP {response.status_code}", response.text)
        
        return True
    
    def test_bills_endpoints(self):
        """Test all bill-related endpoints"""
        print("🔍 Testing Bill Endpoints...")
        
        # Test GET /api/bills
        response = self.make_request("GET", "/bills")
        if isinstance(response, tuple):
            self.log_test("Get All Bills", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            bills = data.get("bills", [])
            ds_info = data.get("dsInfo", "")
            self.log_test("Get All Bills", True, f"Found {len(bills)} bills. DSA: {ds_info}")
        else:
            self.log_test("Get All Bills", False, f"HTTP {response.status_code}", response.text)
            return False
        
        # Test POST /api/bills (add bill)
        new_bill = {
            "name": "Internet Bill",
            "amount": 89.99,
            "dueDate": "2025-02-15",
            "category": "Utilities"
        }
        
        response = self.make_request("POST", "/bills", new_bill)
        if isinstance(response, tuple):
            self.log_test("Add Bill", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            if data.get("success") and "bill" in data and "id" in data["bill"]:
                self.created_bill_id = data["bill"]["id"]
                self.log_test("Add Bill", True, f"Created bill ID: {self.created_bill_id} - {new_bill['name']}")
            else:
                self.log_test("Add Bill", False, "Invalid response structure", data)
                return False
        else:
            self.log_test("Add Bill", False, f"HTTP {response.status_code}", response.text)
            return False
        
        # Test POST /api/bills/{bill_id}/pay (mark paid)
        if self.created_bill_id:
            response = self.make_request("POST", f"/bills/{self.created_bill_id}/pay")
            if isinstance(response, tuple):
                self.log_test("Pay Bill", False, f"Request failed: {response[1]}")
            elif response.status_code == 200:
                data = response.json()
                if data.get("success"):
                    self.log_test("Pay Bill", True, f"Paid bill {self.created_bill_id}")
                else:
                    self.log_test("Pay Bill", False, "Pay operation failed", data)
            else:
                self.log_test("Pay Bill", False, f"HTTP {response.status_code}", response.text)
        
        # Test DELETE /api/bills/{bill_id}
        if self.created_bill_id:
            response = self.make_request("DELETE", f"/bills/{self.created_bill_id}")
            if isinstance(response, tuple):
                self.log_test("Delete Bill", False, f"Request failed: {response[1]}")
            elif response.status_code == 200:
                data = response.json()
                if data.get("success"):
                    self.log_test("Delete Bill", True, f"Deleted bill {self.created_bill_id}")
                else:
                    self.log_test("Delete Bill", False, "Delete operation failed", data)
            else:
                self.log_test("Delete Bill", False, f"HTTP {response.status_code}", response.text)
        
        return True
    
    def test_analytics_endpoints(self):
        """Test all analytics endpoints"""
        print("🔍 Testing Analytics Endpoints...")
        
        # Test GET /api/top-expenses?count=5
        response = self.make_request("GET", "/top-expenses", params={"count": 5})
        if isinstance(response, tuple):
            self.log_test("Get Top Expenses", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            expenses = data.get("topExpenses", [])
            ds_info = data.get("dsInfo", "")
            self.log_test("Get Top Expenses", True, f"Found {len(expenses)} top expenses. DSA: {ds_info}")
        else:
            self.log_test("Get Top Expenses", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/top-categories?count=5
        response = self.make_request("GET", "/top-categories", params={"count": 5})
        if isinstance(response, tuple):
            self.log_test("Get Top Categories", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            categories = data.get("topCategories", [])
            ds_info = data.get("dsInfo", "")
            self.log_test("Get Top Categories", True, f"Found {len(categories)} top categories. DSA: {ds_info}")
        else:
            self.log_test("Get Top Categories", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/monthly-summary
        response = self.make_request("GET", "/monthly-summary")
        if isinstance(response, tuple):
            self.log_test("Get Monthly Summary (current)", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            summary = data.get("summary")
            ds_info = data.get("dsInfo", "")
            if summary:
                month = summary.get("month", "unknown")
                income = summary.get("totalIncome", 0)
                expenses = summary.get("totalExpenses", 0)
                self.log_test("Get Monthly Summary (current)", True, 
                             f"Month: {month}, Income: ${income:.2f}, Expenses: ${expenses:.2f}. DSA: {ds_info}")
            else:
                self.log_test("Get Monthly Summary (current)", False, "No summary data", data)
        else:
            self.log_test("Get Monthly Summary (current)", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/monthly-summary?month=2025-07
        response = self.make_request("GET", "/monthly-summary", params={"month": "2025-07"})
        if isinstance(response, tuple):
            self.log_test("Get Monthly Summary (specific)", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            summary = data.get("summary")
            ds_info = data.get("dsInfo", "")
            if summary:
                month = summary.get("month", "unknown")
                self.log_test("Get Monthly Summary (specific)", True, f"Retrieved summary for {month}. DSA: {ds_info}")
            else:
                self.log_test("Get Monthly Summary (specific)", False, "No summary data for July 2025", data)
        else:
            self.log_test("Get Monthly Summary (specific)", False, f"HTTP {response.status_code}", response.text)
        
        return True
    
    def test_spending_trends_endpoints(self):
        """Test spending trends (sliding window) endpoints"""
        print("🔍 Testing Spending Trends Endpoints...")
        
        # Test GET /api/trends/7-day
        response = self.make_request("GET", "/trends/7-day")
        if isinstance(response, tuple):
            self.log_test("Get 7-Day Trend", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            trend = data.get("trend")
            ds_info = data.get("dsInfo", "")
            if trend:
                period = trend.get("period", "unknown")
                total_expenses = trend.get("totalExpenses", 0)
                avg_daily = trend.get("avgDailyExpense", 0)
                self.log_test("Get 7-Day Trend", True, 
                             f"Period: {period}, Total: ${total_expenses:.2f}, Avg Daily: ${avg_daily:.2f}. DSA: {ds_info}")
            else:
                self.log_test("Get 7-Day Trend", False, "No trend data", data)
        else:
            self.log_test("Get 7-Day Trend", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/trends/30-day
        response = self.make_request("GET", "/trends/30-day")
        if isinstance(response, tuple):
            self.log_test("Get 30-Day Trend", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            trend = data.get("trend")
            ds_info = data.get("dsInfo", "")
            if trend:
                period = trend.get("period", "unknown")
                total_expenses = trend.get("totalExpenses", 0)
                avg_daily = trend.get("avgDailyExpense", 0)
                self.log_test("Get 30-Day Trend", True, 
                             f"Period: {period}, Total: ${total_expenses:.2f}, Avg Daily: ${avg_daily:.2f}. DSA: {ds_info}")
            else:
                self.log_test("Get 30-Day Trend", False, "No trend data", data)
        else:
            self.log_test("Get 30-Day Trend", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/trends/14 (custom days)
        response = self.make_request("GET", "/trends/14")
        if isinstance(response, tuple):
            self.log_test("Get Custom 14-Day Trend", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            trend = data.get("trend")
            ds_info = data.get("dsInfo", "")
            if trend:
                period = trend.get("period", "unknown")
                self.log_test("Get Custom 14-Day Trend", True, f"Period: {period}. DSA: {ds_info}")
            else:
                self.log_test("Get Custom 14-Day Trend", False, "No trend data", data)
        else:
            self.log_test("Get Custom 14-Day Trend", False, f"HTTP {response.status_code}", response.text)
        
        return True
    
    def test_anomaly_detection_endpoints(self):
        """Test anomaly detection (Z-Score) endpoints"""
        print("🔍 Testing Anomaly Detection Endpoints...")
        
        # Test GET /api/anomalies?threshold=2.0
        response = self.make_request("GET", "/anomalies", params={"threshold": 2.0})
        if isinstance(response, tuple):
            self.log_test("Get Anomalies", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            anomalies = data.get("anomalies", [])
            threshold = data.get("threshold", 0)
            ds_info = data.get("dsInfo", "")
            self.log_test("Get Anomalies", True, 
                         f"Found {len(anomalies)} anomalies with threshold {threshold}. DSA: {ds_info}")
        else:
            self.log_test("Get Anomalies", False, f"HTTP {response.status_code}", response.text)
        
        # Test POST /api/anomalies/check?category=Food&amount=500&threshold=2.0
        response = self.make_request("POST", "/anomalies/check", 
                                   params={"category": "Food", "amount": 500, "threshold": 2.0})
        if isinstance(response, tuple):
            self.log_test("Check Anomaly", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            result = data.get("result")
            ds_info = data.get("dsInfo", "")
            if result:
                is_anomaly = result.get("isAnomaly", False)
                z_score = result.get("zScore", 0)
                category = result.get("category", "unknown")
                amount = result.get("amount", 0)
                self.log_test("Check Anomaly", True, 
                             f"Category: {category}, Amount: ${amount}, Anomaly: {is_anomaly}, Z-Score: {z_score:.2f}. DSA: {ds_info}")
            else:
                self.log_test("Check Anomaly", False, "No result data", data)
        else:
            self.log_test("Check Anomaly", False, f"HTTP {response.status_code}", response.text)
        
        return True
    def test_autocomplete_endpoints(self):
        """Test autocomplete and category endpoints"""
        print("🔍 Testing Autocomplete Endpoints...")
        
        # Test GET /api/categories/suggest?prefix=F
        response = self.make_request("GET", "/categories/suggest", params={"prefix": "F"})
        if isinstance(response, tuple):
            self.log_test("Category Autocomplete with Prefix", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            suggestions = data.get("suggestions", [])
            ds_info = data.get("dsInfo", "")
            self.log_test("Category Autocomplete with Prefix", True, 
                         f"Found {len(suggestions)} suggestions for prefix 'F'. DSA: {ds_info}")
        else:
            self.log_test("Category Autocomplete with Prefix", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/categories/suggest (no prefix)
        response = self.make_request("GET", "/categories/suggest")
        if isinstance(response, tuple):
            self.log_test("Category Autocomplete (no prefix)", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            suggestions = data.get("suggestions", [])
            ds_info = data.get("dsInfo", "")
            self.log_test("Category Autocomplete (no prefix)", True, 
                         f"Found {len(suggestions)} suggestions (all categories). DSA: {ds_info}")
        else:
            self.log_test("Category Autocomplete (no prefix)", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/categories
        response = self.make_request("GET", "/categories")
        if isinstance(response, tuple):
            self.log_test("Get All Categories", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            categories = data.get("categories", [])
            self.log_test("Get All Categories", True, f"Found {len(categories)} categories")
        else:
            self.log_test("Get All Categories", False, f"HTTP {response.status_code}", response.text)
        
        return True
    
    def test_dsa_info_endpoint(self):
        """Test DSA info endpoint"""
        print("🔍 Testing DSA Info Endpoint...")
        response = self.make_request("GET", "/dsa-info")
        
        if isinstance(response, tuple):
            self.log_test("DSA Info", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            data_structures = data.get("dataStructures", [])
            complexity = data.get("complexity", {})
            database = data.get("database", {})
            
            if len(data_structures) >= 7 and complexity and database:
                # Check for specific data structures
                ds_names = [ds.get("name", "") for ds in data_structures]
                expected_structures = ["Red-Black Tree", "Skip List", "Indexed Priority Queue", 
                                     "Polynomial Hash Map", "Sliding Window", "IntroSort", 
                                     "Z-Score Anomaly Detection"]
                
                found_structures = [name for name in expected_structures if name in ds_names]
                
                self.log_test("DSA Info", True, 
                             f"Found {len(data_structures)} data structures ({len(found_structures)}/{len(expected_structures)} expected), "
                             f"complexity info for {len(complexity)} operations, database: {database.get('type', 'unknown')}")
                return True
            else:
                self.log_test("DSA Info", False, 
                             f"Incomplete DSA info - structures: {len(data_structures)}, complexity: {len(complexity)}, database: {bool(database)}", 
                             data)
                return False
        else:
            self.log_test("DSA Info", False, f"HTTP {response.status_code}", response.text)
            return False
    
    def test_undo_endpoint(self):
        """Test undo functionality"""
        print("🔍 Testing Undo Endpoint...")
        
        # First add a transaction to have something to undo
        test_transaction = {
            "type": "expense",
            "amount": 75.25,
            "category": "Transportation",
            "description": "Gas for car - test for undo",
            "date": "2025-01-16"
        }
        
        add_response = self.make_request("POST", "/transactions", test_transaction)
        if isinstance(add_response, tuple) or add_response.status_code != 200:
            self.log_test("Undo Setup", False, "Could not add transaction for undo test")
            return False
        
        add_data = add_response.json()
        if not add_data.get("success"):
            self.log_test("Undo Setup", False, "Transaction creation failed", add_data)
            return False
        
        self.log_test("Undo Setup", True, "Added test transaction for undo")
        
        # Now test POST /api/undo
        response = self.make_request("POST", "/undo")
        if isinstance(response, tuple):
            self.log_test("Undo Operation", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            success = data.get("success", False)
            can_undo = data.get("canUndo", False)
            ds_info = data.get("dsInfo", "")
            if success:
                self.log_test("Undo Operation", True, f"Undo successful. Can undo more: {can_undo}. DSA: {ds_info}")
            else:
                self.log_test("Undo Operation", False, "Undo operation failed", data)
        else:
            self.log_test("Undo Operation", False, f"HTTP {response.status_code}", response.text)
        
        return True
    
    def run_all_tests(self):
        """Run all backend API tests"""
        print("🚀 Starting COMPREHENSIVE Smart Personal Finance Tracker Backend API Tests")
        print(f"🌐 Backend URL: {self.base_url}")
        print("=" * 80)
        
        # Test in logical order - all endpoints from the review request
        tests = [
            ("Root & Health", self.test_root_and_health),
            ("Dashboard", self.test_dashboard_endpoint),
            ("Transactions CRUD", self.test_transactions_endpoints),
            ("Budgets", self.test_budgets_endpoints),
            ("Bills", self.test_bills_endpoints),
            ("Analytics", self.test_analytics_endpoints),
            ("Spending Trends", self.test_spending_trends_endpoints),
            ("Anomaly Detection", self.test_anomaly_detection_endpoints),
            ("Autocomplete", self.test_autocomplete_endpoints),
            ("Undo", self.test_undo_endpoint),
            ("DSA Info", self.test_dsa_info_endpoint)
        ]
        
        total_tests = 0
        passed_tests = 0
        failed_tests = []
        
        for test_group_name, test_func in tests:
            print(f"\n📋 Running {test_group_name} Tests...")
            try:
                # Clear previous results
                self.test_results = []
                
                # Run the test
                test_func()
                
                # Count results for this group
                group_total = len(self.test_results)
                group_passed = sum(1 for result in self.test_results if result["success"])
                group_failed = group_total - group_passed
                
                total_tests += group_total
                passed_tests += group_passed
                
                # Track failed tests
                for result in self.test_results:
                    if not result["success"]:
                        failed_tests.append({
                            "group": test_group_name,
                            "test": result["test"],
                            "details": result["details"],
                            "response": result.get("response")
                        })
                
                print(f"   {test_group_name}: {group_passed}/{group_total} passed")
                
            except Exception as e:
                print(f"❌ CRITICAL ERROR in {test_group_name}: {str(e)}")
                total_tests += 1
                failed_tests.append({
                    "group": test_group_name,
                    "test": "Critical Error",
                    "details": str(e),
                    "response": None
                })
        
        print("\n" + "=" * 80)
        print(f"📊 FINAL RESULTS: {passed_tests}/{total_tests} tests passed")
        
        if failed_tests:
            print(f"\n❌ FAILED TESTS ({len(failed_tests)}):")
            for failure in failed_tests:
                print(f"   • {failure['group']} - {failure['test']}: {failure['details']}")
        
        if passed_tests == total_tests:
            print("\n🎉 ALL TESTS PASSED! Backend API is working correctly.")
            return True
        else:
            print(f"\n⚠️  {total_tests - passed_tests} tests failed. Check details above.")
            return False

def main():
    """Main test execution"""
    tester = FinanceTrackerTester()
    success = tester.run_all_tests()
    
    if success:
        print("\n✅ Backend API testing completed successfully!")
        sys.exit(0)
    else:
        print("\n❌ Backend API testing completed with failures!")
        sys.exit(1)

if __name__ == "__main__":
    main()