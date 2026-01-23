import { useEffect, useState } from "react";
import { Card, CardContent, CardHeader, CardTitle, CardDescription } from "@/components/ui/card";
import {
  Accordion,
  AccordionContent,
  AccordionItem,
  AccordionTrigger,
} from "@/components/ui/accordion";
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
  BookOpen,
  Code2,
  Cpu,
  HelpCircle,
  CheckCircle,
  Lightbulb,
} from "lucide-react";
import { toast } from "sonner";
const API =
  (process.env.REACT_APP_BACKEND_URL || "http://localhost:8001") + "/api";


export default function DSAInfo() {
  const [dsaInfo, setDsaInfo] = useState(null);
  const [loading, setLoading] = useState(true);

  useEffect(() => {
    const fetchInfo = async () => {
      try {
        const res = await fetch(`${API}/dsa-info`);
        const data = await res.json();
        setDsaInfo(data);
      } catch {
        toast.error("Failed to load DSA info");
      } finally {
        setLoading(false);
      }
    };
    fetchInfo();
  }, []);

  // Viva preparation questions - Updated for new data structures
  const vivaQuestions = [
    {
      q: "Why use a Red-Black Tree instead of a regular BST for transactions?",
      a: "Red-Black Tree provides guaranteed O(log n) operations even in worst case. Regular BST can degrade to O(n) with skewed data (e.g., transactions added chronologically). Red-Black Tree maintains balance through color constraints and rotations, ensuring consistent performance for our date-ordered transactions.",
    },
    {
      q: "How does Skip List provide O(log n) expected time for ID lookups?",
      a: "Skip List creates multiple layers of linked lists with probabilistic level assignment. Each level skips over elements in lower levels, creating 'express lanes'. On average, searching skips log(n) elements at each level, giving O(log n) expected time. Unlike balanced trees, it's simpler to implement and has good cache performance.",
    },
    {
      q: "Why use an Indexed Priority Queue for budget alerts?",
      a: "Indexed Priority Queue allows both priority-based extraction AND efficient updates to existing priorities in O(log n). When budget spending changes, we need to update the alert priority without removing and re-inserting. Regular heaps don't support efficient updates. IPQ maintains an index map for O(1) lookup of positions.",
    },
    {
      q: "How does Polynomial Hashing improve category lookups?",
      a: "Polynomial hashing creates hash values using the formula: h = (c₁×p^(n-1) + c₂×p^(n-2) + ... + cₙ) mod m. This produces better distribution than simple sum-of-characters, reducing collisions. For category names like 'Food' vs 'Doof', polynomial hashing produces different values, improving O(1) average lookup time.",
    },
    {
      q: "Explain the Sliding Window algorithm for spending trends.",
      a: "Sliding Window maintains a fixed-size window over the data stream. For 7-day trends, we keep daily aggregates and slide the window each day. When adding new day: add new value, remove oldest. This gives O(1) per update instead of O(n) recalculation. We precompute daily spending totals to enable efficient window operations.",
    },
    {
      q: "Why IntroSort instead of QuickSort for expense ranking?",
      a: "IntroSort combines QuickSort, HeapSort, and InsertionSort. It starts with QuickSort for its good average performance, but switches to HeapSort if recursion depth exceeds 2×log(n) - preventing O(n²) worst case. For small partitions, InsertionSort is used for its low overhead. SQLite uses this hybrid approach for ORDER BY.",
    },
    {
      q: "How does Z-Score anomaly detection work in real-time?",
      a: "Z-Score measures how many standard deviations a value is from the mean: Z = (x - μ) / σ. For streaming data, we use Welford's algorithm to compute running mean and variance in O(1) per update without storing all values. If |Z| > threshold (typically 2.0), we flag as anomaly. This enables real-time detection as transactions are added.",
    },
    {
      q: "What's the advantage of SQLite B-Tree over in-memory structures?",
      a: "SQLite B-Tree provides: 1) Persistence - data survives restarts, 2) Memory efficiency - only loads needed pages, 3) ACID compliance - transactions are reliable, 4) Concurrent access - multiple readers, one writer, 5) Built-in indexing - automatic B-Tree for primary keys. For financial data, persistence and reliability are critical.",
    },
    {
      q: "How would you scale this system for millions of transactions?",
      a: "For scale: 1) Partition by date ranges (sharding), 2) Add read replicas for analytics queries, 3) Use materialized views for aggregates, 4) Implement connection pooling, 5) Add caching layer (Redis) for hot data, 6) Consider columnar storage for analytics. The current O(log n) operations scale well, but I/O becomes the bottleneck.",
    },
    {
      q: "What's the space-time tradeoff in your anomaly detection?",
      a: "Using Welford's algorithm, we store only 3 values per category (count, mean, M2) for O(1) space per category, O(k) total for k categories. Traditional approach would store all values for O(n) space. Tradeoff: We can't recompute if a transaction is deleted (would need rebuild). Also, early transactions have less reliable statistics.",
    },
  ];

  if (loading) {
    return (
      <div className="page-container flex items-center justify-center min-h-[50vh]">
        <div className="text-slate-500">Loading DSA information...</div>
      </div>
    );
  }

  return (
    <div className="page-container" data-testid="dsa-info-page">
      {/* Header */}
      <div className="mb-8">
        <h1 className="text-2xl font-bold text-slate-900 flex items-center gap-2">
          <BookOpen className="w-6 h-6" />
          Data Structures Reference
        </h1>
        <p className="text-slate-500 mt-1">
          Academic documentation for DSA Lab - Part B evaluation
        </p>
      </div>

      {/* Overview Card */}
      <Card className="mb-8 bg-gradient-to-r from-primary/5 to-purple-500/5 border-primary/20">
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <Cpu className="w-5 h-5 text-primary" />
            Project Overview
          </CardTitle>
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-2 md:grid-cols-4 gap-4 text-center">
            <div className="p-4 bg-white rounded-lg shadow-sm">
              <div className="text-3xl font-bold text-primary">7</div>
              <div className="text-sm text-slate-500">Data Structures</div>
            </div>
            <div className="p-4 bg-white rounded-lg shadow-sm">
              <div className="text-3xl font-bold text-primary">SQLite</div>
              <div className="text-sm text-slate-500">Database</div>
            </div>
            <div className="p-4 bg-white rounded-lg shadow-sm">
              <div className="text-3xl font-bold text-primary">React</div>
              <div className="text-sm text-slate-500">Frontend UI</div>
            </div>
            <div className="p-4 bg-white rounded-lg shadow-sm">
              <div className="text-3xl font-bold text-primary">FastAPI</div>
              <div className="text-sm text-slate-500">Backend API</div>
            </div>
          </div>
        </CardContent>
      </Card>

      {/* Data Structures Table */}
      <Card className="mb-8" data-testid="ds-table">
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <Code2 className="w-5 h-5" />
            Data Structures Used
          </CardTitle>
          <CardDescription>
            Detailed breakdown of each data structure and its role
          </CardDescription>
        </CardHeader>
        <CardContent>
          <Table>
            <TableHeader>
              <TableRow>
                <TableHead>Data Structure</TableHead>
                <TableHead>Purpose</TableHead>
                <TableHead>Key Operations</TableHead>
                <TableHead>Used In</TableHead>
              </TableRow>
            </TableHeader>
            <TableBody>
              {dsaInfo?.dataStructures?.map((ds, idx) => (
                <TableRow key={idx}>
                  <TableCell className="font-semibold">
                    <Badge
                      variant="outline"
                      className={`
                        ${ds.name.includes("Red-Black") ? "border-red-200 bg-red-50 text-red-700" : ""}
                        ${ds.name.includes("Skip") ? "border-green-200 bg-green-50 text-green-700" : ""}
                        ${ds.name.includes("Priority") ? "border-purple-200 bg-purple-50 text-purple-700" : ""}
                        ${ds.name.includes("Hash") ? "border-blue-200 bg-blue-50 text-blue-700" : ""}
                        ${ds.name.includes("Sliding") ? "border-cyan-200 bg-cyan-50 text-cyan-700" : ""}
                        ${ds.name.includes("IntroSort") ? "border-orange-200 bg-orange-50 text-orange-700" : ""}
                        ${ds.name.includes("Z-Score") ? "border-pink-200 bg-pink-50 text-pink-700" : ""}
                      `}
                    >
                      {ds.name}
                    </Badge>
                  </TableCell>
                  <TableCell>{ds.purpose}</TableCell>
                  <TableCell className="font-mono text-xs">
                    {ds.operations.join(", ")}
                  </TableCell>
                  <TableCell>
                    <div className="flex flex-wrap gap-1">
                      {ds.usedIn.map((use, i) => (
                        <Badge key={i} variant="secondary" className="text-xs">
                          {use}
                        </Badge>
                      ))}
                    </div>
                  </TableCell>
                </TableRow>
              ))}
            </TableBody>
          </Table>
        </CardContent>
      </Card>

      {/* Time Complexity */}
      <Card className="mb-8" data-testid="complexity-table">
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <Cpu className="w-5 h-5" />
            Time Complexity Analysis
          </CardTitle>
        </CardHeader>
        <CardContent>
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            {dsaInfo?.complexity &&
              Object.entries(dsaInfo.complexity).map(([op, complexity]) => (
                <div
                  key={op}
                  className="flex items-center justify-between p-3 bg-slate-50 rounded-lg"
                >
                  <span className="font-medium text-slate-700">{op}</span>
                  <code className="px-2 py-1 bg-slate-200 rounded text-sm font-mono">
                    {complexity}
                  </code>
                </div>
              ))}
          </div>
        </CardContent>
      </Card>

      {/* Viva Questions */}
      <Card data-testid="viva-questions">
        <CardHeader>
          <CardTitle className="flex items-center gap-2">
            <HelpCircle className="w-5 h-5" />
            Viva Preparation (10 Questions)
          </CardTitle>
          <CardDescription>
            Common questions you might be asked during evaluation
          </CardDescription>
        </CardHeader>
        <CardContent>
          <Accordion type="single" collapsible className="w-full">
            {vivaQuestions.map((item, idx) => (
              <AccordionItem key={idx} value={`q-${idx}`}>
                <AccordionTrigger className="text-left">
                  <div className="flex items-start gap-2">
                    <Lightbulb className="w-5 h-5 text-yellow-500 shrink-0 mt-0.5" />
                    <span>Q{idx + 1}: {item.q}</span>
                  </div>
                </AccordionTrigger>
                <AccordionContent>
                  <div className="pl-7 pt-2">
                    <div className="flex items-start gap-2 text-slate-600">
                      <CheckCircle className="w-4 h-4 text-emerald-500 shrink-0 mt-1" />
                      <p>{item.a}</p>
                    </div>
                  </div>
                </AccordionContent>
              </AccordionItem>
            ))}
          </Accordion>
        </CardContent>
      </Card>

      {/* Quick Reference */}
      <div className="mt-8 p-6 bg-slate-100 rounded-lg">
        <h3 className="font-semibold mb-4 flex items-center gap-2">
          <BookOpen className="w-5 h-5" />
          Quick Reference - Data Structure Selection
        </h3>
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4 text-sm">
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-red-500 mb-2">Red-Black Tree</Badge>
            <p className="text-slate-600">Use for: Ordered data, O(log n) guaranteed operations</p>
          </div>
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-green-500 mb-2">Skip List</Badge>
            <p className="text-slate-600">Use for: Fast lookups with simpler implementation</p>
          </div>
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-purple-500 mb-2">Indexed Priority Queue</Badge>
            <p className="text-slate-600">Use for: Priority with efficient updates</p>
          </div>
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-blue-500 mb-2">Polynomial Hash Map</Badge>
            <p className="text-slate-600">Use for: O(1) lookups with good distribution</p>
          </div>
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-cyan-500 mb-2">Sliding Window</Badge>
            <p className="text-slate-600">Use for: Fixed-size aggregate calculations</p>
          </div>
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-orange-500 mb-2">IntroSort</Badge>
            <p className="text-slate-600">Use for: Guaranteed O(n log n) sorting</p>
          </div>
        </div>
      </div>
    </div>
  );
}
