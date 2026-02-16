import { useCallback, useEffect, useMemo, useState } from 'react';
import { invoke } from '@tauri-apps/api/core';
import ReactFlow, {
  addEdge,
  Background,
  Controls,
  Handle,
  Position,
  ReactFlowProvider,
  useEdgesState,
  useNodesState,
  useReactFlow,
} from 'reactflow';
import type { Connection, Edge, Node, NodeChange, EdgeChange, NodeProps, XYPosition } from 'reactflow';
import 'reactflow/dist/style.css';
import './App.css';
import { SearchPanel } from './SearchPanel';
import { PropertiesPanel } from './PropertiesPanel';

type GraphPin = { id: string; ty: string };
type GraphNode = {
  id: string;
  kind: string;
  inputs: GraphPin[];
  outputs: GraphPin[];
  params: Record<string, unknown>;
};
type GraphEdgeEndpoint = { node: string; pin: string };
type GraphEdgeDoc = { from: GraphEdgeEndpoint; to: GraphEdgeEndpoint };
type GraphDoc = { version: number; name: string; nodes: GraphNode[]; edges: GraphEdgeDoc[] };
type CompilePass = { node_id: string; node_kind: string; pass_name: string | null };
type CompileResult = {
  graph_name: string;
  execution_order: string[];
  tenengine_passes: CompilePass[];
};
type FlowNodeData = { kind: string; passName?: string; label: string };

type NodeTypeInfo = { kind: string; description: string };
type NodesByDomain = Record<string, Record<string, NodeTypeInfo[]>>;

const DOMAIN_IDS = ['framegraph', 'shadergraph', 'scriptgraph', 'aitaskgraph'] as const;
const DOMAIN_LABELS: Record<string, string> = {
  framegraph: 'Frame Graph',
  shadergraph: 'Shader Graph',
  scriptgraph: 'Script Graph',
  aitaskgraph: 'AI Task Graph',
};

const FALLBACK_EXAMPLE = `{
  "version": 1,
  "name": "minimal",
  "nodes": [
    { "id": "n1", "kind": "frame.begin_pass", "inputs": [], "outputs": [{ "id": "out", "ty": "color" }], "params": { "pass_name": "GBuffer" } },
    { "id": "n2", "kind": "frame.end_pass", "inputs": [{ "id": "in", "ty": "color" }], "outputs": [], "params": {} }
  ],
  "edges": [ { "from": { "node": "n1", "pin": "out" }, "to": { "node": "n2", "pin": "in" } } ]
}`;

function composeLabel(id: string, kind: string, passName?: string): string {
  if (passName?.length) return `${kind}\n(${id})\npass=${passName}`;
  return `${kind}\n(${id})`;
}

function defaultPinsByKind(kind: string): { inputs: GraphPin[]; outputs: GraphPin[] } {
  const flowIn = [{ id: 'in', ty: 'flow' as const }];
  const flowOut = [{ id: 'out', ty: 'flow' as const }];
  const colorOut = [{ id: 'out', ty: 'color' as const }];
  const colorIn = [{ id: 'in', ty: 'color' as const }];
  const vec3 = { id: 'v', ty: 'vec3' as const };
  const float = { id: 'f', ty: 'float' as const };

  if (kind.startsWith('frame.')) {
    if (kind === 'frame.begin_pass') return { inputs: [], outputs: colorOut };
    if (kind === 'frame.end_pass') return { inputs: colorIn, outputs: [] };
    if (kind === 'frame.read_resource' || kind === 'frame.write_resource')
      return { inputs: flowIn, outputs: flowOut };
    if (kind === 'frame.clear' || kind === 'frame.resolve' || kind === 'frame.copy_texture' || kind === 'frame.blit')
      return { inputs: flowIn, outputs: flowOut };
    if (kind === 'frame.create_texture2d' || kind === 'frame.create_texture_cube' || kind === 'frame.create_buffer')
      return { inputs: [], outputs: flowOut };
  }

  if (kind.startsWith('shader.')) {
    if (kind === 'shader.input') return { inputs: [], outputs: [vec3] };
    if (kind === 'shader.output') return { inputs: [vec3, { id: 'metallic', ty: 'float' }, { id: 'roughness', ty: 'float' }], outputs: [] };
    if (kind === 'shader.sample_texture2d') return { inputs: [{ id: 'uv', ty: 'vec2' }], outputs: [vec3] };
    if (kind === 'shader.sample_texture_cube') return { inputs: [{ id: 'dir', ty: 'vec3' }], outputs: [vec3] };
    if (kind === 'shader.multiply' || kind === 'shader.add' || kind === 'shader.subtract' || kind === 'shader.divide')
      return { inputs: [{ id: 'a', ty: 'vec3' }, { id: 'b', ty: 'vec3' }], outputs: [vec3] };
    if (kind === 'shader.lerp') return { inputs: [{ id: 'a', ty: 'vec3' }, { id: 'b', ty: 'vec3' }, float], outputs: [vec3] };
    if (kind === 'shader.constant') return { inputs: [], outputs: [float] };
    if (kind === 'shader.constant2') return { inputs: [], outputs: [{ id: 'v', ty: 'vec2' }] };
    if (kind === 'shader.constant3') return { inputs: [], outputs: [vec3] };
    if (kind === 'shader.constant4') return { inputs: [], outputs: [{ id: 'v', ty: 'vec4' }] };
    if (kind === 'shader.tex_coord') return { inputs: [], outputs: [{ id: 'uv', ty: 'vec2' }] };
    if (kind === 'shader.normalize' || kind === 'shader.abs' || kind === 'shader.saturate' || kind === 'shader.sqrt' || kind === 'shader.one_minus')
      return { inputs: [vec3], outputs: [vec3] };
    if (kind === 'shader.dot') return { inputs: [vec3, vec3], outputs: [float] };
    if (kind === 'shader.cross') return { inputs: [vec3, vec3], outputs: [vec3] };
    if (kind === 'shader.clamp' || kind === 'shader.pow') return { inputs: [vec3, vec3, vec3], outputs: [vec3] };
    if (kind === 'shader.if') return { inputs: [{ id: 'cond', ty: 'bool' }, vec3, vec3], outputs: [vec3] };
    if (kind === 'shader.fresnel') return { inputs: [vec3, vec3], outputs: [float] };
    return { inputs: [vec3], outputs: [vec3] };
  }

  if (kind.startsWith('script.')) {
    if (kind === 'script.event_begin_play') return { inputs: [], outputs: flowOut };
    if (kind === 'script.event_tick') return { inputs: [], outputs: [flowOut[0], { id: 'delta', ty: 'float' }] };
    if (kind === 'script.event_input_action' || kind === 'script.event_custom') return { inputs: [], outputs: flowOut };
    if (kind === 'script.branch') return { inputs: [flowIn[0], { id: 'condition', ty: 'bool' }], outputs: [{ id: 'true', ty: 'flow' }, { id: 'false', ty: 'flow' }] };
    if (kind === 'script.sequence') return { inputs: flowIn, outputs: [{ id: 'then0', ty: 'flow' }, { id: 'then1', ty: 'flow' }] };
    if (kind === 'script.for_loop') return { inputs: flowIn, outputs: [{ id: 'loop', ty: 'flow' }, { id: 'completed', ty: 'flow' }] };
    if (kind === 'script.for_each_loop' || kind === 'script.while_loop') return { inputs: flowIn, outputs: flowOut };
    if (kind === 'script.return' || kind === 'script.print') return { inputs: flowIn, outputs: [] };
    if (kind === 'script.delay' || kind === 'script.do_once' || kind === 'script.gate') return { inputs: flowIn, outputs: flowOut };
    if (kind === 'script.call_function') return { inputs: flowIn, outputs: flowOut };
    if (kind === 'script.get_variable') return { inputs: [], outputs: [vec3] };
    if (kind === 'script.set_variable') return { inputs: flowIn.concat([vec3]), outputs: flowOut };
    if (kind === 'script.equal' || kind === 'script.not_equal' || kind === 'script.greater' || kind === 'script.less')
      return { inputs: [vec3, vec3], outputs: [{ id: 'result', ty: 'bool' }] };
    if (kind === 'script.and' || kind === 'script.or') return { inputs: [{ id: 'a', ty: 'bool' }, { id: 'b', ty: 'bool' }], outputs: [{ id: 'result', ty: 'bool' }] };
    if (kind === 'script.not') return { inputs: [{ id: 'a', ty: 'bool' }], outputs: [{ id: 'result', ty: 'bool' }] };
    return { inputs: flowIn, outputs: flowOut };
  }

  if (kind.startsWith('ai.')) {
    if (kind === 'ai.selector' || kind === 'ai.sequence') return { inputs: flowIn, outputs: flowOut };
    if (kind === 'ai.task_move_to' || kind === 'ai.task_wait') return { inputs: flowIn, outputs: flowOut };
  }

  return { inputs: flowIn, outputs: flowOut };
}

function flowFromGraphDoc(doc: GraphDoc): { nodes: Node<FlowNodeData>[]; edges: Edge[] } {
  const nodes: Node<FlowNodeData>[] = doc.nodes.map((n, idx) => {
    const passName = typeof n.params?.pass_name === 'string' ? String(n.params.pass_name) : undefined;
    return {
      id: n.id,
      type: CUSTOM_NODE_TYPE,
      position: { x: 120 + (idx % 4) * 260, y: 80 + Math.floor(idx / 4) * 160 },
      data: { kind: n.kind, passName, label: composeLabel(n.id, n.kind, passName) },
    };
  });
  const edges: Edge[] = doc.edges.map((e, idx) => ({
    id: `e${idx}`,
    source: e.from.node,
    target: e.to.node,
    sourceHandle: e.from.pin,
    targetHandle: e.to.pin,
    label: `${e.from.pin} -> ${e.to.pin}`,
    animated: true,
    style: { stroke: '#4c6ef5' },
    labelStyle: { fill: '#ced4da', fontSize: 11 },
  }));
  return { nodes, edges };
}

function graphDocFromCanvas(name: string, nodes: Node<FlowNodeData>[], edges: Edge[]): GraphDoc {
  const graphNodes: GraphNode[] = nodes.map((n) => {
    const kind = n.data?.kind ?? 'frame.begin_pass';
    const pins = defaultPinsByKind(kind);
    const params: Record<string, unknown> = {};
    if (n.data?.passName) params.pass_name = n.data.passName;
    return { id: n.id, kind, inputs: pins.inputs, outputs: pins.outputs, params };
  });
  const graphEdges: GraphEdgeDoc[] = edges
    .filter((e) => e.source && e.target)
    .map((e) => ({
      from: { node: e.source!, pin: (e.sourceHandle as string) ?? 'out' },
      to: { node: e.target!, pin: (e.targetHandle as string) ?? 'in' },
    }));
  return { version: 1, name: name.trim() || 'untitled', nodes: graphNodes, edges: graphEdges };
}

const nodeStyle = {
  whiteSpace: 'pre-wrap' as const,
  width: 220,
  minHeight: 60,
  borderRadius: 8,
  border: '1px solid #4c6ef5',
  background: '#161b22',
  color: '#f1f3f5',
  fontSize: 12,
  padding: 8,
};

const CUSTOM_NODE_TYPE = 'graphNode';

function GraphNodeComponent({ data, id }: NodeProps<FlowNodeData>) {
  const kind = data?.kind ?? 'frame.begin_pass';
  const { inputs, outputs } = defaultPinsByKind(kind);
  const totalHandles = Math.max(inputs.length, outputs.length, 1);
  const baseHeight = Math.max(60, totalHandles * 28);

  return (
    <div className="graph-node" style={{ ...nodeStyle, minHeight: baseHeight }}>
      {inputs.map((pin, i) => (
        <Handle
          key={pin.id}
          type="target"
          id={pin.id}
          position={Position.Left}
          style={{ top: `${((i + 1) * 100) / (inputs.length + 1)}%`, transform: 'translateY(-50%)' }}
          className="node-handle"
        />
      ))}
      <div className="graph-node-content">
        <div className="graph-node-label">
          {(data?.kind ?? '').split('.').pop() || id}
          {data?.passName ? ` (${data.passName})` : ''}
        </div>
      </div>
      {outputs.map((pin, i) => (
        <Handle
          key={pin.id}
          type="source"
          id={pin.id}
          position={Position.Right}
          style={{ top: `${((i + 1) * 100) / (outputs.length + 1)}%`, transform: 'translateY(-50%)' }}
          className="node-handle"
        />
      ))}
    </div>
  );
}

const nodeTypes = { [CUSTOM_NODE_TYPE]: GraphNodeComponent };

function FlowEditorInner({
  nodes,
  setNodes,
  edges,
  setEdges,
  onNodesChange,
  onEdgesChange,
  selectedDomain,
  nodesByDomain,
  setMessage,
}: {
  nodes: Node<FlowNodeData>[];
  setNodes: (u: React.SetStateAction<Node<FlowNodeData>[]>) => void;
  edges: Edge[];
  setEdges: (u: React.SetStateAction<Edge[]>) => void;
  onNodesChange: (changes: NodeChange[]) => void;
  onEdgesChange: (changes: EdgeChange[]) => void;
  selectedDomain: string;
  nodesByDomain: NodesByDomain;
  setMessage: (m: string) => void;
}) {
  const { screenToFlowPosition } = useReactFlow();
  const [contextMenu, setContextMenu] = useState<{ x: number; y: number; flowPosition: XYPosition } | null>(null);
  const [nodeContextMenu, setNodeContextMenu] = useState<{ x: number; y: number; nodeId: string } | null>(null);

  const onConnect = useCallback(
    (c: Connection) => {
      setEdges((eds) =>
        addEdge(
          {
            ...c,
            id: `e${crypto.randomUUID()}`,
            animated: true,
            label: `${c.sourceHandle ?? 'out'} -> ${c.targetHandle ?? 'in'}`,
            style: { stroke: '#4c6ef5' },
            labelStyle: { fill: '#ced4da', fontSize: 11 },
          },
          eds
        )
      );
      setMessage('Connected nodes');
    },
    [setEdges, setMessage]
  );

  const onPaneContextMenu = useCallback(
    (e: React.MouseEvent) => {
      e.preventDefault();
      setNodeContextMenu(null);
      const flowPosition = screenToFlowPosition({ x: e.clientX, y: e.clientY });
      setContextMenu({ x: e.clientX, y: e.clientY, flowPosition });
    },
    [screenToFlowPosition]
  );

  const onNodeContextMenu = useCallback((e: React.MouseEvent, node: Node<FlowNodeData>) => {
    e.preventDefault();
    setContextMenu(null);
    setNodeContextMenu({ x: e.clientX, y: e.clientY, nodeId: node.id });
  }, []);

  const duplicateNode = useCallback(
    (nodeId: string) => {
      const node = nodes.find((n) => n.id === nodeId);
      if (!node) return;
      const newId = `n${Date.now()}_${Math.floor(Math.random() * 1000)}`;
      const newNode: Node<FlowNodeData> = {
        ...node,
        id: newId,
        position: { x: node.position.x + 40, y: node.position.y + 40 },
        data: { ...node.data, label: composeLabel(newId, node.data?.kind ?? '', node.data?.passName) },
      };
      setNodes((prev) => [...prev, newNode]);
      setMessage('已复制节点');
      setNodeContextMenu(null);
    },
    [nodes, setNodes, setMessage]
  );

  const deleteNode = useCallback(
    (nodeId: string) => {
      setNodes((prev) => prev.filter((n) => n.id !== nodeId));
      setEdges((prev) => prev.filter((e) => e.source !== nodeId && e.target !== nodeId));
      setMessage('已删除节点');
      setNodeContextMenu(null);
    },
    [setNodes, setEdges, setMessage]
  );

  const addNodeAt = useCallback(
    (position: XYPosition, kind: string) => {
      const id = `n${Date.now()}_${Math.floor(Math.random() * 1000)}`;
      const passName = kind === 'frame.begin_pass' ? 'NewPass' : undefined;
      setNodes((prev) => [
        ...prev,
        {
          id,
          type: CUSTOM_NODE_TYPE,
          position,
          data: { kind, passName, label: composeLabel(id, kind, passName) },
        },
      ]);
      setMessage(`Added ${kind}`);
      setContextMenu(null);
    },
    [setNodes, setMessage]
  );

  const catalog = nodesByDomain[selectedDomain];
  const categories = catalog ? Object.keys(catalog).sort() : [];

  return (
    <>
      <ReactFlow
        nodes={nodes}
        edges={edges}
        nodeTypes={nodeTypes}
        onNodesChange={onNodesChange}
        onEdgesChange={onEdgesChange}
        onConnect={onConnect}
        onPaneContextMenu={onPaneContextMenu}
        onNodeContextMenu={onNodeContextMenu}
        onPaneClick={() => { setContextMenu(null); setNodeContextMenu(null); setSelectedNodeId(null); }}
        onNodeClick={(e, node) => { setSelectedNodeId(node.id); }}
        deleteKeyCode={['Backspace', 'Delete']}
        fitView
      >
        <Background />
        <Controls />
      </ReactFlow>

      {contextMenu && catalog && (
        <div
          className="context-menu"
          style={{ left: contextMenu.x, top: contextMenu.y }}
          role="menu"
        >
          <div className="context-menu-title">Add Node</div>
          {categories.map((cat) => (
            <div key={cat} className="context-menu-category">
              <div className="context-menu-category-name">{cat}</div>
              {catalog[cat].map((nt) => (
                <button
                  key={nt.kind}
                  type="button"
                  className="context-menu-item"
                  onClick={() => addNodeAt(contextMenu.flowPosition, nt.kind)}
                >
                  {nt.kind.split('.').pop()}
                </button>
              ))}
            </div>
          ))}
        </div>
      )}

      {nodeContextMenu && (
        <div
          className="context-menu"
          style={{ left: nodeContextMenu.x, top: nodeContextMenu.y }}
          role="menu"
        >
          <div className="context-menu-title">节点</div>
          <button
            type="button"
            className="context-menu-item"
            onClick={() => duplicateNode(nodeContextMenu.nodeId)}
          >
            复制
          </button>
          <button
            type="button"
            className="context-menu-item"
            onClick={() => deleteNode(nodeContextMenu.nodeId)}
          >
            删除
          </button>
        </div>
      )}
    </>
  );
}

function App() {
  const [graphText, setGraphText] = useState(FALLBACK_EXAMPLE);
  const [compileResult, setCompileResult] = useState<CompileResult | null>(null);
  const [message, setMessage] = useState('Ready');
  const [nodes, setNodes, onNodesChange] = useNodesState<FlowNodeData>([]);
  const [edges, setEdges, onEdgesChange] = useEdgesState<Edge>([]);
  const [filePath, setFilePath] = useState('tools/te-graph/examples/minimal.graph.json');
  const [selectedDomain, setSelectedDomain] = useState<string>('framegraph');
  const [nodesByDomain, setNodesByDomain] = useState<NodesByDomain>({});
  const [selectedNodeId, setSelectedNodeId] = useState<string | null>(null);

  const selectedNode = useMemo(() => {
    return selectedNodeId ? nodes.find((n) => n.id === selectedNodeId) || null : null;
  }, [nodes, selectedNodeId]);

  const handleUpdateNode = useCallback((nodeId: string, data: Record<string, unknown>) => {
    setNodes((nds) =>
      nds.map((n) =>
        n.id === nodeId ? { ...n, data: { ...n.data, ...data } } : n
      )
    );
  }, [setNodes]);

  const handleAddNodeFromSearch = useCallback((kind: string) => {
    const id = `n${Date.now()}_${Math.floor(Math.random() * 1000)}`;
    const passName = kind === 'frame.begin_pass' ? 'NewPass' : undefined;
    setNodes((prev) => [
      ...prev,
      {
        id,
        type: CUSTOM_NODE_TYPE,
        position: { x: 200 + Math.random() * 400, y: 150 + Math.random() * 300 },
        data: { kind, passName, label: composeLabel(id, kind, passName) },
      },
    ]);
    setMessage(`Added ${kind} from search`);
  }, [setNodes, setMessage]);

  useEffect(() => {
    void (async () => {
      try {
        const data = await invoke<NodesByDomain>('list_nodes_by_domain_and_category');
        setNodesByDomain(data);
        if (!Object.keys(data).includes(selectedDomain)) {
          const first = Object.keys(data)[0];
          if (first) setSelectedDomain(first);
        }
      } catch {
        setNodesByDomain({});
      }
    })();
  }, [selectedDomain]);

  const syncGraphView = useCallback((text: string): boolean => {
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
  }, [setNodes, setEdges]);

  const syncGraphTextFromCanvas = useCallback(() => {
    const doc = graphDocFromCanvas('from_canvas', nodes, edges);
    setGraphText(JSON.stringify(doc, null, 2));
    setMessage('Canvas exported to JSON');
  }, [nodes, edges]);

  const loadExample = async () => {
    try {
      const text = await invoke<string>('load_example_graph');
      setGraphText(text);
      if (syncGraphView(text)) setMessage('Loaded example graph');
    } catch {
      setGraphText(FALLBACK_EXAMPLE);
      syncGraphView(FALLBACK_EXAMPLE);
      setMessage('Loaded fallback example');
    }
  };

  const loadFromPath = async () => {
    try {
      const text = await invoke<string>('load_graph_from_path', { path: filePath });
      setGraphText(text);
      if (syncGraphView(text)) setMessage(`Loaded: ${filePath}`);
    } catch (e) {
      setMessage(`Load failed: ${String(e)}`);
    }
  };

  const saveToPath = async () => {
    try {
      await invoke('save_graph_to_path', { path: filePath, graphText });
      setMessage(`Saved: ${filePath}`);
    } catch (e) {
      setMessage(`Save failed: ${String(e)}`);
    }
  };

  const parseAndRender = () => {
    if (syncGraphView(graphText)) setMessage('Graph rendered');
  };

  const compileGraph = async () => {
    try {
      const doc = graphDocFromCanvas('compile_target', nodes, edges);
      const text = JSON.stringify(doc, null, 2);
      setGraphText(text);
      const result = await invoke<CompileResult>('compile_graph_text', { graphText: text });
      setCompileResult(result);
      setMessage(`Compiled: ${result.execution_order.length} nodes`);
      syncGraphView(text);
    } catch (e) {
      setCompileResult(null);
      setMessage(`Compile failed: ${String(e)}`);
    }
  };

  const knownNodeCount = useMemo(() => nodes.length, [nodes.length]);

  return (
    <div className="app-shell">
      <aside className="left-panel">
        <h2>TE Graph Editor</h2>

        <div className="domain-row">
          <label>Domain</label>
          <select
            value={selectedDomain}
            onChange={(e) => setSelectedDomain(e.target.value)}
          >
            {DOMAIN_IDS.map((id) => (
              <option key={id} value={id}>
                {DOMAIN_LABELS[id] ?? id}
              </option>
            ))}
          </select>
        </div>

        <div className="panel-section">
          <div className="panel-section-title">Search & Add</div>
          <SearchPanel
            nodesByDomain={nodesByDomain}
            selectedDomain={selectedDomain}
            onAddNode={handleAddNodeFromSearch}
          />
        </div>

        <div className="panel-section">
          <PropertiesPanel
            selectedNode={selectedNode}
            onUpdateNode={handleUpdateNode}
          />
        </div>

        <p className="hint">Right-click on canvas to add nodes (filtered by domain).</p>

        <div className="button-row">
          <button type="button" onClick={loadExample}>Load Example</button>
          <button type="button" onClick={parseAndRender}>JSON → Canvas</button>
          <button type="button" onClick={syncGraphTextFromCanvas}>Canvas → JSON</button>
          <button type="button" onClick={compileGraph}>Compile (Rust)</button>
        </div>

        <div className="path-row">
          <input value={filePath} onChange={(e) => setFilePath(e.target.value)} placeholder="graph file path" />
          <button type="button" onClick={loadFromPath}>Load</button>
          <button type="button" onClick={saveToPath}>Save</button>
        </div>

        <textarea className="graph-input" value={graphText} onChange={(e) => setGraphText(e.target.value)} spellCheck={false} />

        <div className="status-box">
          <div><strong>Status:</strong> {message}</div>
          <div><strong>Nodes:</strong> {knownNodeCount}</div>
        </div>

        <div className="result-box">
          <h3>Compile Result</h3>
          {compileResult ? (
            <>
              <div><strong>Order:</strong> {compileResult.execution_order.join(' → ')}</div>
              <ul>
                {compileResult.tenengine_passes.map((p) => (
                  <li key={`${p.node_id}-${p.node_kind}`}>{p.node_id} | {p.node_kind}{p.pass_name ? ` | ${p.pass_name}` : ''}</li>
                ))}
              </ul>
            </>
          ) : (
            <div>No result yet.</div>
          )}
        </div>
      </aside>

      <main className="right-panel">
        <ReactFlowProvider>
          <FlowEditorInner
            nodes={nodes}
            setNodes={setNodes}
            edges={edges}
            setEdges={setEdges}
            onNodesChange={onNodesChange}
            onEdgesChange={onEdgesChange}
            selectedDomain={selectedDomain}
            nodesByDomain={nodesByDomain}
            setMessage={setMessage}
          />
        </ReactFlowProvider>
      </main>
    </div>
  );
}

export default App;
