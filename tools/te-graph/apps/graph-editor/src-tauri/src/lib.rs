use graph_adapter_tenengine::compile_for_tenengine;
use graph_core::compile_graph;
use graph_domain_ai_task::AiTaskGraphPlugin;
use graph_domain_framegraph::FrameGraphPlugin;
use graph_domain_script::ScriptGraphPlugin;
use graph_domain_shader::ShaderGraphPlugin;
use graph_plugin_api::PluginRegistry;
use graph_spec::parse_graph_json;
use serde::Serialize;
use std::fs;

#[derive(Debug, Serialize)]
struct CompilePassDto {
  node_id: String,
  node_kind: String,
  pass_name: Option<String>,
}

#[derive(Debug, Serialize)]
struct CompileResultDto {
  graph_name: String,
  execution_order: Vec<String>,
  tenengine_passes: Vec<CompilePassDto>,
}

fn build_plugin_registry() -> PluginRegistry {
  let mut plugins = PluginRegistry::new();
  plugins.register(Box::new(FrameGraphPlugin));
  plugins.register(Box::new(ShaderGraphPlugin));
  plugins.register(Box::new(ScriptGraphPlugin));
  plugins.register(Box::new(AiTaskGraphPlugin));
  plugins
}

#[tauri::command]
fn load_example_graph() -> String {
  include_str!("../../../../examples/minimal.graph.json").to_string()
}

#[tauri::command]
fn load_graph_from_path(path: String) -> Result<String, String> {
  fs::read_to_string(path).map_err(|e| e.to_string())
}

#[tauri::command]
fn save_graph_to_path(path: String, graph_text: String) -> Result<(), String> {
  let p = std::path::Path::new(&path);
  if let Some(parent) = p.parent() {
    if !parent.as_os_str().is_empty() {
      fs::create_dir_all(parent).map_err(|e| e.to_string())?;
    }
  }
  fs::write(path, graph_text).map_err(|e| e.to_string())
}

#[derive(Debug, Serialize)]
struct NodeTypeInfo {
  kind: String,
  description: String,
}

/// Returns nodes grouped by domain and category: { "framegraph": { "pass": [{ kind, description }, ...] }, ... }
#[tauri::command]
fn list_nodes_by_domain_and_category() -> std::collections::HashMap<String, std::collections::HashMap<String, Vec<NodeTypeInfo>>> {
  let plugins = build_plugin_registry();
  let mut by_domain: std::collections::HashMap<String, std::collections::HashMap<String, Vec<NodeTypeInfo>>> =
    std::collections::HashMap::new();
  for plugin in plugins.plugins() {
    let domain = plugin.domain_name().to_string();
    let categories = by_domain.entry(domain).or_default();
    for nt in plugin.node_types() {
      let entry = categories
        .entry(nt.category.to_string())
        .or_default();
      entry.push(NodeTypeInfo {
        kind: nt.kind.to_string(),
        description: nt.description.to_string(),
      });
    }
  }
  by_domain
}

#[tauri::command]
fn list_supported_node_kinds() -> Vec<String> {
  let plugins = build_plugin_registry();
  let mut out = Vec::new();
  for plugin in plugins.plugins() {
    for node in plugin.node_types() {
      out.push(node.kind.to_string());
    }
  }
  out.sort();
  out.dedup();
  out
}

#[tauri::command]
fn compile_graph_text(graph_text: String) -> Result<CompileResultDto, String> {
  let doc = parse_graph_json(&graph_text).map_err(|e| e.to_string())?;
  let plugins = build_plugin_registry();
  for node in &doc.nodes {
    plugins.validate_node_with_any_plugin(node)?;
  }
  let plan = compile_graph(&doc).map_err(|e| e.to_string())?;
  let passes = compile_for_tenengine(&doc).map_err(|e| e.to_string())?;

  let tenengine_passes = passes
    .into_iter()
    .map(|p| CompilePassDto {
      node_id: p.node_id,
      node_kind: p.node_kind,
      pass_name: p.pass_name,
    })
    .collect();

  Ok(CompileResultDto {
    graph_name: doc.name,
    execution_order: plan.order,
    tenengine_passes,
  })
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
  tauri::Builder::default()
    .invoke_handler(tauri::generate_handler![
      load_example_graph,
      load_graph_from_path,
      save_graph_to_path,
      list_supported_node_kinds,
      list_nodes_by_domain_and_category,
      compile_graph_text
    ])
    .run(tauri::generate_context!())
    .expect("error while running tauri application");
}
