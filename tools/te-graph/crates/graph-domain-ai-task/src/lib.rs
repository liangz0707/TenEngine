use graph_plugin_api::{DomainPlugin, NodeTypeDescriptor};

pub struct AiTaskGraphPlugin;

const NODE_TYPES: &[NodeTypeDescriptor] = &[
    NodeTypeDescriptor {
        kind: "ai.selector",
        category: "composite",
        description: "Run children until one succeeds.",
    },
    NodeTypeDescriptor {
        kind: "ai.sequence",
        category: "composite",
        description: "Run children in order until one fails.",
    },
    NodeTypeDescriptor {
        kind: "ai.task_move_to",
        category: "task",
        description: "Move AI agent to target position.",
    },
    NodeTypeDescriptor {
        kind: "ai.task_wait",
        category: "task",
        description: "Wait for duration in seconds.",
    },
];

impl DomainPlugin for AiTaskGraphPlugin {
    fn domain_name(&self) -> &'static str {
        "aitaskgraph"
    }

    fn node_types(&self) -> &'static [NodeTypeDescriptor] {
        NODE_TYPES
    }
}
