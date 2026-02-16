// ShaderGraph domain: Unreal Material / Shader node set
use graph_plugin_api::{DomainPlugin, NodeTypeDescriptor};

pub struct ShaderGraphPlugin;

const NODE_TYPES: &[NodeTypeDescriptor] = &[
    // --- Input / Output ---
    NodeTypeDescriptor {
        kind: "shader.input",
        category: "io",
        description: "Shader graph input (e.g. UV, Normal).",
    },
    NodeTypeDescriptor {
        kind: "shader.output",
        category: "io",
        description: "Shader graph final output (Base Color, Metallic, etc.).",
    },
    NodeTypeDescriptor {
        kind: "shader.vertex_interpolant",
        category: "io",
        description: "Pass data from vertex to pixel shader.",
    },
    // --- Texture ---
    NodeTypeDescriptor {
        kind: "shader.sample_texture2d",
        category: "texture",
        description: "Sample a 2D texture by UV.",
    },
    NodeTypeDescriptor {
        kind: "shader.sample_texture_cube",
        category: "texture",
        description: "Sample a cube map.",
    },
    NodeTypeDescriptor {
        kind: "shader.texture_object",
        category: "texture",
        description: "Texture object parameter.",
    },
    NodeTypeDescriptor {
        kind: "shader.tex_coord",
        category: "texture",
        description: "Texture coordinate (UV) input.",
    },
    NodeTypeDescriptor {
        kind: "shader.parallax_occlusion",
        category: "texture",
        description: "Parallax occlusion mapping.",
    },
    // --- Math ---
    NodeTypeDescriptor {
        kind: "shader.multiply",
        category: "math",
        description: "Multiply (A * B).",
    },
    NodeTypeDescriptor {
        kind: "shader.add",
        category: "math",
        description: "Add (A + B).",
    },
    NodeTypeDescriptor {
        kind: "shader.subtract",
        category: "math",
        description: "Subtract (A - B).",
    },
    NodeTypeDescriptor {
        kind: "shader.divide",
        category: "math",
        description: "Divide (A / B).",
    },
    NodeTypeDescriptor {
        kind: "shader.lerp",
        category: "math",
        description: "Linear interpolate (Alpha blend).",
    },
    NodeTypeDescriptor {
        kind: "shader.clamp",
        category: "math",
        description: "Clamp value to min/max.",
    },
    NodeTypeDescriptor {
        kind: "shader.saturate",
        category: "math",
        description: "Clamp to [0, 1].",
    },
    NodeTypeDescriptor {
        kind: "shader.dot",
        category: "math",
        description: "Dot product.",
    },
    NodeTypeDescriptor {
        kind: "shader.cross",
        category: "math",
        description: "Cross product.",
    },
    NodeTypeDescriptor {
        kind: "shader.normalize",
        category: "math",
        description: "Normalize vector.",
    },
    NodeTypeDescriptor {
        kind: "shader.abs",
        category: "math",
        description: "Absolute value.",
    },
    NodeTypeDescriptor {
        kind: "shader.pow",
        category: "math",
        description: "Power (A ^ B).",
    },
    NodeTypeDescriptor {
        kind: "shader.sqrt",
        category: "math",
        description: "Square root.",
    },
    NodeTypeDescriptor {
        kind: "shader.one_minus",
        category: "math",
        description: "1 - X.",
    },
    // --- Vector / Constant ---
    NodeTypeDescriptor {
        kind: "shader.constant",
        category: "constant",
        description: "Constant scalar value.",
    },
    NodeTypeDescriptor {
        kind: "shader.constant2",
        category: "constant",
        description: "Constant 2D vector.",
    },
    NodeTypeDescriptor {
        kind: "shader.constant3",
        category: "constant",
        description: "Constant 3D vector.",
    },
    NodeTypeDescriptor {
        kind: "shader.constant4",
        category: "constant",
        description: "Constant 4D vector.",
    },
    NodeTypeDescriptor {
        kind: "shader.append_vector",
        category: "constant",
        description: "Append vector channels.",
    },
    NodeTypeDescriptor {
        kind: "shader.break_vector",
        category: "constant",
        description: "Break vector into components.",
    },
    // --- Utility ---
    NodeTypeDescriptor {
        kind: "shader.fresnel",
        category: "utility",
        description: "Fresnel / rim lighting.",
    },
    NodeTypeDescriptor {
        kind: "shader.normal_from_height",
        category: "utility",
        description: "Derive normal from height map.",
    },
    NodeTypeDescriptor {
        kind: "shader.desaturation",
        category: "utility",
        description: "Desaturate by luminance.",
    },
    NodeTypeDescriptor {
        kind: "shader.if",
        category: "utility",
        description: "Conditional (A >= B ? A : B).",
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
