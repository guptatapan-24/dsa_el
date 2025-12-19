import { useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Progress } from "@/components/ui/progress";
import { Badge } from "@/components/ui/badge";
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from "@/components/ui/dialog";
import { Plus, AlertTriangle, CheckCircle, AlertCircle } from "lucide-react";
import { toast } from "sonner";

const API = `${process.env.REACT_APP_BACKEND_URL}/api`;

export default function Budgets() {
  const [budgets, setBudgets] = useState([]);
  const [categories, setCategories] = useState([]);
  const [loading, setLoading] = useState(true);
  const [dialogOpen, setDialogOpen] = useState(false);
  const [form, setForm] = useState({ category: "", limit: "" });

  const fetchData = async () => {
    try {
      const [budgetRes, catRes] = await Promise.all([
        fetch(`${API}/budgets`),
        fetch(`${API}/categories`),
      ]);
      const budgetData = await budgetRes.json();
      const catData = await catRes.json();
      setBudgets(budgetData.budgets || []);
      setCategories(catData.categories || []);
    } catch {
      toast.error("Failed to load budgets");
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
    window.addEventListener("dataChanged", fetchData);
    return () => window.removeEventListener("dataChanged", fetchData);
  }, []);

  const handleSubmit = async (e) => {
    e.preventDefault();
    if (!form.category || !form.limit) {
      toast.error("Please fill in all fields");
      return;
    }

    try {
      const res = await fetch(`${API}/budgets`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          category: form.category,
          limit: parseFloat(form.limit),
        }),
      });
      const data = await res.json();
      if (data.success) {
        toast.success("Budget set!", {
          description: `$${form.limit} budget for ${form.category}`,
        });
        setDialogOpen(false);
        setForm({ category: "", limit: "" });
        fetchData();
        window.dispatchEvent(new Event("dataChanged"));
      }
    } catch {
      toast.error("Failed to set budget");
    }
  };

  const formatCurrency = (amount) => {
    return new Intl.NumberFormat("en-US", {
      style: "currency",
      currency: "USD",
    }).format(amount);
  };

  const getAlertIcon = (level) => {
    switch (level) {
      case "exceeded":
        return <AlertTriangle className="w-5 h-5 text-red-500" />;
      case "warning":
        return <AlertCircle className="w-5 h-5 text-orange-500" />;
      case "caution":
        return <AlertCircle className="w-5 h-5 text-yellow-500" />;
      default:
        return <CheckCircle className="w-5 h-5 text-emerald-500" />;
    }
  };

  const getProgressColor = (level) => {
    switch (level) {
      case "exceeded":
        return "bg-red-500";
      case "warning":
        return "bg-orange-500";
      case "caution":
        return "bg-yellow-500";
      default:
        return "bg-emerald-500";
    }
  };

  if (loading) {
    return (
      <div className="page-container flex items-center justify-center min-h-[50vh]">
        <div className="text-slate-500">Loading budgets...</div>
      </div>
    );
  }

  return (
    <div className="page-container" data-testid="budgets-page">
      {/* Header */}
      <div className="flex items-center justify-between mb-8">
        <div>
          <h1 className="text-2xl font-bold text-slate-900">Budget Management</h1>
          <p className="text-slate-500 mt-1">
            Set and track spending limits by category
            <span className="ds-badge ds-badge-hashmap ml-2">HashMap</span>
          </p>
        </div>
        <Dialog open={dialogOpen} onOpenChange={setDialogOpen}>
          <DialogTrigger asChild>
            <Button data-testid="add-budget-btn">
              <Plus className="w-4 h-4 mr-2" />
              Set Budget
            </Button>
          </DialogTrigger>
          <DialogContent>
            <DialogHeader>
              <DialogTitle>Set Category Budget</DialogTitle>
              <DialogDescription>
                Set a spending limit for a category. Alerts trigger at 50%, 80%, and 100%.
              </DialogDescription>
            </DialogHeader>
            <form onSubmit={handleSubmit}>
              <div className="space-y-4 py-4">
                <div className="space-y-2">
                  <Label htmlFor="budget-category">Category</Label>
                  <select
                    id="budget-category"
                    className="w-full border border-slate-200 rounded-md p-2"
                    value={form.category}
                    onChange={(e) => setForm({ ...form, category: e.target.value })}
                    data-testid="budget-category-select"
                  >
                    <option value="">Select a category...</option>
                    {categories.map((cat) => (
                      <option key={cat} value={cat}>
                        {cat}
                      </option>
                    ))}
                  </select>
                </div>
                <div className="space-y-2">
                  <Label htmlFor="budget-limit">Monthly Limit ($)</Label>
                  <Input
                    id="budget-limit"
                    type="number"
                    step="0.01"
                    min="1"
                    placeholder="500.00"
                    value={form.limit}
                    onChange={(e) => setForm({ ...form, limit: e.target.value })}
                    data-testid="budget-limit-input"
                  />
                </div>
              </div>
              <DialogFooter>
                <Button type="button" variant="outline" onClick={() => setDialogOpen(false)}>
                  Cancel
                </Button>
                <Button type="submit" data-testid="save-budget-btn">Save Budget</Button>
              </DialogFooter>
            </form>
          </DialogContent>
        </Dialog>
      </div>

      {/* Alert Thresholds Info */}
      <Card className="mb-6 bg-slate-50 border-slate-200">
        <CardContent className="py-4">
          <div className="flex flex-wrap gap-6 text-sm">
            <div className="flex items-center gap-2">
              <div className="w-3 h-3 rounded-full bg-emerald-500"></div>
              <span className="text-slate-600">&lt; 50% - Normal</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="w-3 h-3 rounded-full bg-yellow-500"></div>
              <span className="text-slate-600">50-79% - Caution</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="w-3 h-3 rounded-full bg-orange-500"></div>
              <span className="text-slate-600">80-99% - Warning</span>
            </div>
            <div className="flex items-center gap-2">
              <div className="w-3 h-3 rounded-full bg-red-500"></div>
              <span className="text-slate-600">≥ 100% - Exceeded</span>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Budget Grid */}
      {budgets.length === 0 ? (
        <Card>
          <CardContent className="py-12 text-center">
            <p className="text-slate-500 mb-4">No budgets set yet.</p>
            <Button onClick={() => setDialogOpen(true)}>Set Your First Budget</Button>
          </CardContent>
        </Card>
      ) : (
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4">
          {budgets.map((budget) => (
            <Card
              key={budget.category}
              className={`card-hover ${
                budget.alertLevel === "exceeded"
                  ? "border-red-200 bg-red-50/30"
                  : budget.alertLevel === "warning"
                  ? "border-orange-200 bg-orange-50/30"
                  : budget.alertLevel === "caution"
                  ? "border-yellow-200 bg-yellow-50/30"
                  : ""
              }`}
              data-testid={`budget-card-${budget.category}`}
            >
              <CardHeader className="pb-2">
                <div className="flex items-center justify-between">
                  <CardTitle className="text-base font-semibold">
                    {budget.category}
                  </CardTitle>
                  {getAlertIcon(budget.alertLevel)}
                </div>
              </CardHeader>
              <CardContent>
                <div className="space-y-4">
                  {/* Progress Bar */}
                  <div className="space-y-2">
                    <div className="flex justify-between text-sm">
                      <span className="text-slate-500">Spent</span>
                      <span className="data-value font-medium">
                        {formatCurrency(budget.spent)} / {formatCurrency(budget.limit)}
                      </span>
                    </div>
                    <div className="h-2 bg-slate-200 rounded-full overflow-hidden">
                      <div
                        className={`h-full ${getProgressColor(budget.alertLevel)} transition-all`}
                        style={{ width: `${Math.min(budget.percentUsed, 100)}%` }}
                      />
                    </div>
                  </div>

                  {/* Percentage */}
                  <div className="flex items-center justify-between">
                    <Badge
                      variant={budget.alertLevel === "normal" ? "secondary" : "destructive"}
                      className={
                        budget.alertLevel === "normal"
                          ? "bg-emerald-100 text-emerald-700"
                          : budget.alertLevel === "caution"
                          ? "bg-yellow-100 text-yellow-700"
                          : budget.alertLevel === "warning"
                          ? "bg-orange-100 text-orange-700"
                          : ""
                      }
                    >
                      {budget.percentUsed.toFixed(0)}% used
                    </Badge>
                    <span className="text-xs text-slate-500">
                      {formatCurrency(Math.max(0, budget.limit - budget.spent))} remaining
                    </span>
                  </div>
                </div>
              </CardContent>
            </Card>
          ))}
        </div>
      )}
    </div>
  );
}
