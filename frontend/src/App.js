import { BrowserRouter, Routes, Route, Navigate } from "react-router-dom";
import "@/App.css";
import { Toaster } from "@/components/ui/sonner";

// Pages
import Dashboard from "@/pages/Dashboard";
import Transactions from "@/pages/Transactions";
import AddTransaction from "@/pages/AddTransaction";
import Budgets from "@/pages/Budgets";
import Bills from "@/pages/Bills";
import Analytics from "@/pages/Analytics";
import DSAInfo from "@/pages/DSAInfo";

// Layout
import Sidebar from "@/components/Sidebar";

function App() {
  return (
    <div className="App">
      <BrowserRouter>
        <div className="flex min-h-screen bg-slate-50">
          <Sidebar />
          <main className="main-content flex-1">
            <Routes>
              <Route path="/" element={<Navigate to="/dashboard" replace />} />
              <Route path="/dashboard" element={<Dashboard />} />
              <Route path="/transactions" element={<Transactions />} />
              <Route path="/add-transaction" element={<AddTransaction />} />
              <Route path="/budgets" element={<Budgets />} />
              <Route path="/bills" element={<Bills />} />
              <Route path="/analytics" element={<Analytics />} />
              <Route path="/dsa-info" element={<DSAInfo />} />
            </Routes>
          </main>
        </div>
        <Toaster position="top-right" richColors />
      </BrowserRouter>
    </div>
  );
}

export default App;
