use graph_adapter_tenengine::compile_for_tenengine;
use graph_core::compile_graph;
use graph_spec::parse_graph_json;
use serde::Serialize;

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

#[tauri::command]
fn load_example_graph() -> String {
  include_str!("../../../../examples/minimal.graph.json").to_string()
}

#[tauri::command]
fn compile_graph_text(graph_text: String) -> Result<CompileResultDto, String> {
  let doc = parse_graph_json(&graph_text).map_err(|e| e.to_string())?;
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
      compile_graph_text
    ])
    .run(tauri::generate_context!())
    .expect("error while running tauri application");
}
