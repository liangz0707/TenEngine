import { useState, useMemo, useCallback, useEffect } from 'react';

type NodeTypeInfo = { kind: string; description: string };
type NodesByDomain = Record<string, Record<string, NodeTypeInfo[]>>;

interface SearchPanelProps {
  nodesByDomain: NodesByDomain;
  selectedDomain: string;
  onAddNode: (kind: string) => void;
}

export function SearchPanel({ nodesByDomain, selectedDomain, onAddNode }: SearchPanelProps) {
  const [searchText, setSearchText] = useState('');
  const [showResults, setShowResults] = useState(false);

  const allNodes = useMemo(() => {
    const nodes: Array<{ kind: string; description: string; domain: string }> = [];
    const domain = nodesByDomain[selectedDomain];
    if (domain) {
      Object.entries(domain).forEach(([category, items]) => {
        items.forEach((item) => {
          nodes.push({
            kind: item.kind,
            description: item.description,
            domain: category,
          });
        });
      });
    }
    return nodes;
  }, [nodesByDomain, selectedDomain]);

  const filteredNodes = useMemo(() => {
    if (!searchText.trim()) return [];
    const lower = searchText.toLowerCase();
    return allNodes.filter(
      (n) =>
        n.kind.toLowerCase().includes(lower) ||
        n.description.toLowerCase().includes(lower)
    ).slice(0, 20);
  }, [allNodes, searchText]);

  const handleSelect = useCallback((kind: string) => {
    onAddNode(kind);
    setSearchText('');
    setShowResults(false);
  }, [onAddNode]);

  useEffect(() => {
    setShowResults(searchText.length > 0);
  }, [searchText]);

  return (
    <div className="search-container">
      <input
        className="search-input"
        type="text"
        placeholder="Search nodes..."
        value={searchText}
        onChange={(e) => setSearchText(e.target.value)}
        onFocus={() => searchText && setShowResults(true)}
        onBlur={() => setTimeout(() => setShowResults(false), 200)}
      />
      {showResults && filteredNodes.length > 0 && (
        <div className="search-results">
          {filteredNodes.map((node) => (
            <div
              key={node.kind}
              className="search-result-item"
              onClick={() => handleSelect(node.kind)}
            >
              <span className="kind">{node.kind}</span>
              <span className="description">{node.description}</span>
            </div>
          ))}
        </div>
      )}
    </div>
  );
}
