#!/usr/bin/env python3
"""
Backend API Testing for Smart Personal Finance Tracker
Tests all FastAPI endpoints with C++ DSA engine integration
"""

import requests
import json
import sys
from datetime import datetime, timedelta
import time

# Backend URL from frontend/.env
BACKEND_URL = "https://expense-tracker-2413.preview.emergentagent.com/api"

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
    
    def test_health_endpoint(self):
        """Test /api/health endpoint"""
        print("🔍 Testing Health Endpoint...")
        response = self.make_request("GET", "/health")
        
        if isinstance(response, tuple):
            self.log_test("Health Check", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            engine_available = data.get("engine_available", False)
            status = data.get("status", "unknown")
            
            if engine_available and status == "healthy":
                self.log_test("Health Check", True, f"Status: {status}, Engine: Available")
                return True
            else:
                self.log_test("Health Check", False, f"Status: {status}, Engine: {engine_available}", data)
                return False
        else:
            self.log_test("Health Check", False, f"HTTP {response.status_code}", response.text)
            return False
    
    def test_dashboard_endpoint(self):
        """Test /api/dashboard endpoint"""
        print("🔍 Testing Dashboard Endpoint...")
        response = self.make_request("GET", "/dashboard")
        
        if isinstance(response, tuple):
            self.log_test("Dashboard Data", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            required_fields = ["balance", "totalIncome", "totalExpenses", "transactionCount"]
            
            missing_fields = [field for field in required_fields if field not in data]
            if missing_fields:
                self.log_test("Dashboard Data", False, f"Missing fields: {missing_fields}", data)
                return False
            
            # Check if values are reasonable (demo data should exist)
            balance = data.get("balance", 0)
            income = data.get("totalIncome", 0)
            expenses = data.get("totalExpenses", 0)
            
            self.log_test("Dashboard Data", True, 
                         f"Balance: ${balance:.2f}, Income: ${income:.2f}, Expenses: ${expenses:.2f}")
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
            self.log_test("Get All Transactions", True, f"Found {len(transactions)} transactions")
        else:
            self.log_test("Get All Transactions", False, f"HTTP {response.status_code}", response.text)
            return False
        
        # Test POST /api/transactions
        new_transaction = {
            "type": "expense",
            "amount": 100.0,
            "category": "Food",
            "description": "Test lunch",
            "date": "2025-01-16"
        }
        
        response = self.make_request("POST", "/transactions", new_transaction)
        if isinstance(response, tuple):
            self.log_test("Add Transaction", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            if "transaction" in data and "id" in data["transaction"]:
                self.created_transaction_id = data["transaction"]["id"]
                self.log_test("Add Transaction", True, f"Created transaction ID: {self.created_transaction_id}")
            else:
                self.log_test("Add Transaction", False, "No transaction ID returned", data)
                return False
        else:
            self.log_test("Add Transaction", False, f"HTTP {response.status_code}", response.text)
            return False
        
        # Test GET /api/transactions/recent
        response = self.make_request("GET", "/transactions/recent", params={"count": 5})
        if isinstance(response, tuple):
            self.log_test("Get Recent Transactions", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            recent = data.get("transactions", [])
            self.log_test("Get Recent Transactions", True, f"Found {len(recent)} recent transactions")
        else:
            self.log_test("Get Recent Transactions", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/transactions/range
        start_date = "2025-01-01"
        end_date = "2025-01-31"
        response = self.make_request("GET", "/transactions/range", 
                                   params={"start_date": start_date, "end_date": end_date})
        if isinstance(response, tuple):
            self.log_test("Get Transactions by Range", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            range_transactions = data.get("transactions", [])
            self.log_test("Get Transactions by Range", True, 
                         f"Found {len(range_transactions)} transactions in range {start_date} to {end_date}")
        else:
            self.log_test("Get Transactions by Range", False, f"HTTP {response.status_code}", response.text)
        
        # Test DELETE /api/transactions/{id} (if we have a transaction ID)
        if self.created_transaction_id:
            response = self.make_request("DELETE", f"/transactions/{self.created_transaction_id}")
            if isinstance(response, tuple):
                self.log_test("Delete Transaction", False, f"Request failed: {response[1]}")
            elif response.status_code == 200:
                self.log_test("Delete Transaction", True, f"Deleted transaction {self.created_transaction_id}")
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
            self.log_test("Get All Budgets", True, f"Found {len(budgets)} budgets")
        else:
            self.log_test("Get All Budgets", False, f"HTTP {response.status_code}", response.text)
            return False
        
        # Test POST /api/budgets
        new_budget = {
            "category": "Food",
            "limit": 200.0
        }
        
        response = self.make_request("POST", "/budgets", new_budget)
        if isinstance(response, tuple):
            self.log_test("Set Budget", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            self.log_test("Set Budget", True, f"Set budget for {new_budget['category']}: ${new_budget['limit']}")
        else:
            self.log_test("Set Budget", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/budgets/alerts
        response = self.make_request("GET", "/budgets/alerts")
        if isinstance(response, tuple):
            self.log_test("Get Budget Alerts", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            alerts = data.get("alerts", [])
            self.log_test("Get Budget Alerts", True, f"Found {len(alerts)} budget alerts")
        else:
            self.log_test("Get Budget Alerts", False, f"HTTP {response.status_code}", response.text)
        
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
            self.log_test("Get All Bills", True, f"Found {len(bills)} bills")
        else:
            self.log_test("Get All Bills", False, f"HTTP {response.status_code}", response.text)
            return False
        
        # Test POST /api/bills
        new_bill = {
            "name": "Test Bill",
            "amount": 50.0,
            "dueDate": "2025-01-30",
            "category": "Utilities"
        }
        
        response = self.make_request("POST", "/bills", new_bill)
        if isinstance(response, tuple):
            self.log_test("Add Bill", False, f"Request failed: {response[1]}")
            return False
        
        if response.status_code == 200:
            data = response.json()
            if "bill" in data and "id" in data["bill"]:
                self.created_bill_id = data["bill"]["id"]
                self.log_test("Add Bill", True, f"Created bill ID: {self.created_bill_id}")
            else:
                self.log_test("Add Bill", False, "No bill ID returned", data)
                return False
        else:
            self.log_test("Add Bill", False, f"HTTP {response.status_code}", response.text)
            return False
        
        # Test POST /api/bills/{id}/pay (if we have a bill ID)
        if self.created_bill_id:
            response = self.make_request("POST", f"/bills/{self.created_bill_id}/pay")
            if isinstance(response, tuple):
                self.log_test("Pay Bill", False, f"Request failed: {response[1]}")
            elif response.status_code == 200:
                self.log_test("Pay Bill", True, f"Paid bill {self.created_bill_id}")
            else:
                self.log_test("Pay Bill", False, f"HTTP {response.status_code}", response.text)
        
        # Test DELETE /api/bills/{id} (if we have a bill ID)
        if self.created_bill_id:
            response = self.make_request("DELETE", f"/bills/{self.created_bill_id}")
            if isinstance(response, tuple):
                self.log_test("Delete Bill", False, f"Request failed: {response[1]}")
            elif response.status_code == 200:
                self.log_test("Delete Bill", True, f"Deleted bill {self.created_bill_id}")
            else:
                self.log_test("Delete Bill", False, f"HTTP {response.status_code}", response.text)
        
        return True
    
    def test_analytics_endpoints(self):
        """Test all analytics endpoints"""
        print("🔍 Testing Analytics Endpoints...")
        
        # Test GET /api/analytics/top-expenses
        response = self.make_request("GET", "/analytics/top-expenses", params={"count": 5})
        if isinstance(response, tuple):
            self.log_test("Get Top Expenses", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            expenses = data.get("expenses", [])
            self.log_test("Get Top Expenses", True, f"Found {len(expenses)} top expenses")
        else:
            self.log_test("Get Top Expenses", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/analytics/top-categories
        response = self.make_request("GET", "/analytics/top-categories", params={"count": 5})
        if isinstance(response, tuple):
            self.log_test("Get Top Categories", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            categories = data.get("categories", [])
            self.log_test("Get Top Categories", True, f"Found {len(categories)} top categories")
        else:
            self.log_test("Get Top Categories", False, f"HTTP {response.status_code}", response.text)
        
        # Test GET /api/analytics/monthly-summary
        response = self.make_request("GET", "/analytics/monthly-summary", params={"month": "2025-01"})
        if isinstance(response, tuple):
            self.log_test("Get Monthly Summary", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            self.log_test("Get Monthly Summary", True, "Monthly summary retrieved successfully")
        else:
            self.log_test("Get Monthly Summary", False, f"HTTP {response.status_code}", response.text)
        
        return True
    
    def test_autocomplete_endpoints(self):
        """Test autocomplete and category endpoints"""
        print("🔍 Testing Autocomplete Endpoints...")
        
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
        
        # Test GET /api/categories/suggest
        response = self.make_request("GET", "/categories/suggest", params={"prefix": "Fo"})
        if isinstance(response, tuple):
            self.log_test("Category Autocomplete", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            suggestions = data.get("suggestions", [])
            self.log_test("Category Autocomplete", True, f"Found {len(suggestions)} suggestions for 'Fo'")
        else:
            self.log_test("Category Autocomplete", False, f"HTTP {response.status_code}", response.text)
        
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
            
            if len(data_structures) >= 7 and complexity:
                self.log_test("DSA Info", True, f"Found {len(data_structures)} data structures with complexity info")
                return True
            else:
                self.log_test("DSA Info", False, "Incomplete DSA information", data)
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
            "amount": 25.0,
            "category": "Test",
            "description": "Test for undo",
            "date": "2025-01-16"
        }
        
        add_response = self.make_request("POST", "/transactions", test_transaction)
        if isinstance(add_response, tuple) or add_response.status_code != 200:
            self.log_test("Undo Setup", False, "Could not add transaction for undo test")
            return False
        
        # Now test undo
        response = self.make_request("POST", "/undo")
        if isinstance(response, tuple):
            self.log_test("Undo Operation", False, f"Request failed: {response[1]}")
        elif response.status_code == 200:
            data = response.json()
            self.log_test("Undo Operation", True, "Undo operation completed successfully")
        else:
            self.log_test("Undo Operation", False, f"HTTP {response.status_code}", response.text)
        
        return True
    
    def run_all_tests(self):
        """Run all backend API tests"""
        print("🚀 Starting Smart Personal Finance Tracker Backend API Tests")
        print(f"🌐 Backend URL: {self.base_url}")
        print("=" * 80)
        
        # Test in logical order
        tests = [
            self.test_health_endpoint,
            self.test_dashboard_endpoint,
            self.test_transactions_endpoints,
            self.test_budgets_endpoints,
            self.test_bills_endpoints,
            self.test_analytics_endpoints,
            self.test_autocomplete_endpoints,
            self.test_dsa_info_endpoint,
            self.test_undo_endpoint
        ]
        
        total_tests = 0
        passed_tests = 0
        
        for test_func in tests:
            try:
                test_func()
                # Count individual test results
                for result in self.test_results:
                    total_tests += 1
                    if result["success"]:
                        passed_tests += 1
                # Clear results for next test group
                self.test_results = []
            except Exception as e:
                print(f"❌ CRITICAL ERROR in {test_func.__name__}: {str(e)}")
                total_tests += 1
        
        print("=" * 80)
        print(f"📊 FINAL RESULTS: {passed_tests}/{total_tests} tests passed")
        
        if passed_tests == total_tests:
            print("🎉 ALL TESTS PASSED! Backend API is working correctly.")
            return True
        else:
            print(f"⚠️  {total_tests - passed_tests} tests failed. Check details above.")
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