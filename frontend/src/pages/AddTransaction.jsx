import { useState, useEffect } from "react";
import { useNavigate } from "react-router-dom";
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Textarea } from "@/components/ui/textarea";
import { Calendar } from "@/components/ui/calendar";
import { Popover, PopoverContent, PopoverTrigger } from "@/components/ui/popover";
import {
  Select,
  SelectContent,
  SelectItem,
  SelectTrigger,
  SelectValue,
} from "@/components/ui/select";
import { Calendar as CalendarIcon, TrendingUp, TrendingDown } from "lucide-react";
import { toast } from "sonner";
import { format } from "date-fns";
const API =
  (process.env.REACT_APP_BACKEND_URL || "http://localhost:8001") + "/api";


export default function AddTransaction() {
  const navigate = useNavigate();
  const [loading, setLoading] = useState(false);
  const [categories, setCategories] = useState([]);
  const [suggestions, setSuggestions] = useState([]);
  const [form, setForm] = useState({
    type: "expense",
    amount: "",
    category: "",
    description: "",
    date: new Date(),
  });

  // Fetch categories
  useEffect(() => {
    const fetchCategories = async () => {
      try {
        const res = await fetch(`${API}/categories`);
        const data = await res.json();
        setCategories(data.categories || []);
      } catch {
        // Use defaults
        setCategories([
          "Food", "Transport", "Shopping", "Entertainment", "Bills",
          "Healthcare", "Education", "Salary", "Freelance", "Investment"
        ]);
      }
    };
    fetchCategories();
  }, []);

  // Category autocomplete using Trie
  const handleCategoryInput = async (value) => {
    setForm({ ...form, category: value });
    if (value.length >= 1) {
      try {
        const res = await fetch(`${API}/categories/suggest?prefix=${encodeURIComponent(value)}`);
        const data = await res.json();
        setSuggestions(data.suggestions || []);
      } catch {
        setSuggestions([]);
      }
    } else {
      setSuggestions([]);
    }
  };

  const handleSubmit = async (e) => {
  e.preventDefault();

  if (!form.amount || !form.category) {
    toast.error("Please fill in required fields");
    return;
  }

  setLoading(true);

  try {
    const res = await fetch(`${API}/transactions`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({
        type: form.type,
        amount: Number(form.amount),
        category: form.category,
        description: form.description,
        date: format(form.date, "yyyy-MM-dd"),
      }),
    });

    if (!res.ok) {
      throw new Error("HTTP error " + res.status);
    }

    const data = await res.json();

    if (data.success) {
      toast.success("Transaction added");
      setLoading(false);                 // 🔴 IMPORTANT
      window.dispatchEvent(new Event("dataChanged"));
      navigate("/transactions");
      return;
    }

    throw new Error("API returned success=false");
  } catch (err) {
    console.error(err);
    toast.error("Failed to add transaction");
    setLoading(false);                   // 🔴 IMPORTANT
  }
};

  return (
    <div className="page-container" data-testid="add-transaction-page">
      {/* Header */}
      <div className="mb-8">
        <h1 className="text-2xl font-bold text-slate-900">Add Transaction</h1>
        <p className="text-slate-500 mt-1">
          Record a new income or expense
          <span className="ds-badge ds-badge-linkedlist ml-2">Linked List</span>
          <span className="ds-badge ds-badge-bst ml-1">BST</span>
        </p>
      </div>

      <div className="max-w-2xl">
        <Card>
          <CardHeader>
            <CardTitle>Transaction Details</CardTitle>
            <CardDescription>
              Fill in the details below. Category autocomplete uses Trie data structure.
            </CardDescription>
          </CardHeader>
          <CardContent>
            <form onSubmit={handleSubmit} className="space-y-6">
              {/* Transaction Type */}
              <div className="space-y-2">
                <Label>Transaction Type</Label>
                <div className="flex gap-4">
                  <Button
                    type="button"
                    variant={form.type === "income" ? "default" : "outline"}
                    className={form.type === "income" ? "bg-emerald-500 hover:bg-emerald-600" : ""}
                    onClick={() => setForm({ ...form, type: "income" })}
                    data-testid="type-income-btn"
                  >
                    <TrendingUp className="w-4 h-4 mr-2" />
                    Income
                  </Button>
                  <Button
                    type="button"
                    variant={form.type === "expense" ? "default" : "outline"}
                    className={form.type === "expense" ? "bg-red-500 hover:bg-red-600" : ""}
                    onClick={() => setForm({ ...form, type: "expense" })}
                    data-testid="type-expense-btn"
                  >
                    <TrendingDown className="w-4 h-4 mr-2" />
                    Expense
                  </Button>
                </div>
              </div>

              {/* Amount */}
              <div className="space-y-2">
                <Label htmlFor="amount">Amount *</Label>
                <div className="relative">
                  <span className="absolute left-3 top-1/2 -translate-y-1/2 text-slate-500">$</span>
                  <Input
                    id="amount"
                    type="number"
                    step="0.01"
                    min="0.01"
                    placeholder="0.00"
                    value={form.amount}
                    onChange={(e) => setForm({ ...form, amount: e.target.value })}
                    className="pl-8 data-value"
                    required
                    data-testid="amount-input"
                  />
                </div>
              </div>

              {/* Category with Autocomplete */}
              <div className="space-y-2">
                <Label htmlFor="category">
                  Category *
                  <span className="ds-badge ds-badge-trie ml-2">Trie Autocomplete</span>
                </Label>
                <div className="relative">
                  <Input
                    id="category"
                    placeholder="Start typing a category..."
                    value={form.category}
                    onChange={(e) => handleCategoryInput(e.target.value)}
                    required
                    data-testid="category-input"
                  />
                  {suggestions.length > 0 && (
                    <div className="absolute z-10 w-full mt-1 bg-white border border-slate-200 rounded-md shadow-lg">
                      {suggestions.map((cat) => (
                        <button
                          key={cat}
                          type="button"
                          className="w-full px-4 py-2 text-left hover:bg-slate-100 text-sm"
                          onClick={() => {
                            setForm({ ...form, category: cat });
                            setSuggestions([]);
                          }}
                          data-testid={`suggestion-${cat}`}
                        >
                          {cat}
                        </button>
                      ))}
                    </div>
                  )}
                </div>
                {/* Quick category buttons */}
                <div className="flex flex-wrap gap-2 mt-2">
                  {categories.slice(0, 8).map((cat) => (
                    <Button
                      key={cat}
                      type="button"
                      variant="outline"
                      size="sm"
                      onClick={() => setForm({ ...form, category: cat })}
                      className="text-xs"
                    >
                      {cat}
                    </Button>
                  ))}
                </div>
              </div>

              {/* Description */}
              <div className="space-y-2">
                <Label htmlFor="description">Description (Optional)</Label>
                <Textarea
                  id="description"
                  placeholder="Add a note or description..."
                  value={form.description}
                  onChange={(e) => setForm({ ...form, description: e.target.value })}
                  rows={3}
                  data-testid="description-input"
                />
              </div>

              {/* Date */}
              <div className="space-y-2">
                <Label>Date</Label>
                <Popover>
                  <PopoverTrigger asChild>
                    <Button
                      variant="outline"
                      className="w-full justify-start text-left font-normal"
                      data-testid="date-picker-btn"
                    >
                      <CalendarIcon className="mr-2 h-4 w-4" />
                      {form.date ? format(form.date, "PPP") : "Select date"}
                    </Button>
                  </PopoverTrigger>
                  <PopoverContent className="w-auto p-0" align="start">
                    <Calendar
                      mode="single"
                      selected={form.date}
                      onSelect={(date) => setForm({ ...form, date: date || new Date() })}
                      initialFocus
                    />
                  </PopoverContent>
                </Popover>
              </div>

              {/* Submit */}
              <div className="flex gap-4 pt-4">
                <Button
                  type="submit"
                  disabled={loading}
                  className="flex-1"
                  data-testid="submit-transaction-btn"
                >
                  {loading ? "Adding..." : "Add Transaction"}
                </Button>
                <Button
                  type="button"
                  variant="outline"
                  onClick={() => navigate("/transactions")}
                  data-testid="cancel-btn"
                >
                  Cancel
                </Button>
              </div>
            </form>
          </CardContent>
        </Card>
      </div>
    </div>
  );
}
