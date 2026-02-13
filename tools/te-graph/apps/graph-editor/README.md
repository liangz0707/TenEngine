# TE Graph Editor (Tauri + React Flow)

This app is the UI shell for the independent Rust graph toolkit.

## Stack

- Frontend: React + TypeScript + React Flow
- Desktop shell: Tauri (Rust)
- Backend commands: calls into `graph-spec`, `graph-core`, and `graph-adapter-tenengine`

## Commands

```bash
npm install
npm run dev           # frontend only
npm run tauri dev     # desktop app
npm run build
```

## Linux note

Tauri on Linux needs WebKitGTK/GTK development packages (for example `gdk-3.0` pkg-config).
If they are missing, frontend build still works but `cargo check` in `src-tauri` will fail until system deps are installed.

## Current features

- Load sample graph JSON from Rust side.
- Load/save graph JSON by file path via Tauri backend commands.
- Render graph nodes and edges in React Flow.
- Add node on canvas and create links by dragging handles.
- Sync JSON and canvas in both directions.
- Compile graph via Rust Tauri command with plugin validation and show execution order.
