// FrameGraph domain: Unreal-style render pass / resource nodes
use graph_plugin_api::{DomainPlugin, NodeTypeDescriptor};

pub struct FrameGraphPlugin;

const NODE_TYPES: &[NodeTypeDescriptor] = &[
    // --- Pass ---
    NodeTypeDescriptor {
        kind: "frame.begin_pass",
        category: "pass",
        description: "Begin a render pass scope.",
    },
    NodeTypeDescriptor {
        kind: "frame.end_pass",
        category: "pass",
        description: "End / finalize a render pass scope.",
    },
    NodeTypeDescriptor {
        kind: "frame.clear",
        category: "pass",
        description: "Clear render target (color/depth).",
    },
    NodeTypeDescriptor {
        kind: "frame.resolve",
        category: "pass",
        description: "Resolve MSAA render target.",
    },
    NodeTypeDescriptor {
        kind: "frame.copy_texture",
        category: "pass",
        description: "Copy between textures.",
    },
    NodeTypeDescriptor {
        kind: "frame.blit",
        category: "pass",
        description: "Fullscreen blit / post-process pass.",
    },
    // --- Resource ---
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
        kind: "frame.create_texture2d",
        category: "resource",
        description: "Create 2D texture resource.",
    },
    NodeTypeDescriptor {
        kind: "frame.create_texture_cube",
        category: "resource",
        description: "Create cube map texture.",
    },
    NodeTypeDescriptor {
        kind: "frame.create_buffer",
        category: "resource",
        description: "Create buffer (vertex/index/constant).",
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
