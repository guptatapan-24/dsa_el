import { useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Badge } from "@/components/ui/badge";
import {
  Table,
  TableBody,
  TableCell,
  TableHead,
  TableHeader,
  TableRow,
} from "@/components/ui/table";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import { Calendar } from "@/components/ui/calendar";
import { Popover, PopoverContent, PopoverTrigger } from "@/components/ui/popover";
import { Trash2, Calendar as CalendarIcon, Search } from "lucide-react";
import { toast } from "sonner";
import { format } from "date-fns";
import { Link } from "react-router-dom";

const API = `${process.env.REACT_APP_BACKEND_URL}/api`;

export default function Transactions() {
  const [transactions, setTransactions] = useState([]);
  const [filteredTx, setFilteredTx] = useState([]);
  const [loading, setLoading] = useState(true);
  const [searchTerm, setSearchTerm] = useState("");
  const [typeFilter, setTypeFilter] = useState("all");
  const [dateRange, setDateRange] = useState({ from: null, to: null });
  const [dsInfo, setDsInfo] = useState("");

  const fetchTransactions = async () => {
    try {
      const res = await fetch(`${API}/transactions`);
      const data = await res.json();
      setTransactions(data.transactions || []);
      setFilteredTx(data.transactions || []);
      setDsInfo(data.dsInfo || "");
    } catch (error) {
      console.error("Failed to fetch transactions:", error);
      toast.error("Failed to load transactions");
    } finally {
      setLoading(false);
    }
  };

  useEffect(() => {
    fetchTransactions();
    window.addEventListener("dataChanged", fetchTransactions);
    return () => window.removeEventListener("dataChanged", fetchTransactions);
  }, []);

  // Apply filters
  useEffect(() => {
    let filtered = [...transactions];

    // Search filter
    if (searchTerm) {
      const term = searchTerm.toLowerCase();
      filtered = filtered.filter(
        (tx) =>
          tx.description.toLowerCase().includes(term) ||
          tx.category.toLowerCase().includes(term)
      );
    }

    // Type filter
    if (typeFilter !== "all") {
      filtered = filtered.filter((tx) => tx.type === typeFilter);
    }

    setFilteredTx(filtered);
  }, [searchTerm, typeFilter, transactions]);

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
      setFilteredTx(data.transactions || []);
      setDsInfo(data.dsInfo || "BST range query");
      toast.success(`Found ${data.transactions?.length || 0} transactions in range`);
    } catch {
      toast.error("Range query failed");
    }
  };

  const handleDelete = async (id) => {
    if (!window.confirm("Delete this transaction?")) return;

    try {
      const res = await fetch(`${API}/transactions/${id}`, { method: "DELETE" });
      const data = await res.json();
      if (data.success) {
        toast.success("Transaction deleted");
        fetchTransactions();
        window.dispatchEvent(new Event("dataChanged"));
      } else {
        toast.error("Delete failed");
      }
    } catch {
      toast.error("Delete failed");
    }
  };

  const formatCurrency = (amount) => {
    return new Intl.NumberFormat("en-US", {
      style: "currency",
      currency: "USD",
    }).format(amount);
  };

  const clearFilters = () => {
    setSearchTerm("");
    setTypeFilter("all");
    setDateRange({ from: null, to: null });
    setFilteredTx(transactions);
    setDsInfo("");
  };

  if (loading) {
    return (
      <div className="page-container flex items-center justify-center min-h-[50vh]">
        <div className="text-slate-500">Loading transactions...</div>
      </div>
    );
  }

  return (
    <div className="page-container" data-testid="transactions-page">
      {/* Header */}
      <div className="flex items-center justify-between mb-6">
        <div>
          <h1 className="text-2xl font-bold text-slate-900">Transactions</h1>
          <p className="text-slate-500 mt-1">
            View and manage all your transactions
            {dsInfo && (
              <span className="ds-badge ds-badge-bst ml-2">{dsInfo}</span>
            )}
          </p>
        </div>
        <Link to="/add-transaction">
          <Button data-testid="add-transaction-btn">Add Transaction</Button>
        </Link>
      </div>

      {/* Filters */}
      <Card className="mb-6">
        <CardHeader>
          <CardTitle className="text-base">Filters & Search</CardTitle>
        </CardHeader>
        <CardContent>
          <div className="flex flex-wrap gap-4 items-end">
            {/* Search */}
            <div className="flex-1 min-w-[200px]">
              <label className="text-sm text-slate-500 mb-1 block">Search</label>
              <div className="relative">
                <Search className="absolute left-3 top-1/2 -translate-y-1/2 w-4 h-4 text-slate-400" />
                <Input
                  placeholder="Search by description or category..."
                  value={searchTerm}
                  onChange={(e) => setSearchTerm(e.target.value)}
                  className="pl-9"
                  data-testid="search-input"
                />
              </div>
            </div>

            {/* Type Filter */}
            <div className="w-[150px]">
              <label className="text-sm text-slate-500 mb-1 block">Type</label>
              <Select value={typeFilter} onValueChange={setTypeFilter}>
                <SelectTrigger data-testid="type-filter">
                  <SelectValue placeholder="All types" />
                </SelectTrigger>
                <SelectContent>
                  <SelectItem value="all">All Types</SelectItem>
                  <SelectItem value="income">Income</SelectItem>
                  <SelectItem value="expense">Expense</SelectItem>
                </SelectContent>
              </Select>
            </div>

            {/* Date Range */}
            <div>
              <label className="text-sm text-slate-500 mb-1 block">
                Date Range (BST Query)
              </label>
              <Popover>
                <PopoverTrigger asChild>
                  <Button variant="outline" className="w-[220px] justify-start" data-testid="date-range-btn">
                    <CalendarIcon className="mr-2 h-4 w-4" />
                    {dateRange.from ? (
                      dateRange.to ? (
                        `${format(dateRange.from, "MMM d")} - ${format(dateRange.to, "MMM d")}`
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
            </div>

            <Button onClick={handleDateRangeQuery} disabled={!dateRange.from} data-testid="apply-range-btn">
              Apply Range
            </Button>

            <Button variant="ghost" onClick={clearFilters} data-testid="clear-filters-btn">
              Clear
            </Button>
          </div>
        </CardContent>
      </Card>

      {/* Transactions Table */}
      <Card data-testid="transactions-table">
        <CardContent className="p-0">
          {filteredTx.length === 0 ? (
            <div className="py-12 text-center text-slate-500">
              <p>No transactions found.</p>
              <Link to="/add-transaction" className="text-primary hover:underline mt-2 inline-block">
                Add your first transaction
              </Link>
            </div>
          ) : (
            <Table>
              <TableHeader>
                <TableRow>
                  <TableHead>Date</TableHead>
                  <TableHead>Description</TableHead>
                  <TableHead>Category</TableHead>
                  <TableHead>Type</TableHead>
                  <TableHead className="text-right">Amount</TableHead>
                  <TableHead className="w-[50px]"></TableHead>
                </TableRow>
              </TableHeader>
              <TableBody>
                {filteredTx.map((tx) => (
                  <TableRow key={tx.id} data-testid={`tx-row-${tx.id}`}>
                    <TableCell className="data-value text-sm">
                      {tx.date}
                    </TableCell>
                    <TableCell className="font-medium">
                      {tx.description || "-"}
                    </TableCell>
                    <TableCell>
                      <Badge variant="secondary">{tx.category}</Badge>
                    </TableCell>
                    <TableCell>
                      <Badge
                        variant={tx.type === "income" ? "default" : "destructive"}
                        className={tx.type === "income" ? "bg-emerald-500" : ""}
                      >
                        {tx.type}
                      </Badge>
                    </TableCell>
                    <TableCell
                      className={`text-right data-value font-semibold ${
                        tx.type === "income" ? "income-text" : "expense-text"
                      }`}
                    >
                      {tx.type === "income" ? "+" : "-"}
                      {formatCurrency(tx.amount)}
                    </TableCell>
                    <TableCell>
                      <Button
                        variant="ghost"
                        size="icon"
                        onClick={() => handleDelete(tx.id)}
                        className="text-slate-400 hover:text-red-500"
                        data-testid={`delete-tx-${tx.id}`}
                      >
                        <Trash2 className="w-4 h-4" />
                      </Button>
                    </TableCell>
                  </TableRow>
                ))}
              </TableBody>
            </Table>
          )}
        </CardContent>
      </Card>

      {/* Summary */}
      <div className="mt-4 text-sm text-slate-500">
        Showing {filteredTx.length} of {transactions.length} transactions
        <span className="ds-badge ds-badge-linkedlist ml-2">Doubly Linked List</span>
      </div>
    </div>
  );
}
