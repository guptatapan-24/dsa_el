import { useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Calendar } from "@/components/ui/calendar";
import { Popover, PopoverContent, PopoverTrigger } from "@/components/ui/popover";
import { Badge } from "@/components/ui/badge";
import { Alert, AlertDescription, AlertTitle } from "@/components/ui/alert";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import {
  BarChart3,
  TrendingUp,
  TrendingDown,
  Calendar as CalendarIcon,
  PieChart,
  Activity,
  Zap,
  AlertTriangle,
} from "lucide-react";
import { toast } from "sonner";
import { format } from "date-fns";
const API =
  (process.env.REACT_APP_BACKEND_URL || "http://localhost:8001") + "/api";


export default function Analytics() {
  const [loading, setLoading] = useState(true);
  const [topExpenses, setTopExpenses] = useState([]);
  const [topCategories, setTopCategories] = useState([]);
  const [monthlySummary, setMonthlySummary] = useState(null);
  const [selectedMonth, setSelectedMonth] = useState(
    format(new Date(), "yyyy-MM")
  );
  const [dateRange, setDateRange] = useState({ from: null, to: null });
  const [rangeTransactions, setRangeTransactions] = useState([]);
  const [trend7Day, setTrend7Day] = useState(null);
  const [trend30Day, setTrend30Day] = useState(null);
  const [anomalies, setAnomalies] = useState([]);

  const fetchData = async () => {
    try {
      const [topExpRes, topCatRes, summaryRes, trend7Res, trend30Res, anomalyRes] = await Promise.all([
        fetch(`${API}/top-expenses?count=10`),
        fetch(`${API}/top-categories?count=10`),
        fetch(`${API}/monthly-summary?month=${selectedMonth}`),
        fetch(`${API}/trends/7-day`),
        fetch(`${API}/trends/30-day`),
        fetch(`${API}/anomalies?threshold=2.0`)
      ]);

      const topExpData = await topExpRes.json();
      const topCatData = await topCatRes.json();
      const summaryData = await summaryRes.json();
      const trend7Data = await trend7Res.json();
      const trend30Data = await trend30Res.json();
      const anomalyData = await anomalyRes.json();

      setTopExpenses(topExpData.topExpenses || []);
      setTopCategories(topCatData.topCategories || []);
      setMonthlySummary(summaryData.summary || null);
      setTrend7Day(trend7Data.trend || null);
      setTrend30Day(trend30Data.trend || null);
      setAnomalies(anomalyData.anomalies || []);
    } catch (error) {
      console.error("Failed to fetch analytics:", error);
      toast.error("Failed to load analytics");
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
    window.addEventListener("dataChanged", fetchData);
    return () => window.removeEventListener("dataChanged", fetchData);
  }, [selectedMonth]);

  const handleDateRangeQuery = async () => {
    if (!dateRange.from || !dateRange.to) {
      toast.error("Please select both start and end dates");
      return;
    }

    try {
      const startDate = format(dateRange.from, "yyyy-MM-dd");
      const endDate = format(dateRange.to, "yyyy-MM-dd");
      const res = await fetch(
        `${API}/transactions/range?start_date=${startDate}&end_date=${endDate}`
      );
      const data = await res.json();
      setRangeTransactions(data.transactions || []);
      toast.success(`Found ${data.transactions?.length || 0} transactions in range`);
    } catch {
      toast.error("Range query failed");
    }
  };

  const formatCurrency = (amount) => {
    return new Intl.NumberFormat("en-US", {
      style: "currency",
      currency: "USD",
    }).format(amount);
  };

  // Generate month options for last 12 months
  const monthOptions = [];
  for (let i = 0; i < 12; i++) {
    const date = new Date();
    date.setMonth(date.getMonth() - i);
    monthOptions.push(format(date, "yyyy-MM"));
  }

  // Calculate range summary
  const rangeSummary = rangeTransactions.reduce(
    (acc, tx) => {
      if (tx.type === "income") acc.income += tx.amount;
      else acc.expenses += tx.amount;
      return acc;
    },
    { income: 0, expenses: 0 }
  );

  if (loading) {
    return (
      <div className="page-container flex items-center justify-center min-h-[50vh]">
        <div className="text-slate-500">Loading analytics...</div>
      </div>
    );
  }

  return (
    <div className="page-container" data-testid="analytics-page">
      {/* Header */}
      <div className="mb-8">
        <h1 className="text-2xl font-bold text-slate-900">Analytics</h1>
        <p className="text-slate-500 mt-1">
          Financial insights powered by data structures
          <span className="ds-badge ds-badge-heap ml-2">Max Heap</span>
          <span className="ds-badge ds-badge-bst ml-1">BST</span>
        </p>
      </div>

      {/* Monthly Summary Section */}
      <div className="mb-8">
        <div className="flex items-center justify-between mb-4">
          <h2 className="text-lg font-semibold flex items-center gap-2">
            <BarChart3 className="w-5 h-5 text-primary" />
            Monthly Summary
            <span className="ds-badge ds-badge-bst">BST Range Query</span>
          </h2>
          <Select value={selectedMonth} onValueChange={setSelectedMonth}>
            <SelectTrigger className="w-[180px]" data-testid="month-select">
              <SelectValue placeholder="Select month" />
            </SelectTrigger>
            <SelectContent>
              {monthOptions.map((month) => (
                <SelectItem key={month} value={month}>
                  {format(new Date(month + "-01"), "MMMM yyyy")}
                </SelectItem>
              ))}
            </SelectContent>
          </Select>
        </div>

        {monthlySummary && (
          <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
            <Card className="kpi-card" data-testid="monthly-income">
              <CardHeader className="pb-2">
                <CardTitle className="text-sm text-slate-500 flex items-center gap-2">
                  <TrendingUp className="w-4 h-4 text-emerald-500" />
                  Income
                </CardTitle>
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold income-text">
                  {formatCurrency(monthlySummary.totalIncome)}
                </div>
              </CardContent>
            </Card>

            <Card className="kpi-card" data-testid="monthly-expenses">
              <CardHeader className="pb-2">
                <CardTitle className="text-sm text-slate-500 flex items-center gap-2">
                  <TrendingDown className="w-4 h-4 text-red-500" />
                  Expenses
                </CardTitle>
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold expense-text">
                  {formatCurrency(monthlySummary.totalExpenses)}
                </div>
              </CardContent>
            </Card>

            <Card className="kpi-card" data-testid="monthly-savings">
              <CardHeader className="pb-2">
                <CardTitle className="text-sm text-slate-500">Net Savings</CardTitle>
              </CardHeader>
              <CardContent>
                <div
                  className={`text-2xl font-bold ${
                    monthlySummary.netSavings >= 0 ? "income-text" : "expense-text"
                  }`}
                >
                  {formatCurrency(monthlySummary.netSavings)}
                </div>
              </CardContent>
            </Card>

            <Card className="kpi-card" data-testid="monthly-count">
              <CardHeader className="pb-2">
                <CardTitle className="text-sm text-slate-500">Transactions</CardTitle>
              </CardHeader>
              <CardContent>
                <div className="text-2xl font-bold text-slate-900">
                  {monthlySummary.transactionCount}
                </div>
              </CardContent>
            </Card>
          </div>
        )}

        {/* Category Breakdown */}
        {monthlySummary?.categoryBreakdown?.length > 0 && (
          <Card className="mt-4">
            <CardHeader>
              <CardTitle className="text-base">Category Breakdown</CardTitle>
            </CardHeader>
            <CardContent>
              <div className="space-y-3">
                {monthlySummary.categoryBreakdown.map((cat, idx) => (
                  <div key={idx} className="flex items-center justify-between">
                    <div className="flex items-center gap-2">
                      <Badge variant="secondary">{cat.category}</Badge>
                    </div>
                    <span className="font-semibold expense-text">
                      {formatCurrency(cat.amount)}
                    </span>
                  </div>
                ))}
              </div>
            </CardContent>
          </Card>
        )}
      </div>

      {/* Date Range Query Section */}
      <Card className="mb-8" data-testid="date-range-section">
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <CalendarIcon className="w-5 h-5" />
            Date Range Expense Report
            <span className="ds-badge ds-badge-bst">BST Range Query</span>
          </CardTitle>
          <CardDescription>
            Query transactions within a date range using BST traversal
          </CardDescription>
        </CardHeader>
        <CardContent>
          <div className="flex flex-wrap gap-4 items-end mb-4">
            <Popover>
              <PopoverTrigger asChild>
                <Button variant="outline" className="w-[280px]" data-testid="range-picker">
                  <CalendarIcon className="mr-2 h-4 w-4" />
                  {dateRange.from ? (
                    dateRange.to ? (
                      `${format(dateRange.from, "MMM d, yyyy")} - ${format(
                        dateRange.to,
                        "MMM d, yyyy"
                      )}`
                    ) : (
                      format(dateRange.from, "MMM d, yyyy")
                    )
                  ) : (
                    "Select date range"
                  )}
                </Button>
              </PopoverTrigger>
              <PopoverContent className="w-auto p-0" align="start">
                <Calendar
                  initialFocus
                  mode="range"
                  defaultMonth={dateRange.from}
                  selected={dateRange}
                  onSelect={setDateRange}
                  numberOfMonths={2}
                />
              </PopoverContent>
            </Popover>
            <Button onClick={handleDateRangeQuery} disabled={!dateRange.from} data-testid="run-range-query">
              Run Query
            </Button>
          </div>

          {rangeTransactions.length > 0 && (
            <div>
              <div className="grid grid-cols-3 gap-4 mb-4">
                <div className="p-4 bg-slate-50 rounded-lg">
                  <p className="text-sm text-slate-500">Transactions</p>
                  <p className="text-xl font-bold">{rangeTransactions.length}</p>
                </div>
                <div className="p-4 bg-emerald-50 rounded-lg">
                  <p className="text-sm text-emerald-600">Income</p>
                  <p className="text-xl font-bold income-text">
                    {formatCurrency(rangeSummary.income)}
                  </p>
                </div>
                <div className="p-4 bg-red-50 rounded-lg">
                  <p className="text-sm text-red-600">Expenses</p>
                  <p className="text-xl font-bold expense-text">
                    {formatCurrency(rangeSummary.expenses)}
                  </p>
                </div>
              </div>

              <div className="max-h-[300px] overflow-y-auto">
                <table className="w-full text-sm">
                  <thead className="sticky top-0 bg-white">
                    <tr className="border-b">
                      <th className="text-left py-2">Date</th>
                      <th className="text-left py-2">Description</th>
                      <th className="text-left py-2">Category</th>
                      <th className="text-right py-2">Amount</th>
                    </tr>
                  </thead>
                  <tbody>
                    {rangeTransactions.map((tx) => (
                      <tr key={tx.id} className="border-b border-slate-100">
                        <td className="py-2 data-value">{tx.date}</td>
                        <td className="py-2">{tx.description || "-"}</td>
                        <td className="py-2">
                          <Badge variant="secondary">{tx.category}</Badge>
                        </td>
                        <td
                          className={`py-2 text-right font-semibold ${
                            tx.type === "income" ? "income-text" : "expense-text"
                          }`}
                        >
                          {tx.type === "income" ? "+" : "-"}
                          {formatCurrency(tx.amount)}
                        </td>
                      </tr>
                    ))}
                  </tbody>
                </table>
              </div>
            </div>
          )}
        </CardContent>
      </Card>

      {/* Two Column Layout for Top Insights */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        {/* Top Expenses (Heap) */}
        <Card data-testid="top-expenses-analytics">
          <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <TrendingDown className="w-5 h-5 text-red-500" />
              Top 10 Expenses
              <span className="ds-badge ds-badge-heap">Max Heap</span>
            </CardTitle>
            <CardDescription>
              Highest expenses extracted using Max Heap (extractMax O(log n))
            </CardDescription>
          </CardHeader>
          <CardContent>
            {topExpenses.length === 0 ? (
              <p className="text-slate-500 text-center py-4">No expenses recorded</p>
            ) : (
              <div className="space-y-2">
                {topExpenses.map((tx, idx) => (
                  <div
                    key={tx.id}
                    className="flex items-center justify-between py-2 border-b border-slate-100 last:border-0"
                    data-testid={`top-expense-${idx}`}
                  >
                    <div className="flex items-center gap-3">
                      <div className="w-6 h-6 rounded-full bg-red-100 flex items-center justify-center text-xs font-bold text-red-600">
                        {idx + 1}
                      </div>
                      <div>
                        <p className="font-medium text-sm">
                          {tx.description || tx.category}
                        </p>
                        <p className="text-xs text-slate-500">
                          {tx.category} • {tx.date}
                        </p>
                      </div>
                    </div>
                    <span className="font-semibold expense-text">
                      {formatCurrency(tx.amount)}
                    </span>
                  </div>
                ))}
              </div>
            )}
          </CardContent>
        </Card>

        {/* Top Categories (Category Heap) */}
        <Card data-testid="top-categories-analytics">
          <CardHeader>
            <CardTitle className="flex items-center gap-2">
              <PieChart className="w-5 h-5 text-primary" />
              Top Spending Categories
              <span className="ds-badge ds-badge-heap">Category Heap</span>
            </CardTitle>
            <CardDescription>
              Categories sorted by total spending using Category Max Heap
            </CardDescription>
          </CardHeader>
          <CardContent>
            {topCategories.length === 0 ? (
              <p className="text-slate-500 text-center py-4">No categories yet</p>
            ) : (
              <div className="space-y-3">
                {topCategories.map((cat, idx) => {
                  const maxAmount = topCategories[0]?.totalAmount || 1;
                  const percentage = (cat.totalAmount / maxAmount) * 100;
                  return (
                    <div key={cat.category} data-testid={`top-cat-${idx}`}>
                      <div className="flex items-center justify-between mb-1">
                        <div className="flex items-center gap-2">
                          <span className="w-5 h-5 rounded bg-primary/10 flex items-center justify-center text-xs font-bold text-primary">
                            {idx + 1}
                          </span>
                          <span className="font-medium text-sm">{cat.category}</span>
                        </div>
                        <span className="font-semibold expense-text">
                          {formatCurrency(cat.totalAmount)}
                        </span>
                      </div>
                      <div className="h-2 bg-slate-100 rounded-full overflow-hidden">
                        <div
                          className="h-full bg-primary/60 transition-all"
                          style={{ width: `${percentage}%` }}
                        />
                      </div>
                    </div>
                  );
                })}
              </div>
            )}
          </CardContent>
        </Card>
      </div>
    </div>
  );
}
