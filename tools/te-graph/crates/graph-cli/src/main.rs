use graph_adapter_tenengine::compile_for_tenengine;
use graph_core::compile_graph;
use graph_domain_ai_task::AiTaskGraphPlugin;
use graph_domain_framegraph::FrameGraphPlugin;
use graph_domain_shader::ShaderGraphPlugin;
use graph_domain_script::ScriptGraphPlugin;
use graph_plugin_api::PluginRegistry;
use graph_spec::parse_graph_json;
use std::fs;
use std::process::ExitCode;

fn main() -> ExitCode {
    let args: Vec<String> = std::env::args().collect();
    if args.len() < 3 {
        print_usage();
        return ExitCode::from(2);
    }

    let command = args[1].as_str();
    let input_path = args[2].as_str();

    let text = match fs::read_to_string(input_path) {
        Ok(v) => v,
        Err(e) => {
            eprintln!("failed to read '{input_path}': {e}");
            return ExitCode::from(1);
        }
    };

    let doc = match parse_graph_json(&text) {
        Ok(v) => v,
        Err(e) => {
            eprintln!("invalid graph file: {e}");
            return ExitCode::from(1);
        }
    };

    let mut plugins = PluginRegistry::new();
    plugins.register(Box::new(FrameGraphPlugin));
    plugins.register(Box::new(ShaderGraphPlugin));
    plugins.register(Box::new(ScriptGraphPlugin));
    plugins.register(Box::new(AiTaskGraphPlugin));

    for node in &doc.nodes {
        if let Err(e) = plugins.validate_node_with_any_plugin(node) {
            eprintln!("{e}");
            return ExitCode::from(1);
        }
    }

    match command {
        "validate" => {
            println!(
                "validate ok: name='{}', nodes={}, edges={}",
                doc.name,
                doc.nodes.len(),
                doc.edges.len()
            );
            ExitCode::SUCCESS
        }
        "compile" => {
            let plan = match compile_graph(&doc) {
                Ok(v) => v,
                Err(e) => {
                    eprintln!("compile failed: {e}");
                    return ExitCode::from(1);
                }
            };

            println!("compile ok: execution order = {:?}", plan.order);

            match compile_for_tenengine(&doc) {
                Ok(passes) => {
                    println!("tenengine adapter passes = {}", passes.len());
                    ExitCode::SUCCESS
                }
                Err(e) => {
                    eprintln!("adapter compile failed: {e}");
                    ExitCode::from(1)
                }
            }
        }
        _ => {
            print_usage();
            ExitCode::from(2)
        }
    }
}

fn print_usage() {
    eprintln!("usage: graph-cli <validate|compile> <graph.json>");
}
