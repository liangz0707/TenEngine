use graph_plugin_api::{DomainPlugin, NodeTypeDescriptor};

pub struct FrameGraphPlugin;

const NODE_TYPES: &[NodeTypeDescriptor] = &[
    NodeTypeDescriptor {
        kind: "frame.begin_pass",
        category: "pass",
        description: "Begin a render pass scope.",
    },
    NodeTypeDescriptor {
        kind: "frame.read_resource",
        category: "resource",
        description: "Declare read dependency to a resource.",
    },
    NodeTypeDescriptor {
        kind: "frame.write_resource",
        category: "resource",
        description: "Declare write dependency to a resource.",
    },
    NodeTypeDescriptor {
        kind: "frame.end_pass",
        category: "pass",
        description: "Finalize a render pass scope.",
    },
];

impl DomainPlugin for FrameGraphPlugin {
    fn domain_name(&self) -> &'static str {
        "framegraph"
    }

    fn node_types(&self) -> &'static [NodeTypeDescriptor] {
        NODE_TYPES
    }
}
