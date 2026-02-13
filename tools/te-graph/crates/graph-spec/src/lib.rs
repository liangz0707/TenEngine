use serde::{Deserialize, Serialize};
use serde_json::{Map, Value};

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq)]
pub struct Endpoint {
    pub node: String,
    pub pin: String,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq)]
pub struct Edge {
    pub from: Endpoint,
    pub to: Endpoint,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq, Eq)]
pub struct Pin {
    pub id: String,
    pub ty: String,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
pub struct Node {
    pub id: String,
    pub kind: String,
    #[serde(default)]
    pub inputs: Vec<Pin>,
    #[serde(default)]
    pub outputs: Vec<Pin>,
    #[serde(default)]
    pub params: Map<String, Value>,
}

#[derive(Debug, Clone, Serialize, Deserialize, PartialEq)]
pub struct GraphDoc {
    pub version: u32,
    pub name: String,
    #[serde(default)]
    pub nodes: Vec<Node>,
    #[serde(default)]
    pub edges: Vec<Edge>,
}

#[derive(Debug, Clone, PartialEq, Eq)]
pub enum SpecError {
    Parse(String),
    EmptyName,
    DuplicateNodeId(String),
}

impl std::fmt::Display for SpecError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            SpecError::Parse(e) => write!(f, "parse error: {e}"),
            SpecError::EmptyName => write!(f, "graph name is empty"),
            SpecError::DuplicateNodeId(id) => write!(f, "duplicate node id: {id}"),
        }
    }
}

impl std::error::Error for SpecError {}

pub fn parse_graph_json(text: &str) -> Result<GraphDoc, SpecError> {
    let doc: GraphDoc = serde_json::from_str(text).map_err(|e| SpecError::Parse(e.to_string()))?;
    validate_graph_doc(&doc)?;
    Ok(doc)
}

pub fn to_graph_json_pretty(doc: &GraphDoc) -> Result<String, SpecError> {
    serde_json::to_string_pretty(doc).map_err(|e| SpecError::Parse(e.to_string()))
}

pub fn validate_graph_doc(doc: &GraphDoc) -> Result<(), SpecError> {
    if doc.name.trim().is_empty() {
        return Err(SpecError::EmptyName);
    }
    let mut seen = std::collections::HashSet::new();
    for node in &doc.nodes {
        if !seen.insert(node.id.as_str()) {
            return Err(SpecError::DuplicateNodeId(node.id.clone()));
        }
    }
    Ok(())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parse_and_validate_ok() {
        let text = r#"{
          "version": 1,
          "name": "sample",
          "nodes": [{"id":"a","kind":"k","inputs":[],"outputs":[],"params":{}}],
          "edges": []
        }"#;
        let doc = parse_graph_json(text).expect("must parse");
        assert_eq!(doc.name, "sample");
        assert_eq!(doc.nodes.len(), 1);
    }

    #[test]
    fn detect_duplicate_node_id() {
        let text = r#"{
          "version": 1,
          "name": "sample",
          "nodes": [
            {"id":"a","kind":"k","inputs":[],"outputs":[],"params":{}},
            {"id":"a","kind":"k2","inputs":[],"outputs":[],"params":{}}
          ],
          "edges": []
        }"#;
        let err = parse_graph_json(text).expect_err("must fail");
        match err {
            SpecError::DuplicateNodeId(id) => assert_eq!(id, "a"),
            _ => panic!("unexpected error"),
        }
    }
}
