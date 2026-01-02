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

const API = `${process.env.REACT_APP_BACKEND_URL}/api`;

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

  // Viva preparation questions
  const vivaQuestions = [
    {
      q: "Why did you choose a Hash Map for budget management?",
      a: "Hash Map provides O(1) average time for lookup, insert, and update operations. This is ideal for budget management where we need frequent category lookups. Unlike arrays or linked lists (O(n) search), HashMap allows instant access to any category's budget.",
    },
    {
      q: "Why use a Doubly Linked List instead of an array for transactions?",
      a: "Doubly Linked List allows O(1) insertion at both ends and efficient bidirectional traversal. We can easily add new transactions at the front (most recent) and traverse backwards for history. Unlike arrays, we don't need to shift elements during deletion.",
    },
    {
      q: "How does BST help with date range queries?",
      a: "BST maintains transactions sorted by date, enabling O(log n + k) range queries where k is the result count. For date ranges, we traverse from start date to end date using in-order traversal, skipping irrelevant subtrees - much faster than O(n) linear search.",
    },
    {
      q: "Why Max Heap for top expenses instead of sorting?",
      a: "Max Heap allows us to extract top K expenses in O(k log n) time. Full sorting would take O(n log n). For finding top 5-10 expenses from thousands of transactions, heap is significantly faster. BuildHeap is also O(n) which is optimal.",
    },
    {
      q: "Why Queue for bills instead of Stack?",
      a: "Bills follow FIFO order - bills added first should be paid first (based on due dates). Queue's FIFO behavior naturally represents this. Stack (LIFO) would pay newest bills first, which isn't practical for bill management.",
    },
    {
      q: "How does Stack enable undo functionality?",
      a: "Stack's LIFO property is perfect for undo - the last action is undone first. We push each action to stack, and pop to undo. This is the standard approach used in editors, browsers, and most software with undo functionality.",
    },
    {
      q: "What advantage does Trie provide for autocomplete?",
      a: "Trie enables O(m) prefix search where m is prefix length, regardless of total words. For autocomplete, this means instant suggestions as users type. Alternative approaches like filtering an array would be O(n*m) for n categories.",
    },
    {
      q: "What would you change if you had millions of transactions?",
      a: "For scalability: 1) Use self-balancing BST (AVL/Red-Black) for guaranteed O(log n) operations, 2) Implement database indexing, 3) Add pagination for large results, 4) Consider B-trees for disk-based storage, 5) Use persistent heap structures.",
    },
    {
      q: "How would you handle concurrent access?",
      a: "Add thread-safe mechanisms: 1) Mutex locks for write operations, 2) Read-write locks for better read performance, 3) Atomic operations where possible, 4) Transaction isolation for database operations.",
    },
    {
      q: "What's the space complexity of your implementation?",
      a: "O(n) for all structures where n is the data count. HashMap: O(n) for entries, DLL: O(n) for nodes, BST: O(n) for nodes, Heap: O(n) for array, Queue: O(n) for nodes, Stack: O(k) where k is undo limit, Trie: O(m*k) for m words of avg length k.",
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
              <div className="text-3xl font-bold text-primary">6+</div>
              <div className="text-sm text-slate-500">Data Structures</div>
            </div>
            <div className="p-4 bg-white rounded-lg shadow-sm">
              <div className="text-3xl font-bold text-primary">C++</div>
              <div className="text-sm text-slate-500">Backend Logic</div>
            </div>
            <div className="p-4 bg-white rounded-lg shadow-sm">
              <div className="text-3xl font-bold text-primary">React</div>
              <div className="text-sm text-slate-500">Frontend UI</div>
            </div>
            <div className="p-4 bg-white rounded-lg shadow-sm">
              <div className="text-3xl font-bold text-primary">JSON</div>
              <div className="text-sm text-slate-500">Data Exchange</div>
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
                        ${ds.name.includes("Hash") ? "border-blue-200 bg-blue-50 text-blue-700" : ""}
                        ${ds.name.includes("Linked") ? "border-green-200 bg-green-50 text-green-700" : ""}
                        ${ds.name.includes("BST") ? "border-purple-200 bg-purple-50 text-purple-700" : ""}
                        ${ds.name.includes("Heap") ? "border-orange-200 bg-orange-50 text-orange-700" : ""}
                        ${ds.name.includes("Queue") ? "border-cyan-200 bg-cyan-50 text-cyan-700" : ""}
                        ${ds.name.includes("Stack") ? "border-pink-200 bg-pink-50 text-pink-700" : ""}
                        ${ds.name.includes("Trie") ? "border-teal-200 bg-teal-50 text-teal-700" : ""}
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
            <Badge className="bg-blue-500 mb-2">Hash Map</Badge>
            <p className="text-slate-600">Use for: Key-value mapping, O(1) lookups</p>
          </div>
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-green-500 mb-2">Linked List</Badge>
            <p className="text-slate-600">Use for: Sequential data, frequent inserts/deletes</p>
          </div>
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-purple-500 mb-2">BST</Badge>
            <p className="text-slate-600">Use for: Sorted data, range queries</p>
          </div>
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-orange-500 mb-2">Heap</Badge>
            <p className="text-slate-600">Use for: Priority queues, top-K elements</p>
          </div>
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-cyan-500 mb-2">Queue</Badge>
            <p className="text-slate-600">Use for: FIFO operations, scheduling</p>
          </div>
          <div className="p-3 bg-white rounded shadow-sm">
            <Badge className="bg-pink-500 mb-2">Stack</Badge>
            <p className="text-slate-600">Use for: LIFO operations, undo/redo</p>
          </div>
        </div>
      </div>
    </div>
  );
}
