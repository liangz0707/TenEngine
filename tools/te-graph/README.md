# te-graph (independent graph toolkit)

`te-graph` is a standalone Rust workspace for graph authoring and execution planning.
It is intentionally decoupled from TenEngine runtime modules.

## Goals

- Keep graph core independent from engine code.
- Support multiple domains via plugins (FrameGraph, ShaderGraph, ScriptGraph, AI graph).
- Enable lightweight tools and optional integration back into TenEngine through adapters.

## Workspace layout

- `graph-spec`: graph asset schema and parsing models.
- `graph-core`: generic DAG compile/planning.
- `graph-plugin-api`: domain plugin traits.
- `graph-domain-framegraph`: sample FrameGraph domain plugin.
- `graph-domain-shader`: sample ShaderGraph domain plugin.
- `graph-adapter-tenengine`: bridge layer (no direct runtime dependency yet).
- `graph-cli`: command line tool for validation and compile checks.
- `apps/graph-editor`: Tauri + React Flow desktop editor shell.

## Quick start

```bash
cd tools/te-graph
cargo run -p graph-cli -- validate examples/minimal.graph.json
cargo run -p graph-cli -- compile examples/minimal.graph.json
```

```bash
cd tools/te-graph/apps/graph-editor
npm install
npm run tauri dev
```

## Notes

- This workspace is independent and can be moved to a separate repository later.
- TenEngine integration is done only in adapter crates.
