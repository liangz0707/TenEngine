use graph_plugin_api::{DomainPlugin, NodeTypeDescriptor};

pub struct ShaderGraphPlugin;

const NODE_TYPES: &[NodeTypeDescriptor] = &[
    NodeTypeDescriptor {
        kind: "shader.input",
        category: "io",
        description: "Shader graph input value.",
    },
    NodeTypeDescriptor {
        kind: "shader.sample_texture2d",
        category: "texture",
        description: "Sample a texture by UV.",
    },
    NodeTypeDescriptor {
        kind: "shader.multiply",
        category: "math",
        description: "Multiply two vectors/scalars.",
    },
    NodeTypeDescriptor {
        kind: "shader.output",
        category: "io",
        description: "Shader graph final output.",
    },
];

impl DomainPlugin for ShaderGraphPlugin {
    fn domain_name(&self) -> &'static str {
        "shadergraph"
    }

    fn node_types(&self) -> &'static [NodeTypeDescriptor] {
        NODE_TYPES
    }
}
