import { useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Alert, AlertDescription, AlertTitle } from "@/components/ui/alert";
import { Badge } from "@/components/ui/badge";
import {
  TrendingUp,
  TrendingDown,
  Wallet,
  AlertTriangle,
  Receipt,
  Activity,
  Zap,
} from "lucide-react";
import { Link } from "react-router-dom";
const API =
  (process.env.REACT_APP_BACKEND_URL || "http://localhost:8001") + "/api";


export default function Dashboard() {
  const [dashboard, setDashboard] = useState(null);
  const [alerts, setAlerts] = useState([]);
  const [recentTx, setRecentTx] = useState([]);
  const [topExpenses, setTopExpenses] = useState([]);
  const [trend7Day, setTrend7Day] = useState(null);
  const [anomalies, setAnomalies] = useState([]);
  const [loading, setLoading] = useState(true);

  const fetchData = async () => {
    try {
      const [dashRes, alertRes, recentRes, topRes, trendRes, anomalyRes] = await Promise.all([
        fetch(`${API}/dashboard`),
        fetch(`${API}/budgets/alerts`),
        fetch(`${API}/transactions/recent?count=5`),
        fetch(`${API}/top-expenses?count=5`),
        fetch(`${API}/trends/7-day`),
        fetch(`${API}/anomalies?threshold=2.0`)
      ]);

      const dashData = await dashRes.json();
      const alertData = await alertRes.json();
      const recentData = await recentRes.json();
      const topData = await topRes.json();
      const trendData = await trendRes.json();
      const anomalyData = await anomalyRes.json();

      setDashboard(dashData);
      setAlerts(alertData.alerts || []);
      setRecentTx(recentData.transactions || []);
      setTopExpenses(topData.topExpenses || []);
      setTrend7Day(trendData.trend || null);
      setAnomalies(anomalyData.anomalies || []);
    } catch (error) {
      console.error("Failed to fetch dashboard:", error);
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchData();
    window.addEventListener("dataChanged", fetchData);
    return () => window.removeEventListener("dataChanged", fetchData);
  }, []);

  const formatCurrency = (amount) => {
    return new Intl.NumberFormat("en-US", {
      style: "currency",
      currency: "USD",
    }).format(amount);
  };

  if (loading) {
    return (
      <div className="page-container flex items-center justify-center min-h-[50vh]">
        <div className="text-slate-500">Loading dashboard...</div>
      </div>
    );
  }

  return (
    <div className="page-container" data-testid="dashboard-page">
      {/* Page Header */}
      <div className="mb-8">
        <h1 className="text-2xl font-bold text-slate-900">Dashboard</h1>
        <p className="text-slate-500 mt-1">Your financial overview at a glance</p>
      </div>

      {/* Budget Alerts */}
      {alerts.length > 0 && (
        <div className="mb-6 space-y-3" data-testid="budget-alerts">
          {alerts.map((alert, idx) => (
            <Alert
              key={idx}
              className={`alert-${alert.level}`}
              data-testid={`alert-${alert.category}`}
            >
              <AlertTriangle className="h-4 w-4" />
              <AlertTitle className="font-semibold">
                {alert.category} Budget {alert.level === "exceeded" ? "Exceeded!" : "Alert"}
              </AlertTitle>
              <AlertDescription>
                {alert.message} (
                <span className="data-value">{alert.percentUsed.toFixed(0)}%</span> used)
              </AlertDescription>
            </Alert>
          ))}
        </div>
      )}

      {/* Anomaly Alerts (Z-Score Detection) */}
      {anomalies.length > 0 && (
        <div className="mb-6 space-y-3" data-testid="anomaly-alerts">
          {anomalies.slice(0, 3).map((anomaly, idx) => (
            <Alert
              key={idx}
              className="border-orange-200 bg-orange-50"
              data-testid={`anomaly-${idx}`}
            >
              <Zap className="h-4 w-4 text-orange-600" />
              <AlertTitle className="font-semibold text-orange-800">
                Unusual {anomaly.isHigh ? "High" : "Low"} Expense Detected
                <span className="ds-badge ds-badge-zscore ml-2">Z-Score: {anomaly.zScore}</span>
              </AlertTitle>
              <AlertDescription className="text-orange-700">
                {anomaly.category}: {formatCurrency(anomaly.amount)} 
                (Average: {formatCurrency(anomaly.mean)}) - {anomaly.description || 'No description'}
              </AlertDescription>
            </Alert>
          ))}
        </div>
      )}

      {/* KPI Cards */}
      <div className="grid grid-cols-1 md:grid-cols-4 gap-4 mb-8">
        <Card className="kpi-card card-hover" data-testid="balance-card">
          <CardHeader className="flex flex-row items-center justify-between pb-2">
            <CardTitle className="text-sm font-medium text-slate-500">
              Total Balance
            </CardTitle>
            <Wallet className="w-5 h-5 text-primary" />
          </CardHeader>
          <CardContent>
            <div
              className={`text-2xl font-bold data-value ${
                dashboard?.balance >= 0 ? "income-text" : "expense-text"
              }`}
            >
              {formatCurrency(dashboard?.balance || 0)}
            </div>
          </CardContent>
        </Card>

        <Card className="kpi-card card-hover" data-testid="income-card">
          <CardHeader className="flex flex-row items-center justify-between pb-2">
            <CardTitle className="text-sm font-medium text-slate-500">
              Total Income
            </CardTitle>
            <TrendingUp className="w-5 h-5 text-emerald-500" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold data-value income-text">
              {formatCurrency(dashboard?.totalIncome || 0)}
            </div>
          </CardContent>
        </Card>

        <Card className="kpi-card card-hover" data-testid="expenses-card">
          <CardHeader className="flex flex-row items-center justify-between pb-2">
            <CardTitle className="text-sm font-medium text-slate-500">
              Total Expenses
            </CardTitle>
            <TrendingDown className="w-5 h-5 text-red-500" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold data-value expense-text">
              {formatCurrency(dashboard?.totalExpenses || 0)}
            </div>
          </CardContent>
        </Card>

        <Card className="kpi-card card-hover" data-testid="count-card">
          <CardHeader className="flex flex-row items-center justify-between pb-2">
            <CardTitle className="text-sm font-medium text-slate-500">
              Transactions
            </CardTitle>
            <Receipt className="w-5 h-5 text-slate-400" />
          </CardHeader>
          <CardContent>
            <div className="text-2xl font-bold data-value text-slate-900">
              {dashboard?.transactionCount || 0}
            </div>
          </CardContent>
        </Card>
      </div>

      {/* 7-Day Spending Trend (Sliding Window) */}
      {trend7Day && (
        <Card className="mb-8 card-hover" data-testid="spending-trend-card">
          <CardHeader className="flex flex-row items-center justify-between">
            <div>
              <CardTitle className="text-lg flex items-center gap-2">
                <Activity className="w-5 h-5 text-primary" />
                7-Day Spending Trend
              </CardTitle>
              <span className="ds-badge ds-badge-window mt-1">Sliding Window</span>
            </div>
            <Badge variant={trend7Day.trend === 'increasing' ? 'destructive' : trend7Day.trend === 'decreasing' ? 'success' : 'secondary'}>
              {trend7Day.trend === 'increasing' ? '↑ Increasing' : trend7Day.trend === 'decreasing' ? '↓ Decreasing' : '→ Stable'}
            </Badge>
          </CardHeader>
          <CardContent>
            <div className="grid grid-cols-3 gap-4 mb-4">
              <div className="p-4 bg-slate-50 rounded-lg text-center">
                <p className="text-sm text-slate-500">Total Expenses</p>
                <p className="text-xl font-bold expense-text">{formatCurrency(trend7Day.totalExpenses)}</p>
              </div>
              <div className="p-4 bg-emerald-50 rounded-lg text-center">
                <p className="text-sm text-emerald-600">Total Income</p>
                <p className="text-xl font-bold income-text">{formatCurrency(trend7Day.totalIncome)}</p>
              </div>
              <div className="p-4 bg-primary/5 rounded-lg text-center">
                <p className="text-sm text-primary">Avg Daily Expense</p>
                <p className="text-xl font-bold text-primary">{formatCurrency(trend7Day.avgDailyExpense)}</p>
              </div>
            </div>
            <p className="text-xs text-slate-500 text-center">
              Period: {trend7Day.startDate} to {trend7Day.endDate}
            </p>
          </CardContent>
        </Card>
      )}

      {/* Two Column Layout */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        {/* Recent Transactions (Stack) */}
        <Card className="card-hover" data-testid="recent-transactions-card">
          <CardHeader className="flex flex-row items-center justify-between">
            <div>
              <CardTitle className="text-lg">Recent Transactions</CardTitle>
              <span className="ds-badge ds-badge-stack mt-1">Stack (LIFO)</span>
            </div>
            <Link
              to="/transactions"
              className="text-sm text-primary hover:underline"
            >
              View all
            </Link>
          </CardHeader>
          <CardContent>
            {recentTx.length === 0 ? (
              <p className="text-slate-500 text-sm py-4 text-center">
                No transactions yet. Add your first transaction!
              </p>
            ) : (
              <div className="space-y-3">
                {recentTx.map((tx, idx) => (
                  <div
                    key={tx.id}
                    className="flex items-center justify-between py-2 border-b border-slate-100 last:border-0"
                    data-testid={`recent-tx-${idx}`}
                  >
                    <div className="flex items-center gap-3">
                      {idx === 0 && (
                        <Badge variant="outline" className="text-xs">
                          TOP
                        </Badge>
                      )}
                      <div>
                        <p className="font-medium text-sm text-slate-900">
                          {tx.description || tx.category}
                        </p>
                        <p className="text-xs text-slate-500">
                          {tx.category} • {tx.date}
                        </p>
                      </div>
                    </div>
                    <span
                      className={`data-value font-semibold ${
                        tx.type === "income" ? "income-text" : "expense-text"
                      }`}
                    >
                      {tx.type === "income" ? "+" : "-"}
                      {formatCurrency(tx.amount)}
                    </span>
                  </div>
                ))}
              </div>
            )}
          </CardContent>
        </Card>

        {/* Top Expenses (Heap) */}
        <Card className="card-hover" data-testid="top-expenses-card">
          <CardHeader className="flex flex-row items-center justify-between">
            <div>
              <CardTitle className="text-lg">Top Expenses</CardTitle>
              <span className="ds-badge ds-badge-heap mt-1">Max Heap</span>
            </div>
            <Link
              to="/analytics"
              className="text-sm text-primary hover:underline"
            >
              View analytics
            </Link>
          </CardHeader>
          <CardContent>
            {topExpenses.length === 0 ? (
              <p className="text-slate-500 text-sm py-4 text-center">
                No expenses recorded yet.
              </p>
            ) : (
              <div className="space-y-3">
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
                        <p className="font-medium text-sm text-slate-900">
                          {tx.description || tx.category}
                        </p>
                        <p className="text-xs text-slate-500">
                          {tx.category} • {tx.date}
                        </p>
                      </div>
                    </div>
                    <span className="data-value font-semibold expense-text">
                      {formatCurrency(tx.amount)}
                    </span>
                  </div>
                ))}
              </div>
            )}
          </CardContent>
        </Card>
      </div>
    </div>
  );
}
