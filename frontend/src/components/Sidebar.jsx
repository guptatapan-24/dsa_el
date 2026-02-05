import { NavLink } from "react-router-dom";
import {
  LayoutDashboard,
  Receipt,
  PlusCircle,
  Wallet,
  CalendarClock,
  BarChart3,
  BookOpen,
  Undo2,
} from "lucide-react";
import { Button } from "@/components/ui/button";
import { toast } from "sonner";
import { useEffect, useState } from "react";

const API =
  (process.env.REACT_APP_BACKEND_URL || "http://localhost:8001") + "/api";
;

const navItems = [
  { path: "/dashboard", icon: LayoutDashboard, label: "Dashboard" },
  { path: "/transactions", icon: Receipt, label: "Transactions" },
  { path: "/add-transaction", icon: PlusCircle, label: "Add Transaction" },
  { path: "/budgets", icon: Wallet, label: "Budgets" },
  { path: "/bills", icon: CalendarClock, label: "Bills" },
  { path: "/analytics", icon: BarChart3, label: "Analytics" },
  // { path: "/dsa-info", icon: BookOpen, label: "DSA Reference" },
];

export default function Sidebar() {
  const [canUndo, setCanUndo] = useState(false);

  const checkUndoStatus = async () => {
    try {
      const res = await fetch(`${API}/dashboard`);
      const data = await res.json();
      setCanUndo(data.canUndo || false);
    } catch {
      // Silently fail
    }
  };

  useEffect(() => {
    checkUndoStatus();
    const interval = setInterval(checkUndoStatus, 5000);
    return () => clearInterval(interval);
  }, []);

  const handleUndo = async () => {
    try {
      const res = await fetch(`${API}/undo`, { method: "POST" });
      const data = await res.json();
      if (data.success) {
        toast.success("Undo successful", {
          description: "Last action has been undone (Stack pop operation)",
        });
        setCanUndo(data.canUndo);
        // Trigger page refresh
        window.dispatchEvent(new Event("dataChanged"));
      } else {
        toast.error("Nothing to undo");
      }
    } catch {
      toast.error("Undo failed");
    }
  };

  return (
    <aside className="sidebar" data-testid="sidebar">
      {/* Logo */}
      <div className="p-6 border-b border-slate-200">
        <h1 className="text-xl font-bold text-primary flex items-center gap-2">
          <Wallet className="w-6 h-6" />
          <span>Finance Tracker</span>
        </h1>
        <p className="text-xs text-slate-500 mt-1 font-mono">DSA Lab Project</p>
      </div>

      {/* Navigation */}
      <nav className="p-4 space-y-1">
        {navItems.map((item) => (
          <NavLink
            key={item.path}
            to={item.path}
            data-testid={`nav-${item.path.slice(1)}`}
            className={({ isActive }) =>
              `flex items-center gap-3 px-4 py-3 rounded-lg text-sm transition-smooth ${
                isActive
                  ? "nav-active"
                  : "text-slate-600 hover:bg-slate-100 hover:text-slate-900"
              }`
            }
          >
            <item.icon className="w-5 h-5" />
            <span>{item.label}</span>
          </NavLink>
        ))}
      </nav>

      {/* Undo Button */}
      <div className="absolute bottom-0 left-0 right-0 p-4 border-t border-slate-200 bg-white">
        <Button
          variant="outline"
          className="w-full justify-start gap-2"
          onClick={handleUndo}
          disabled={!canUndo}
          data-testid="undo-btn"
        >
          <Undo2 className="w-4 h-4" />
          <span>Undo Last Action</span>
          <span className="ds-badge ds-badge-stack ml-auto">Stack</span>
        </Button>
      </div>
    </aside>
  );
}
