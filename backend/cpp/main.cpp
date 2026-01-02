// Main Finance Engine Executable
// Processes JSON input and produces JSON output
// Data Structures & Applications Lab Project

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#include "finance_engine.h"

// Simple JSON parsing helpers (for academic purposes - no external libraries)
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\"");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r\"");
    return str.substr(first, (last - first + 1));
}

std::string extractValue(const std::string& json, const std::string& key) {
    size_t pos = json.find("\"" + key + "\"");
    if (pos == std::string::npos) return "";
    
    pos = json.find(":", pos);
    if (pos == std::string::npos) return "";
    
    size_t start = json.find_first_not_of(" \t\n\r", pos + 1);
    if (start == std::string::npos) return "";
    
    if (json[start] == '"') {
        size_t end = json.find("\"", start + 1);
        return json.substr(start + 1, end - start - 1);
    } else if (json[start] == '[' || json[start] == '{') {
        int depth = 1;
        char open = json[start];
        char close = (open == '[') ? ']' : '}';
        size_t end = start + 1;
        while (depth > 0 && end < json.length()) {
            if (json[end] == open) depth++;
            else if (json[end] == close) depth--;
            end++;
        }
        return json.substr(start, end - start);
    } else {
        size_t end = json.find_first_of(",}\n", start);
        return trim(json.substr(start, end - start));
    }
}

double extractDouble(const std::string& json, const std::string& key) {
    std::string val = extractValue(json, key);
    if (val.empty()) return 0.0;
    try {
        return std::stod(val);
    } catch (...) {
        return 0.0;
    }
}

int extractInt(const std::string& json, const std::string& key) {
    std::string val = extractValue(json, key);
    if (val.empty()) return 0;
    try {
        return std::stoi(val);
    } catch (...) {
        return 0;
    }
}

bool extractBool(const std::string& json, const std::string& key) {
    std::string val = extractValue(json, key);
    return val == "true";
}

std::vector<std::string> splitJsonArray(const std::string& arr) {
    std::vector<std::string> result;
    if (arr.length() < 2) return result;
    
    std::string content = arr.substr(1, arr.length() - 2);  // Remove [ ]
    
    int depth = 0;
    size_t start = 0;
    
    for (size_t i = 0; i < content.length(); i++) {
        if (content[i] == '{') depth++;
        else if (content[i] == '}') depth--;
        else if (content[i] == ',' && depth == 0) {
            result.push_back(trim(content.substr(start, i - start)));
            start = i + 1;
        }
    }
    
    if (start < content.length()) {
        result.push_back(trim(content.substr(start)));
    }
    
    return result;
}

// JSON output helpers
std::string escapeJson(const std::string& str) {
    std::string result;
    for (char c : str) {
        switch (c) {
            case '"': result += "\\\""; break;
            case '\\': result += "\\\\"; break;
            case '\n': result += "\\n"; break;
            case '\r': result += "\\r"; break;
            case '\t': result += "\\t"; break;
            default: result += c;
        }
    }
    return result;
}

std::string transactionToJson(const Transaction& t) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{\"id\":\"" << escapeJson(t.id) << "\","
       << "\"type\":\"" << escapeJson(t.type) << "\","
       << "\"amount\":" << t.amount << ","
       << "\"category\":\"" << escapeJson(t.category) << "\","
       << "\"description\":\"" << escapeJson(t.description) << "\","
       << "\"date\":\"" << escapeJson(t.date) << "\"}";
    return ss.str();
}

std::string budgetToJson(const Budget& b) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{\"category\":\"" << escapeJson(b.category) << "\","
       << "\"limit\":" << b.limit << ","
       << "\"spent\":" << b.spent << ","
       << "\"percentUsed\":" << b.getPercentUsed() << ","
       << "\"alertLevel\":\"" << b.getAlertLevel() << "\"}";
    return ss.str();
}

std::string billToJson(const Bill& b) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{\"id\":\"" << escapeJson(b.id) << "\","
       << "\"name\":\"" << escapeJson(b.name) << "\","
       << "\"amount\":" << b.amount << ","
       << "\"dueDate\":\"" << escapeJson(b.dueDate) << "\","
       << "\"category\":\"" << escapeJson(b.category) << "\","
       << "\"isPaid\":" << (b.isPaid ? "true" : "false") << "}";
    return ss.str();
}

std::string alertToJson(const BudgetAlert& a) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{\"category\":\"" << escapeJson(a.category) << "\","
       << "\"level\":\"" << escapeJson(a.level) << "\","
       << "\"percentUsed\":" << a.percentUsed << ","
       << "\"spent\":" << a.spent << ","
       << "\"limit\":" << a.limit << ","
       << "\"message\":\"" << escapeJson(a.message) << "\"}";
    return ss.str();
}

std::string categoryAmountToJson(const CategoryAmount& ca) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{\"category\":\"" << escapeJson(ca.category) << "\","
       << "\"totalAmount\":" << ca.totalAmount << "}";
    return ss.str();
}

std::string summaryToJson(const MonthlySummary& s) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(2);
    ss << "{\"month\":\"" << escapeJson(s.month) << "\","
       << "\"totalIncome\":" << s.totalIncome << ","
       << "\"totalExpenses\":" << s.totalExpenses << ","
       << "\"netSavings\":" << s.netSavings << ","
       << "\"transactionCount\":" << s.transactionCount << ","
       << "\"categoryBreakdown\":[";
    
    for (size_t i = 0; i < s.categoryBreakdown.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"category\":\"" << escapeJson(s.categoryBreakdown[i].first) << "\","
           << "\"amount\":" << s.categoryBreakdown[i].second << "}";
    }
    
    ss << "]}";
    return ss.str();
}

// Global finance engine instance
FinanceEngine engine;

// Load data from files
void loadData(const std::string& dataDir) {
    // Load transactions
    std::ifstream transFile(dataDir + "/transactions.json");
    if (transFile.is_open()) {
        std::stringstream buffer;
        buffer << transFile.rdbuf();
        std::string content = buffer.str();
        transFile.close();
        
        std::string arr = extractValue(content, "transactions");
        if (!arr.empty()) {
            auto items = splitJsonArray(arr);
            for (const auto& item : items) {
                std::string id = extractValue(item, "id");
                std::string type = extractValue(item, "type");
                double amount = extractDouble(item, "amount");
                std::string category = extractValue(item, "category");
                std::string description = extractValue(item, "description");
                std::string date = extractValue(item, "date");
                
                if (!id.empty() && !type.empty()) {
                    engine.loadTransaction(id, type, amount, category, description, date);
                }
            }
        }
    }
    
    // Load budgets
    std::ifstream budgetFile(dataDir + "/budgets.json");
    if (budgetFile.is_open()) {
        std::stringstream buffer;
        buffer << budgetFile.rdbuf();
        std::string content = buffer.str();
        budgetFile.close();
        
        std::string arr = extractValue(content, "budgets");
        if (!arr.empty()) {
            auto items = splitJsonArray(arr);
            for (const auto& item : items) {
                std::string category = extractValue(item, "category");
                double limit = extractDouble(item, "limit");
                
                if (!category.empty() && limit > 0) {
                    engine.loadBudget(category, limit);
                }
            }
        }
    }
    
    // Load bills
    std::ifstream billFile(dataDir + "/bills.json");
    if (billFile.is_open()) {
        std::stringstream buffer;
        buffer << billFile.rdbuf();
        std::string content = buffer.str();
        billFile.close();
        
        std::string arr = extractValue(content, "bills");
        if (!arr.empty()) {
            auto items = splitJsonArray(arr);
            for (const auto& item : items) {
                std::string id = extractValue(item, "id");
                std::string name = extractValue(item, "name");
                double amount = extractDouble(item, "amount");
                std::string dueDate = extractValue(item, "dueDate");
                std::string category = extractValue(item, "category");
                bool isPaid = extractBool(item, "isPaid");
                
                if (!id.empty() && !name.empty()) {
                    engine.loadBill(id, name, amount, dueDate, category, isPaid);
                }
            }
        }
    }
    
    // Load undo stack (NEW - for persistence across commands)
    std::ifstream undoFile(dataDir + "/undo_stack.json");
    if (undoFile.is_open()) {
        std::stringstream buffer;
        buffer << undoFile.rdbuf();
        std::string content = buffer.str();
        undoFile.close();
        
        std::string arr = extractValue(content, "actions");
        if (!arr.empty()) {
            auto items = splitJsonArray(arr);
            // Load in reverse order since we're pushing to stack
            for (int i = items.size() - 1; i >= 0; i--) {
                int type = extractInt(items[i], "type");
                std::string data = extractValue(items[i], "data");
                engine.loadUndoAction(static_cast<ActionType>(type), data);
            }
        }
    }
}

// Save data to files
void saveData(const std::string& dataDir) {
    // Save transactions
    std::ofstream transFile(dataDir + "/transactions.json");
    if (transFile.is_open()) {
        transFile << "{\"transactions\":[";
        auto transactions = engine.getAllTransactions();
        for (size_t i = 0; i < transactions.size(); i++) {
            if (i > 0) transFile << ",";
            transFile << transactionToJson(transactions[i]);
        }
        transFile << "]}";
        transFile.close();
    }
    
    // Save budgets
    std::ofstream budgetFile(dataDir + "/budgets.json");
    if (budgetFile.is_open()) {
        budgetFile << "{\"budgets\":[";
        auto budgets = engine.getAllBudgets();
        for (size_t i = 0; i < budgets.size(); i++) {
            if (i > 0) budgetFile << ",";
            budgetFile << "{\"category\":\"" << escapeJson(budgets[i].category) << "\","
                       << "\"limit\":" << budgets[i].limit << "}";
        }
        budgetFile << "]}";
        budgetFile.close();
    }
    
    // Save bills
    std::ofstream billFile(dataDir + "/bills.json");
    if (billFile.is_open()) {
        billFile << "{\"bills\":[";
        auto bills = engine.getAllBills();
        for (size_t i = 0; i < bills.size(); i++) {
            if (i > 0) billFile << ",";
            billFile << billToJson(bills[i]);
        }
        billFile << "]}";
        billFile.close();
    }
    
    // Save undo stack (NEW - for persistence across commands)
    std::ofstream undoFile(dataDir + "/undo_stack.json");
    if (undoFile.is_open()) {
        undoFile << "{\"actions\":[";
        auto actions = engine.getUndoActions();
        for (size_t i = 0; i < actions.size(); i++) {
            if (i > 0) undoFile << ",";
            undoFile << "{\"type\":" << actions[i].type << ","
                     << "\"data\":\"" << escapeJson(actions[i].data) << "\"}";
        }
        undoFile << "]}";
        undoFile.close();
    }
}

// Process command and return JSON result
std::string processCommand(const std::string& command, const std::string& params) {
    std::ostringstream result;
    result << std::fixed << std::setprecision(2);
    
    if (command == "add_transaction") {
        std::string type = extractValue(params, "type");
        double amount = extractDouble(params, "amount");
        std::string category = extractValue(params, "category");
        std::string description = extractValue(params, "description");
        std::string date = extractValue(params, "date");
        
        if (date.empty()) {
            // Get current date
            time_t now = time(0);
            tm* ltm = localtime(&now);
            char buffer[11];
            strftime(buffer, 11, "%Y-%m-%d", ltm);
            date = buffer;
        }
        
        Transaction t = engine.addTransaction(type, amount, category, description, date);
        result << "{\"success\":true,\"transaction\":" << transactionToJson(t) 
               << ",\"canUndo\":" << (engine.canUndo() ? "true" : "false") << "}";
    }
    else if (command == "delete_transaction") {
        std::string id = extractValue(params, "id");
        bool success = engine.deleteTransaction(id);
        result << "{\"success\":" << (success ? "true" : "false") 
               << ",\"canUndo\":" << (engine.canUndo() ? "true" : "false") << "}";
    }
    else if (command == "get_transactions") {
        auto transactions = engine.getTransactionsByDateDesc();
        result << "{\"transactions\":[";
        for (size_t i = 0; i < transactions.size(); i++) {
            if (i > 0) result << ",";
            result << transactionToJson(transactions[i]);
        }
        result << "]}";
    }
    else if (command == "get_recent_transactions") {
        int count = 10;
        std::string countStr = extractValue(params, "count");
        if (!countStr.empty()) {
            count = std::stoi(countStr);
        }
        auto transactions = engine.getRecentTransactions(count);
        result << "{\"transactions\":[";
        for (size_t i = 0; i < transactions.size(); i++) {
            if (i > 0) result << ",";
            result << transactionToJson(transactions[i]);
        }
        result << "],\"dsInfo\":\"Recent transactions from Stack (LIFO)\"}";
    }
    else if (command == "get_transactions_by_date") {
        std::string startDate = extractValue(params, "startDate");
        std::string endDate = extractValue(params, "endDate");
        auto transactions = engine.getTransactionsInRange(startDate, endDate);
        result << "{\"transactions\":[";
        for (size_t i = 0; i < transactions.size(); i++) {
            if (i > 0) result << ",";
            result << transactionToJson(transactions[i]);
        }
        result << "],\"dsInfo\":\"Date range query using BST\"}";
    }
    else if (command == "set_budget") {
        std::string category = extractValue(params, "category");
        double limit = extractDouble(params, "limit");
        engine.setBudget(category, limit);
        Budget b;
        engine.getBudget(category, b);
        result << "{\"success\":true,\"budget\":" << budgetToJson(b) 
               << ",\"canUndo\":" << (engine.canUndo() ? "true" : "false") << "}";
    }
    else if (command == "get_budgets") {
        auto budgets = engine.getAllBudgets();
        result << "{\"budgets\":[";
        for (size_t i = 0; i < budgets.size(); i++) {
            if (i > 0) result << ",";
            result << budgetToJson(budgets[i]);
        }
        result << "],\"dsInfo\":\"Budget data stored in HashMap\"}";
    }
    else if (command == "get_alerts") {
        auto alerts = engine.getBudgetAlerts();
        result << "{\"alerts\":[";
        for (size_t i = 0; i < alerts.size(); i++) {
            if (i > 0) result << ",";
            result << alertToJson(alerts[i]);
        }
        result << "]}";
    }
    else if (command == "add_bill") {
        std::string name = extractValue(params, "name");
        double amount = extractDouble(params, "amount");
        std::string dueDate = extractValue(params, "dueDate");
        std::string category = extractValue(params, "category");
        
        Bill b = engine.addBill(name, amount, dueDate, category);
        result << "{\"success\":true,\"bill\":" << billToJson(b) 
               << ",\"canUndo\":" << (engine.canUndo() ? "true" : "false") << "}";
    }
    else if (command == "get_bills") {
        auto bills = engine.getAllBills();
        result << "{\"bills\":[";
        for (size_t i = 0; i < bills.size(); i++) {
            if (i > 0) result << ",";
            result << billToJson(bills[i]);
        }
        result << "],\"dsInfo\":\"Bills managed in Queue (FIFO)\"}";
    }
    else if (command == "pay_bill") {
        std::string id = extractValue(params, "id");
        bool success = engine.payBill(id);
        result << "{\"success\":" << (success ? "true" : "false") 
               << ",\"canUndo\":" << (engine.canUndo() ? "true" : "false") << "}";
    }
    else if (command == "delete_bill") {
        std::string id = extractValue(params, "id");
        bool success = engine.removeBill(id);
        result << "{\"success\":" << (success ? "true" : "false") 
               << ",\"canUndo\":" << (engine.canUndo() ? "true" : "false") << "}";
    }
    else if (command == "get_top_expenses") {
        int k = 5;
        std::string kStr = extractValue(params, "count");
        if (!kStr.empty()) {
            k = std::stoi(kStr);
        }
        auto expenses = engine.getTopExpenses(k);
        result << "{\"topExpenses\":[";
        for (size_t i = 0; i < expenses.size(); i++) {
            if (i > 0) result << ",";
            result << transactionToJson(expenses[i]);
        }
        result << "],\"dsInfo\":\"Top expenses extracted from Max Heap\"}";
    }
    else if (command == "get_top_categories") {
        int k = 5;
        std::string kStr = extractValue(params, "count");
        if (!kStr.empty()) {
            k = std::stoi(kStr);
        }
        auto categories = engine.getTopCategories(k);
        result << "{\"topCategories\":[";
        for (size_t i = 0; i < categories.size(); i++) {
            if (i > 0) result << ",";
            result << categoryAmountToJson(categories[i]);
        }
        result << "],\"dsInfo\":\"Top categories from Category Max Heap\"}";
    }
    else if (command == "get_monthly_summary") {
        std::string month = extractValue(params, "month");
        if (month.empty()) {
            time_t now = time(0);
            tm* ltm = localtime(&now);
            char buffer[8];
            strftime(buffer, 8, "%Y-%m", ltm);
            month = buffer;
        }
        MonthlySummary summary = engine.getMonthlySummary(month);
        result << "{\"summary\":" << summaryToJson(summary) << ",\"dsInfo\":\"Monthly data from BST range query\"}";
    }
    else if (command == "get_category_suggestions") {
        std::string prefix = extractValue(params, "prefix");
        auto suggestions = engine.getCategorySuggestions(prefix);
        result << "{\"suggestions\":[";
        for (size_t i = 0; i < suggestions.size(); i++) {
            if (i > 0) result << ",";
            result << "\"" << escapeJson(suggestions[i]) << "\"";
        }
        result << "],\"dsInfo\":\"Autocomplete using Trie\"}";
    }
    else if (command == "get_all_categories") {
        auto categories = engine.getAllCategories();
        result << "{\"categories\":[";
        for (size_t i = 0; i < categories.size(); i++) {
            if (i > 0) result << ",";
            result << "\"" << escapeJson(categories[i]) << "\"";
        }
        result << "]}";
    }
    else if (command == "undo") {
        bool success = engine.undo();
        result << "{\"success\":" << (success ? "true" : "false") 
               << ",\"canUndo\":" << (engine.canUndo() ? "true" : "false") 
               << ",\"dsInfo\":\"Undo operation using Stack\"}";
    }
    else if (command == "get_dashboard") {
        result << "{\"balance\":" << engine.getTotalBalance() << ","
               << "\"totalIncome\":" << engine.getTotalIncome() << ","
               << "\"totalExpenses\":" << engine.getTotalExpenses() << ","
               << "\"transactionCount\":" << engine.getTransactionCount() << ","
               << "\"budgetCount\":" << engine.getBudgetCount() << ","
               << "\"billCount\":" << engine.getBillCount() << ","
               << "\"canUndo\":" << (engine.canUndo() ? "true" : "false") << "}";
    }
    else if (command == "clear_undo") {
        engine.clearUndoStack();
        result << "{\"success\":true,\"canUndo\":false}";
    }
    else {
        result << "{\"error\":\"Unknown command: " << escapeJson(command) << "\"}";
    }
    
    return result.str();
}

int main(int argc, char* argv[]) {
    std::string dataDir = "../data";
    
    // Parse arguments
    if (argc >= 2) {
        dataDir = argv[1];
    }
    
    // Load existing data (including undo stack)
    loadData(dataDir);
    
    // Read command from stdin (JSON format)
    std::string input;
    std::getline(std::cin, input);
    
    std::string command = extractValue(input, "command");
    std::string params = extractValue(input, "params");
    
    // Process command
    std::string output = processCommand(command, params.empty() ? input : params);
    
    // Save data after modifications (including undo stack)
    if (command == "add_transaction" || command == "delete_transaction" ||
        command == "set_budget" || command == "add_bill" || 
        command == "pay_bill" || command == "delete_bill" || 
        command == "undo" || command == "clear_undo") {
        saveData(dataDir);
    }
    
    // Output result
    std::cout << output << std::endl;
    
    return 0;
}
