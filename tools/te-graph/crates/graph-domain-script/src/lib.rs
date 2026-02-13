use graph_plugin_api::{DomainPlugin, NodeTypeDescriptor};

pub struct ScriptGraphPlugin;

const NODE_TYPES: &[NodeTypeDescriptor] = &[
    NodeTypeDescriptor {
        kind: "script.event_begin_play",
        category: "event",
        description: "Entry event for runtime script logic.",
    },
    NodeTypeDescriptor {
        kind: "script.call_function",
        category: "call",
        description: "Invoke a named function.",
    },
    NodeTypeDescriptor {
        kind: "script.branch",
        category: "flow",
        description: "Boolean branch with true/false routes.",
    },
    NodeTypeDescriptor {
        kind: "script.return",
        category: "flow",
        description: "Terminate script execution path.",
    },
];

impl DomainPlugin for ScriptGraphPlugin {
    fn domain_name(&self) -> &'static str {
        "scriptgraph"
    }

    fn node_types(&self) -> &'static [NodeTypeDescriptor] {
        NODE_TYPES
    }
}
