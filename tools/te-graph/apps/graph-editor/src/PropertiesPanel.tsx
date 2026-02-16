import { useState, useEffect, useCallback } from 'react';
import type { Node } from 'reactflow';

interface PropertiesPanelProps {
  selectedNode: Node | null;
  onUpdateNode: (nodeId: string, data: Record<string, unknown>) => void;
}

export function PropertiesPanel({ selectedNode, onUpdateNode }: PropertiesPanelProps) {
  const [passName, setPassName] = useState('');
  const [customParams, setCustomParams] = useState<Record<string, string>>({});

  useEffect(() => {
    if (selectedNode) {
      setPassName((selectedNode.data?.passName as string) || '');
      setCustomParams({});
    } else {
      setPassName('');
      setCustomParams({});
    }
  }, [selectedNode?.id]);

  const handlePassNameChange = useCallback((value: string) => {
    setPassName(value);
    if (selectedNode) {
      onUpdateNode(selectedNode.id, {
        ...selectedNode.data,
        passName: value,
      });
    }
  }, [selectedNode, onUpdateNode]);

  if (!selectedNode) {
    return (
      <div className="properties-panel">
        <h4>Properties</h4>
        <div className="no-selection">
          Select a node to edit its properties
        </div>
      </div>
    );
  }

  const kind = selectedNode.data?.kind || '';
  const nodeId = selectedNode.id;

  return (
    <div className="properties-panel">
      <h4>Node Properties</h4>

      <div className="property-row">
        <label>Node ID</label>
        <input type="text" value={nodeId} disabled />
      </div>

      <div className="property-row">
        <label>Type</label>
        <input type="text" value={kind} disabled />
      </div>

      {(kind.startsWith('frame.') || kind.startsWith('shader.')) && (
        <div className="property-row">
          <label>Pass Name</label>
          <input
            type="text"
            value={passName}
            onChange={(e) => handlePassNameChange(e.target.value)}
            placeholder="e.g., GBuffer, Lighting"
          />
        </div>
      )}

      {kind.startsWith('shader.') && (
        <>
          <div className="property-row">
            <label>Float Value</label>
            <input
              type="number"
              step="0.01"
              placeholder="0.0"
              value={customParams['float'] || ''}
              onChange={(e) => {
                setCustomParams({ ...customParams, float: e.target.value });
              }}
            />
          </div>
          <div className="property-row">
            <label>Vec3 (x, y, z)</label>
            <input
              type="text"
              placeholder="1.0, 0.5, 0.0"
              value={customParams['vec3'] || ''}
              onChange={(e) => {
                setCustomParams({ ...customParams, vec3: e.target.value });
              }}
            />
          </div>
        </>
      )}

      {kind.startsWith('script.') && (
        <div className="property-row">
          <label>Variable Name</label>
          <input
            type="text"
            placeholder="variable_name"
            value={customParams['varName'] || ''}
            onChange={(e) => {
              setCustomParams({ ...customParams, varName: e.target.value });
            }}
          />
        </div>
      )}
    </div>
  );
}
