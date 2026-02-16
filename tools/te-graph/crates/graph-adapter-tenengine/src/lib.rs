use graph_core::{compile_graph, CompileError};
use graph_spec::GraphDoc;
use serde::{Deserialize, Serialize};
use serde_json::Value;
use std::collections::HashMap;

/// Frame graph pass descriptor for TenEngine
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct FrameGraphPassDesc {
    pub node_id: String,
    pub node_kind: String,
    pub pass_name: Option<String>,
}

/// RenderPipeline configuration generated from graph
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct RenderPipelineConfig {
    pub name: String,
    pub passes: Vec<PassConfig>,
    pub resources: Vec<ResourceConfig>,
}

/// Individual pass configuration
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct PassConfig {
    pub id: String,
    pub kind: PassKind,
    pub name: String,
    pub inputs: Vec<String>,
    pub outputs: Vec<String>,
    pub width: u32,
    pub height: u32,
    pub material: Option<String>,
    pub mesh: Option<String>,
}

/// Pass kind enum matching TenEngine's PassKind
#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub enum PassKind {
    Scene,
    Light,
    PostProcess,
    Effect,
    Custom,
}

impl std::fmt::Display for PassKind {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            PassKind::Scene => write!(f, "Scene"),
            PassKind::Light => write!(f, "Light"),
            PassKind::PostProcess => write!(f, "PostProcess"),
            PassKind::Effect => write!(f, "Effect"),
            PassKind::Custom => write!(f, "Custom"),
        }
    }
}

/// Resource configuration
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct ResourceConfig {
    pub id: String,
    pub kind: ResourceKind,
    pub format: Option<String>,
    pub width: u32,
    pub height: u32,
}

#[derive(Debug, Clone, Copy, PartialEq, Serialize, Deserialize)]
pub enum ResourceKind {
    Texture2D,
    TextureCube,
    Buffer,
}

#[derive(Debug, Clone, PartialEq)]
pub enum AdapterError {
    Compile(CompileError),
    InvalidNodeKind(String),
}

impl std::fmt::Display for AdapterError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        match self {
            AdapterError::Compile(e) => write!(f, "{e}"),
            AdapterError::InvalidNodeKind(kind) => write!(f, "Invalid node kind: {kind}"),
        }
    }
}

impl std::error::Error for AdapterError {}

/// Compile graph to TenEngine frame graph pass descriptors
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

/// Compile graph to full RenderPipeline configuration
pub fn compile_to_pipeline_config(doc: &GraphDoc) -> Result<RenderPipelineConfig, AdapterError> {
    let plan = compile_graph(doc).map_err(AdapterError::Compile)?;
    let node_lookup: HashMap<&str, _> = doc.nodes.iter().map(|n| (n.id.as_str(), n)).collect();

    let mut passes = Vec::new();
    let mut resources = Vec::new();
    let mut resource_ids = HashMap::new();

    for node_id in &plan.order {
        if let Some(node) = node_lookup.get(node_id.as_str()) {
            let pass_kind = node_kind_to_pass_kind(&node.kind)?;
            
            // Extract inputs and outputs from edges
            let inputs: Vec<String> = doc.edges
                .iter()
                .filter(|e| e.to.node == node.id)
                .map(|e| e.from.pin.clone())
                .collect();
            
            let outputs: Vec<String> = doc.edges
                .iter()
                .filter(|e| e.from.node == node.id)
                .map(|e| e.to.pin.clone())
                .collect();

            // Extract dimensions from params
            let width = extract_u32_param(node.params.get("width")).unwrap_or(1280);
            let height = extract_u32_param(node.params.get("height")).unwrap_or(720);

            // Track resources
            for output in &outputs {
                if !resource_ids.contains_key(output) {
                    let res_id = format!("res_{}", output);
                    resource_ids.insert(output.clone(), res_id.clone());
                    resources.push(ResourceConfig {
                        id: res_id,
                        kind: ResourceKind::Texture2D,
                        format: Some("RGBA8".to_string()),
                        width,
                        height,
                    });
                }
            }

            let pass_name = extract_string_param(node.params.get("pass_name"))
                .unwrap_or_else(|| format!("Pass_{}", passes.len()));

            passes.push(PassConfig {
                id: node.id.clone(),
                kind: pass_kind,
                name: pass_name,
                inputs,
                outputs,
                width,
                height,
                material: extract_string_param(node.params.get("material")),
                mesh: extract_string_param(node.params.get("mesh")),
            });
        }
    }

    Ok(RenderPipelineConfig {
        name: doc.name.clone(),
        passes,
        resources,
    })
}

/// Convert node kind string to PassKind
fn node_kind_to_pass_kind(kind: &str) -> Result<PassKind, AdapterError> {
    if kind.starts_with("frame.") {
        match kind {
            "frame.begin_pass" | "frame.end_pass" | "frame.read_resource" | "frame.write_resource" => Ok(PassKind::Scene),
            _ => Ok(PassKind::Custom),
        }
    } else if kind.starts_with("shader.") {
        Ok(PassKind::Effect)
    } else if kind.starts_with("script.") {
        Ok(PassKind::Custom)
    } else if kind.starts_with("ai.") {
        Ok(PassKind::Custom)
    } else {
        Err(AdapterError::InvalidNodeKind(kind.to_string()))
    }
}

fn extract_string_param(value: Option<&Value>) -> Option<String> {
    value.and_then(|v| v.as_str().map(ToString::to_string))
}

fn extract_u32_param(value: Option<&Value>) -> Option<u32> {
    value.and_then(|v| v.as_u64().map(|n| n as u32))
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

    #[test]
    fn compile_to_pipeline_config_test() {
        let doc = parse_graph_json(
            r#"{
              "version": 1,
              "name": "test_pipeline",
              "nodes": [
                {"id":"n1","kind":"frame.begin_pass","inputs":[],"outputs":[{"id":"out","ty":"color"}],"params":{"pass_name":"GBuffer","width":1920,"height":1080}},
                {"id":"n2","kind":"frame.end_pass","inputs":[{"id":"in","ty":"color"}],"outputs":[],"params":{"pass_name":"Final"}}
              ],
              "edges": [
                {"from":{"node":"n1","pin":"out"},"to":{"node":"n2","pin":"in"}}
              ]
            }"#,
        )
        .expect("parse");

        let config = compile_to_pipeline_config(&doc).expect("compile");
        assert_eq!(config.name, "test_pipeline");
        assert_eq!(config.passes.len(), 2);
        assert_eq!(config.passes[0].name, "GBuffer");
        assert_eq!(config.passes[0].width, 1920);
        assert_eq!(config.passes[0].height, 1080);
    }
}
