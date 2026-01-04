import { useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from "@/components/ui/card";
import { Button } from "@/components/ui/button";
import { Input } from "@/components/ui/input";
import { Label } from "@/components/ui/label";
import { Badge } from "@/components/ui/badge";
import { Calendar } from "@/components/ui/calendar";
import { Popover, PopoverContent, PopoverTrigger } from "@/components/ui/popover";
import {
  Dialog,
  DialogContent,
  DialogDescription,
  DialogFooter,
  DialogHeader,
  DialogTitle,
  DialogTrigger,
} from "@/components/ui/dialog";
import {
  Plus,
  Calendar as CalendarIcon,
  CheckCircle,
  Trash2,
  Clock,
  AlertTriangle,
} from "lucide-react";
import { toast } from "sonner";
import { format, isBefore, parseISO, startOfDay } from "date-fns";
const API =
  (process.env.REACT_APP_BACKEND_URL || "http://localhost:8001") + "/api";


export default function Bills() {
  const [bills, setBills] = useState([]);
  const [categories, setCategories] = useState([]);
  const [loading, setLoading] = useState(true);
  const [dialogOpen, setDialogOpen] = useState(false);
  const [form, setForm] = useState({
    name: "",
    amount: "",
    dueDate: null,
    category: "",
  });

  const fetchData = async () => {
    try {
      const [billRes, catRes] = await Promise.all([
        fetch(`${API}/bills`),
        fetch(`${API}/categories`),
      ]);
      const billData = await billRes.json();
      const catData = await catRes.json();
      setBills(billData.bills || []);
      setCategories(catData.categories || []);
    } catch {
      toast.error("Failed to load bills");
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
    if (!form.name || !form.amount || !form.dueDate || !form.category) {
      toast.error("Please fill in all fields");
      return;
    }

    try {
      const res = await fetch(`${API}/bills`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({
          name: form.name,
          amount: parseFloat(form.amount),
          dueDate: format(form.dueDate, "yyyy-MM-dd"),
          category: form.category,
        }),
      });
      const data = await res.json();
      if (data.success) {
        toast.success("Bill added to queue!", {
          description: `${form.name} due ${format(form.dueDate, "MMM d, yyyy")}`,
        });
        setDialogOpen(false);
        setForm({ name: "", amount: "", dueDate: null, category: "" });
        fetchData();
        window.dispatchEvent(new Event("dataChanged"));
      }
    } catch {
      toast.error("Failed to add bill");
    }
  };

  const handlePayBill = async (id) => {
    try {
      const res = await fetch(`${API}/bills/${id}/pay`, { method: "POST" });
      const data = await res.json();
      if (data.success) {
        toast.success("Bill marked as paid!");
        fetchData();
        window.dispatchEvent(new Event("dataChanged"));
      }
    } catch {
      toast.error("Failed to mark bill as paid");
    }
  };

  const handleDeleteBill = async (id) => {
    if (!window.confirm("Delete this bill?")) return;

    try {
      const res = await fetch(`${API}/bills/${id}`, { method: "DELETE" });
      const data = await res.json();
      if (data.success) {
        toast.success("Bill removed");
        fetchData();
        window.dispatchEvent(new Event("dataChanged"));
      }
    } catch {
      toast.error("Failed to delete bill");
    }
  };

  const formatCurrency = (amount) => {
    return new Intl.NumberFormat("en-US", {
      style: "currency",
      currency: "USD",
    }).format(amount);
  };

  const isOverdue = (dateStr) => {
    return isBefore(parseISO(dateStr), startOfDay(new Date()));
  };

  const unpaidBills = bills.filter((b) => !b.isPaid);
  const paidBills = bills.filter((b) => b.isPaid);

  if (loading) {
    return (
      <div className="page-container flex items-center justify-center min-h-[50vh]">
        <div className="text-slate-500">Loading bills...</div>
      </div>
    );
  }

  return (
    <div className="page-container" data-testid="bills-page">
      {/* Header */}
      <div className="flex items-center justify-between mb-8">
        <div>
          <h1 className="text-2xl font-bold text-slate-900">Upcoming Bills</h1>
          <p className="text-slate-500 mt-1">
            Manage scheduled payments in order
            <span className="ds-badge ds-badge-queue ml-2">Queue (FIFO)</span>
          </p>
        </div>
        <Dialog open={dialogOpen} onOpenChange={setDialogOpen}>
          <DialogTrigger asChild>
            <Button data-testid="add-bill-btn">
              <Plus className="w-4 h-4 mr-2" />
              Add Bill
            </Button>
          </DialogTrigger>
          <DialogContent>
            <DialogHeader>
              <DialogTitle>Add New Bill</DialogTitle>
              <DialogDescription>
                Add a recurring or one-time bill to the payment queue.
              </DialogDescription>
            </DialogHeader>
            <form onSubmit={handleSubmit}>
              <div className="space-y-4 py-4">
                <div className="space-y-2">
                  <Label htmlFor="bill-name">Bill Name</Label>
                  <Input
                    id="bill-name"
                    placeholder="e.g., Electricity Bill"
                    value={form.name}
                    onChange={(e) => setForm({ ...form, name: e.target.value })}
                    data-testid="bill-name-input"
                  />
                </div>
                <div className="space-y-2">
                  <Label htmlFor="bill-amount">Amount ($)</Label>
                  <Input
                    id="bill-amount"
                    type="number"
                    step="0.01"
                    min="0.01"
                    placeholder="100.00"
                    value={form.amount}
                    onChange={(e) => setForm({ ...form, amount: e.target.value })}
                    data-testid="bill-amount-input"
                  />
                </div>
                <div className="space-y-2">
                  <Label>Due Date</Label>
                  <Popover>
                    <PopoverTrigger asChild>
                      <Button
                        variant="outline"
                        className="w-full justify-start"
                        data-testid="bill-date-btn"
                      >
                        <CalendarIcon className="mr-2 h-4 w-4" />
                        {form.dueDate ? format(form.dueDate, "PPP") : "Select date"}
                      </Button>
                    </PopoverTrigger>
                    <PopoverContent className="w-auto p-0" align="start">
                      <Calendar
                        mode="single"
                        selected={form.dueDate}
                        onSelect={(date) => setForm({ ...form, dueDate: date })}
                        initialFocus
                      />
                    </PopoverContent>
                  </Popover>
                </div>
                <div className="space-y-2">
                  <Label htmlFor="bill-category">Category</Label>
                  <select
                    id="bill-category"
                    className="w-full border border-slate-200 rounded-md p-2"
                    value={form.category}
                    onChange={(e) => setForm({ ...form, category: e.target.value })}
                    data-testid="bill-category-select"
                  >
                    <option value="">Select a category...</option>
                    {categories.map((cat) => (
                      <option key={cat} value={cat}>
                        {cat}
                      </option>
                    ))}
                  </select>
                </div>
              </div>
              <DialogFooter>
                <Button type="button" variant="outline" onClick={() => setDialogOpen(false)}>
                  Cancel
                </Button>
                <Button type="submit" data-testid="save-bill-btn">Add to Queue</Button>
              </DialogFooter>
            </form>
          </DialogContent>
        </Dialog>
      </div>

      {/* Queue Visualization */}
      <Card className="mb-6 bg-slate-50">
        <CardHeader className="pb-2">
          <CardTitle className="text-sm font-medium text-slate-500 flex items-center gap-2">
            Queue Status
            <span className="ds-badge ds-badge-queue">FIFO</span>
          </CardTitle>
        </CardHeader>
        <CardContent>
          {unpaidBills.length === 0 ? (
            <p className="text-slate-500 text-center py-4">Queue is empty</p>
          ) : (
            <div className="flex items-center gap-2 overflow-x-auto pb-2">
              <Badge variant="outline" className="shrink-0">Front</Badge>
              {unpaidBills.map((bill, idx) => (
                <div key={bill.id} className="flex items-center gap-2">
                  <div
                    className={`px-3 py-2 rounded border text-sm whitespace-nowrap ${
                      isOverdue(bill.dueDate)
                        ? "bg-red-50 border-red-200"
                        : "bg-white border-slate-200"
                    }`}
                  >
                    {bill.name}
                  </div>
                  {idx < unpaidBills.length - 1 && (
                    <span className="text-slate-300">→</span>
                  )}
                </div>
              ))}
              <Badge variant="outline" className="shrink-0">Rear</Badge>
            </div>
          )}
        </CardContent>
      </Card>

      {/* Bills Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        {/* Unpaid Bills */}
        <div>
          <h2 className="text-lg font-semibold mb-4 flex items-center gap-2">
            <Clock className="w-5 h-5 text-slate-400" />
            Pending ({unpaidBills.length})
          </h2>
          <div className="space-y-3">
            {unpaidBills.length === 0 ? (
              <Card>
                <CardContent className="py-8 text-center text-slate-500">
                  No pending bills
                </CardContent>
              </Card>
            ) : (
              unpaidBills.map((bill) => (
                <Card
                  key={bill.id}
                  className={`card-hover ${
                    isOverdue(bill.dueDate) ? "border-red-200 bg-red-50/50" : ""
                  }`}
                  data-testid={`bill-card-${bill.id}`}
                >
                  <CardContent className="py-4">
                    <div className="flex items-center justify-between">
                      <div className="flex-1">
                        <div className="flex items-center gap-2 mb-1">
                          <span className="font-semibold">{bill.name}</span>
                          {isOverdue(bill.dueDate) && (
                            <Badge variant="destructive" className="text-xs">
                              <AlertTriangle className="w-3 h-3 mr-1" />
                              Overdue
                            </Badge>
                          )}
                        </div>
                        <div className="text-sm text-slate-500">
                          <span>{bill.category}</span>
                          <span className="mx-2">•</span>
                          <span className="data-value">Due: {bill.dueDate}</span>
                        </div>
                      </div>
                      <div className="flex items-center gap-3">
                        <span className="data-value font-bold text-lg">
                          {formatCurrency(bill.amount)}
                        </span>
                        <Button
                          size="sm"
                          onClick={() => handlePayBill(bill.id)}
                          className="bg-emerald-500 hover:bg-emerald-600"
                          data-testid={`pay-bill-${bill.id}`}
                        >
                          <CheckCircle className="w-4 h-4 mr-1" />
                          Pay
                        </Button>
                        <Button
                          size="sm"
                          variant="ghost"
                          onClick={() => handleDeleteBill(bill.id)}
                          className="text-slate-400 hover:text-red-500"
                          data-testid={`delete-bill-${bill.id}`}
                        >
                          <Trash2 className="w-4 h-4" />
                        </Button>
                      </div>
                    </div>
                  </CardContent>
                </Card>
              ))
            )}
          </div>
        </div>

        {/* Paid Bills */}
        <div>
          <h2 className="text-lg font-semibold mb-4 flex items-center gap-2">
            <CheckCircle className="w-5 h-5 text-emerald-500" />
            Paid ({paidBills.length})
          </h2>
          <div className="space-y-3">
            {paidBills.length === 0 ? (
              <Card>
                <CardContent className="py-8 text-center text-slate-500">
                  No paid bills yet
                </CardContent>
              </Card>
            ) : (
              paidBills.map((bill) => (
                <Card
                  key={bill.id}
                  className="opacity-60"
                  data-testid={`paid-bill-${bill.id}`}
                >
                  <CardContent className="py-4">
                    <div className="flex items-center justify-between">
                      <div>
                        <div className="flex items-center gap-2 mb-1">
                          <span className="font-medium line-through text-slate-500">
                            {bill.name}
                          </span>
                          <Badge variant="secondary" className="text-xs bg-emerald-100 text-emerald-700">
                            Paid
                          </Badge>
                        </div>
                        <div className="text-sm text-slate-400">
                          {bill.category} • {bill.dueDate}
                        </div>
                      </div>
                      <span className="data-value text-slate-400">
                        {formatCurrency(bill.amount)}
                      </span>
                    </div>
                  </CardContent>
                </Card>
              ))
            )}
          </div>
        </div>
      </div>
    </div>
  );
}
