import { useMemo, useState } from 'react';
import { invoke } from '@tauri-apps/api/core';
import ReactFlow, {
  Background,
  Controls,
  useEdgesState,
  useNodesState,
} from 'reactflow';
import type { Edge, Node } from 'reactflow';
import 'reactflow/dist/style.css';
import './App.css';

type GraphPin = {
  id: string;
  ty: string;
};

type GraphNode = {
  id: string;
  kind: string;
  inputs: GraphPin[];
  outputs: GraphPin[];
  params: Record<string, unknown>;
};

type GraphEdgeEndpoint = {
  node: string;
  pin: string;
};

type GraphEdgeDoc = {
  from: GraphEdgeEndpoint;
  to: GraphEdgeEndpoint;
};

type GraphDoc = {
  version: number;
  name: string;
  nodes: GraphNode[];
  edges: GraphEdgeDoc[];
};

type CompilePass = {
  node_id: string;
  node_kind: string;
  pass_name: string | null;
};

type CompileResult = {
  graph_name: string;
  execution_order: string[];
  tenengine_passes: CompilePass[];
};

const FALLBACK_EXAMPLE = `{
  "version": 1,
  "name": "minimal",
  "nodes": [
    {
      "id": "n1",
      "kind": "frame.begin_pass",
      "inputs": [],
      "outputs": [{ "id": "out", "ty": "color" }],
      "params": { "pass_name": "GBuffer" }
    },
    {
      "id": "n2",
      "kind": "frame.end_pass",
      "inputs": [{ "id": "in", "ty": "color" }],
      "outputs": [],
      "params": {}
    }
  ],
  "edges": [
    {
      "from": { "node": "n1", "pin": "out" },
      "to": { "node": "n2", "pin": "in" }
    }
  ]
}`;

function flowFromGraphDoc(doc: GraphDoc): { nodes: Node[]; edges: Edge[] } {
  const nodes: Node[] = doc.nodes.map((n, idx) => ({
    id: n.id,
    position: { x: 120 + (idx % 4) * 260, y: 80 + Math.floor(idx / 4) * 160 },
    data: {
      label: `${n.kind}\n(${n.id})`,
    },
    style: {
      whiteSpace: 'pre-wrap',
      width: 220,
      borderRadius: 8,
      border: '1px solid #4c6ef5',
      background: '#161b22',
      color: '#f1f3f5',
      fontSize: 12,
      padding: 8,
    },
  }));

  const edges: Edge[] = doc.edges.map((e, idx) => ({
    id: `e${idx}`,
    source: e.from.node,
    target: e.to.node,
    label: `${e.from.pin} -> ${e.to.pin}`,
    animated: true,
    style: { stroke: '#4c6ef5' },
    labelStyle: { fill: '#ced4da', fontSize: 11 },
  }));

  return { nodes, edges };
}

function App() {
  const [graphText, setGraphText] = useState<string>(FALLBACK_EXAMPLE);
  const [compileResult, setCompileResult] = useState<CompileResult | null>(null);
  const [message, setMessage] = useState<string>('Ready');
  const [nodes, setNodes, onNodesChange] = useNodesState<Node>([]);
  const [edges, setEdges, onEdgesChange] = useEdgesState<Edge>([]);

  const knownNodeCount = useMemo(() => nodes.length, [nodes.length]);

  const syncGraphView = (text: string): boolean => {
    try {
      const doc = JSON.parse(text) as GraphDoc;
      const flow = flowFromGraphDoc(doc);
      setNodes(flow.nodes);
      setEdges(flow.edges);
      return true;
    } catch (e) {
      setMessage(`JSON parse failed: ${(e as Error).message}`);
      return false;
    }
  };

  const loadExample = async () => {
    try {
      const text = await invoke<string>('load_example_graph');
      setGraphText(text);
      const ok = syncGraphView(text);
      if (ok) setMessage('Loaded example graph');
    } catch {
      setGraphText(FALLBACK_EXAMPLE);
      const ok = syncGraphView(FALLBACK_EXAMPLE);
      if (ok) setMessage('Loaded fallback example graph');
    }
  };

  const parseAndRender = () => {
    if (syncGraphView(graphText)) {
      setMessage('Graph rendered');
    }
  };

  const compileGraph = async () => {
    try {
      const result = await invoke<CompileResult>('compile_graph_text', {
        graphText,
      });
      setCompileResult(result);
      setMessage(
        `Compiled: ${result.execution_order.length} nodes, ${result.tenengine_passes.length} frame passes`
      );
      syncGraphView(graphText);
    } catch (e) {
      setCompileResult(null);
      setMessage(`Compile failed: ${String(e)}`);
    }
  };

  return (
    <div className="app-shell">
      <aside className="left-panel">
        <h2>TE Graph Editor (Tauri + React Flow)</h2>
        <div className="button-row">
          <button onClick={loadExample}>Load Example</button>
          <button onClick={parseAndRender}>Render</button>
          <button onClick={compileGraph}>Compile (Rust)</button>
        </div>

        <textarea
          className="graph-input"
          value={graphText}
          onChange={(e) => setGraphText(e.target.value)}
          spellCheck={false}
        />

        <div className="status-box">
          <div><strong>Status:</strong> {message}</div>
          <div><strong>Rendered nodes:</strong> {knownNodeCount}</div>
        </div>

        <div className="result-box">
          <h3>Compile Result</h3>
          {compileResult ? (
            <>
              <div><strong>Graph:</strong> {compileResult.graph_name}</div>
              <div>
                <strong>Execution order:</strong>{' '}
                {compileResult.execution_order.join(' -> ')}
              </div>
              <div><strong>TenEngine frame passes:</strong></div>
              <ul>
                {compileResult.tenengine_passes.map((p) => (
                  <li key={`${p.node_id}-${p.node_kind}`}>
                    {p.node_id} | {p.node_kind}
                    {p.pass_name ? ` | pass_name=${p.pass_name}` : ''}
                  </li>
                ))}
              </ul>
            </>
          ) : (
            <div>No result yet.</div>
          )}
        </div>
      </aside>

      <main className="right-panel">
        <ReactFlow
          nodes={nodes}
          edges={edges}
          onNodesChange={onNodesChange}
          onEdgesChange={onEdgesChange}
          fitView
        >
          <Background />
          <Controls />
        </ReactFlow>
      </main>
    </div>
  );
}

export default App;
