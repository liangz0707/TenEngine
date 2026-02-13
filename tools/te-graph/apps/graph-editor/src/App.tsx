import { useCallback, useEffect, useMemo, useState } from 'react';
import { invoke } from '@tauri-apps/api/core';
import ReactFlow, {
  addEdge,
  Background,
  Connection,
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

type FlowNodeData = {
  kind: string;
  passName?: string;
  label: string;
};

const FALLBACK_NODE_KINDS = [
  'frame.begin_pass',
  'frame.end_pass',
  'shader.input',
  'shader.multiply',
  'shader.output',
  'script.event_begin_play',
  'script.branch',
  'script.return',
  'ai.selector',
  'ai.sequence',
  'ai.task_move_to',
  'ai.task_wait',
];

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

function composeLabel(id: string, kind: string, passName?: string): string {
  if (passName && passName.length > 0) {
    return `${kind}\n(${id})\npass=${passName}`;
  }
  return `${kind}\n(${id})`;
}

function defaultPinsByKind(kind: string): { inputs: GraphPin[]; outputs: GraphPin[] } {
  if (kind === 'frame.begin_pass') {
    return { inputs: [], outputs: [{ id: 'out', ty: 'color' }] };
  }
  if (kind === 'frame.end_pass') {
    return { inputs: [{ id: 'in', ty: 'color' }], outputs: [] };
  }
  if (kind === 'script.event_begin_play') {
    return { inputs: [], outputs: [{ id: 'next', ty: 'flow' }] };
  }
  if (kind === 'script.return') {
    return { inputs: [{ id: 'in', ty: 'flow' }], outputs: [] };
  }
  return {
    inputs: [{ id: 'in', ty: 'flow' }],
    outputs: [{ id: 'out', ty: 'flow' }],
  };
}

function flowFromGraphDoc(doc: GraphDoc): { nodes: Node<FlowNodeData>[]; edges: Edge[] } {
  const nodes: Node<FlowNodeData>[] = doc.nodes.map((n, idx) => {
    const passName = typeof n.params?.pass_name === 'string' ? String(n.params.pass_name) : undefined;
    return {
    id: n.id,
    position: { x: 120 + (idx % 4) * 260, y: 80 + Math.floor(idx / 4) * 160 },
    data: {
      kind: n.kind,
      passName,
      label: composeLabel(n.id, n.kind, passName),
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
  }});

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

function graphDocFromCanvas(
  name: string,
  nodes: Node<FlowNodeData>[],
  edges: Edge[]
): GraphDoc {
  const graphNodes: GraphNode[] = nodes.map((n) => {
    const kind = n.data?.kind ?? 'frame.begin_pass';
    const pins = defaultPinsByKind(kind);
    const params: Record<string, unknown> = {};
    if (n.data?.passName) {
      params.pass_name = n.data.passName;
    }
    return {
      id: n.id,
      kind,
      inputs: pins.inputs,
      outputs: pins.outputs,
      params,
    };
  });

  const graphEdges: GraphEdgeDoc[] = edges
    .filter((e) => !!e.source && !!e.target)
    .map((e) => ({
      from: { node: e.source!, pin: e.sourceHandle ?? 'out' },
      to: { node: e.target!, pin: e.targetHandle ?? 'in' },
    }));

  return {
    version: 1,
    name: name.trim().length > 0 ? name : 'untitled',
    nodes: graphNodes,
    edges: graphEdges,
  };
}

function App() {
  const [graphText, setGraphText] = useState<string>(FALLBACK_EXAMPLE);
  const [compileResult, setCompileResult] = useState<CompileResult | null>(null);
  const [message, setMessage] = useState<string>('Ready');
  const [nodes, setNodes, onNodesChange] = useNodesState<Node<FlowNodeData>>([]);
  const [edges, setEdges, onEdgesChange] = useEdgesState<Edge>([]);
  const [filePath, setFilePath] = useState<string>('tools/te-graph/examples/minimal.graph.json');
  const [newNodeKind, setNewNodeKind] = useState<string>('frame.begin_pass');
  const [supportedNodeKinds, setSupportedNodeKinds] = useState<string[]>(FALLBACK_NODE_KINDS);

  const knownNodeCount = useMemo(() => nodes.length, [nodes.length]);

  useEffect(() => {
    void (async () => {
      try {
        const kinds = await invoke<string[]>('list_supported_node_kinds');
        if (kinds.length > 0) {
          setSupportedNodeKinds(kinds);
          setNewNodeKind(kinds[0]);
        }
      } catch {
        setSupportedNodeKinds(FALLBACK_NODE_KINDS);
      }
    })();
  }, []);

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

  const syncGraphTextFromCanvas = () => {
    const doc = graphDocFromCanvas('from_canvas', nodes, edges);
    const text = JSON.stringify(doc, null, 2);
    setGraphText(text);
    setMessage('Canvas exported to JSON');
  };

  const onConnect = useCallback(
    (connection: Connection) => {
      setEdges((eds) =>
        addEdge(
          {
            ...connection,
            id: `e${crypto.randomUUID()}`,
            animated: true,
            label: `${connection.sourceHandle ?? 'out'} -> ${connection.targetHandle ?? 'in'}`,
            style: { stroke: '#4c6ef5' },
            labelStyle: { fill: '#ced4da', fontSize: 11 },
          },
          eds
        )
      );
      setMessage('Connected nodes on canvas');
    },
    [setEdges]
  );

  const addNodeToCanvas = () => {
    const id = `n${nodes.length + 1}_${Math.floor(Math.random() * 1000)}`;
    const passName = newNodeKind === 'frame.begin_pass' ? 'NewPass' : undefined;
    setNodes((prev) => [
      ...prev,
      {
        id,
        position: { x: 100 + (prev.length % 4) * 260, y: 100 + Math.floor(prev.length / 4) * 160 },
        data: {
          kind: newNodeKind,
          passName,
          label: composeLabel(id, newNodeKind, passName),
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
      },
    ]);
    setMessage(`Added node '${newNodeKind}'`);
  };

  const loadFromPath = async () => {
    try {
      const text = await invoke<string>('load_graph_from_path', { path: filePath });
      setGraphText(text);
      const ok = syncGraphView(text);
      if (ok) setMessage(`Loaded file: ${filePath}`);
    } catch (e) {
      setMessage(`Load file failed: ${String(e)}`);
    }
  };

  const saveToPath = async () => {
    try {
      await invoke('save_graph_to_path', { path: filePath, graphText });
      setMessage(`Saved file: ${filePath}`);
    } catch (e) {
      setMessage(`Save file failed: ${String(e)}`);
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
      const canvasDoc = graphDocFromCanvas('compile_target', nodes, edges);
      const text = JSON.stringify(canvasDoc, null, 2);
      setGraphText(text);
      const result = await invoke<CompileResult>('compile_graph_text', {
        graphText: text,
      });
      setCompileResult(result);
      setMessage(
        `Compiled: ${result.execution_order.length} nodes, ${result.tenengine_passes.length} frame passes`
      );
      syncGraphView(text);
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
          <button onClick={parseAndRender}>JSON -> Canvas</button>
          <button onClick={syncGraphTextFromCanvas}>Canvas -> JSON</button>
          <button onClick={compileGraph}>Compile (Rust)</button>
        </div>

        <div className="path-row">
          <input
            value={filePath}
            onChange={(e) => setFilePath(e.target.value)}
            placeholder="graph file path"
          />
          <button onClick={loadFromPath}>Load File</button>
          <button onClick={saveToPath}>Save File</button>
        </div>

        <div className="kind-row">
          <select value={newNodeKind} onChange={(e) => setNewNodeKind(e.target.value)}>
            {supportedNodeKinds.map((kind) => (
              <option key={kind} value={kind}>
                {kind}
              </option>
            ))}
          </select>
          <button onClick={addNodeToCanvas}>Add Node</button>
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
          onConnect={onConnect}
          deleteKeyCode={['Backspace', 'Delete']}
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
