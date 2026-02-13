use graph_spec::GraphDoc;
use std::collections::{HashMap, VecDeque};

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct CompilePlan {
    pub order: Vec<String>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum CompileError {
    MissingNode { edge_index: usize, node_id: String },
    CycleDetected,
}

impl std::fmt::Display for CompileError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            CompileError::MissingNode {
                edge_index,
                node_id,
            } => {
                write!(f, "edge #{edge_index} references unknown node '{node_id}'")
            }
            CompileError::CycleDetected => write!(f, "cycle detected in graph"),
        }
    }
}

impl std::error::Error for CompileError {}

pub fn compile_graph(doc: &GraphDoc) -> Result<CompilePlan, CompileError> {
    let mut indegree: HashMap<&str, usize> = HashMap::new();
    let mut outgoing: HashMap<&str, Vec<&str>> = HashMap::new();
    let mut node_order: Vec<&str> = Vec::new();

    for node in &doc.nodes {
        indegree.insert(node.id.as_str(), 0);
        outgoing.entry(node.id.as_str()).or_default();
        node_order.push(node.id.as_str());
    }

    for (idx, edge) in doc.edges.iter().enumerate() {
        let from = edge.from.node.as_str();
        let to = edge.to.node.as_str();

        if !indegree.contains_key(from) {
            return Err(CompileError::MissingNode {
                edge_index: idx,
                node_id: from.to_string(),
            });
        }
        if !indegree.contains_key(to) {
            return Err(CompileError::MissingNode {
                edge_index: idx,
                node_id: to.to_string(),
            });
        }

        outgoing.entry(from).or_default().push(to);
        *indegree.entry(to).or_default() += 1;
    }

    let mut queue = VecDeque::new();
    for id in &node_order {
        if indegree.get(id).copied().unwrap_or_default() == 0 {
            queue.push_back(*id);
        }
    }

    let mut result = Vec::with_capacity(doc.nodes.len());
    while let Some(id) = queue.pop_front() {
        result.push(id.to_string());
        if let Some(next_list) = outgoing.get(id) {
            for next in next_list {
                if let Some(v) = indegree.get_mut(next) {
                    *v -= 1;
                    if *v == 0 {
                        queue.push_back(next);
                    }
                }
            }
        }
    }

    if result.len() != doc.nodes.len() {
        return Err(CompileError::CycleDetected);
    }

    Ok(CompilePlan { order: result })
}

#[cfg(test)]
mod tests {
    use super::*;
    use graph_spec::{Edge, Endpoint, GraphDoc, Node};

    fn node(id: &str) -> Node {
        Node {
            id: id.to_string(),
            kind: "test.node".to_string(),
            inputs: vec![],
            outputs: vec![],
            params: Default::default(),
        }
    }

    #[test]
    fn topological_sort_ok() {
        let doc = GraphDoc {
            version: 1,
            name: "ok".to_string(),
            nodes: vec![node("a"), node("b"), node("c")],
            edges: vec![
                Edge {
                    from: Endpoint {
                        node: "a".to_string(),
                        pin: "o".to_string(),
                    },
                    to: Endpoint {
                        node: "b".to_string(),
                        pin: "i".to_string(),
                    },
                },
                Edge {
                    from: Endpoint {
                        node: "b".to_string(),
                        pin: "o".to_string(),
                    },
                    to: Endpoint {
                        node: "c".to_string(),
                        pin: "i".to_string(),
                    },
                },
            ],
        };

        let plan = compile_graph(&doc).expect("must compile");
        assert_eq!(plan.order, vec!["a", "b", "c"]);
    }

    #[test]
    fn detect_cycle() {
        let doc = GraphDoc {
            version: 1,
            name: "cycle".to_string(),
            nodes: vec![node("a"), node("b")],
            edges: vec![
                Edge {
                    from: Endpoint {
                        node: "a".to_string(),
                        pin: "o".to_string(),
                    },
                    to: Endpoint {
                        node: "b".to_string(),
                        pin: "i".to_string(),
                    },
                },
                Edge {
                    from: Endpoint {
                        node: "b".to_string(),
                        pin: "o".to_string(),
                    },
                    to: Endpoint {
                        node: "a".to_string(),
                        pin: "i".to_string(),
                    },
                },
            ],
        };

        let err = compile_graph(&doc).expect_err("must fail");
        assert_eq!(err, CompileError::CycleDetected);
    }
}
