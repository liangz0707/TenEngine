use graph_core::{compile_graph, CompileError};
use graph_spec::GraphDoc;
use serde_json::Value;
use std::collections::HashMap;

#[derive(Debug, Clone, PartialEq)]
pub struct FrameGraphPassDesc {
    pub node_id: String,
    pub node_kind: String,
    pub pass_name: Option<String>,
}

#[derive(Debug, Clone, PartialEq)]
pub enum AdapterError {
    Compile(CompileError),
}

impl std::fmt::Display for AdapterError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            AdapterError::Compile(e) => write!(f, "{e}"),
        }
    }
}

impl std::error::Error for AdapterError {}

pub fn compile_for_tenengine(doc: &GraphDoc) -> Result<Vec<FrameGraphPassDesc>, AdapterError> {
    let plan = compile_graph(doc).map_err(AdapterError::Compile)?;
    let node_lookup: HashMap<&str, _> = doc.nodes.iter().map(|n| (n.id.as_str(), n)).collect();

    let mut out = Vec::new();
    for node_id in plan.order {
        if let Some(node) = node_lookup.get(node_id.as_str()) {
            if node.kind.starts_with("frame.") {
                out.push(FrameGraphPassDesc {
                    node_id: node.id.clone(),
                    node_kind: node.kind.clone(),
                    pass_name: extract_string_param(node.params.get("pass_name")),
                });
            }
        }
    }
    Ok(out)
}

fn extract_string_param(value: Option<&Value>) -> Option<String> {
    value.and_then(|v| v.as_str().map(ToString::to_string))
}

#[cfg(test)]
mod tests {
    use super::*;
    use graph_spec::parse_graph_json;

    #[test]
    fn compile_to_pass_descs() {
        let doc = parse_graph_json(
            r#"{
              "version": 1,
              "name": "sample",
              "nodes": [
                {"id":"n1","kind":"frame.begin_pass","inputs":[],"outputs":[],"params":{"pass_name":"Main"}},
                {"id":"n2","kind":"frame.end_pass","inputs":[],"outputs":[],"params":{}}
              ],
              "edges": [
                {"from":{"node":"n1","pin":"o"},"to":{"node":"n2","pin":"i"}}
              ]
            }"#,
        )
        .expect("parse");

        let result = compile_for_tenengine(&doc).expect("compile");
        assert_eq!(result.len(), 2);
        assert_eq!(result[0].pass_name.as_deref(), Some("Main"));
    }
}
