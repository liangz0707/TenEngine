// ScriptGraph domain: Unreal Blueprint-style script nodes
use graph_plugin_api::{DomainPlugin, NodeTypeDescriptor};

pub struct ScriptGraphPlugin;

const NODE_TYPES: &[NodeTypeDescriptor] = &[
    // --- Event ---
    NodeTypeDescriptor {
        kind: "script.event_begin_play",
        category: "event",
        description: "Called when gameplay starts (like BeginPlay).",
    },
    NodeTypeDescriptor {
        kind: "script.event_tick",
        category: "event",
        description: "Called every frame (DeltaSeconds).",
    },
    NodeTypeDescriptor {
        kind: "script.event_input_action",
        category: "event",
        description: "Input action triggered (key/button).",
    },
    NodeTypeDescriptor {
        kind: "script.event_custom",
        category: "event",
        description: "Custom event (dispatched from elsewhere).",
    },
    // --- Flow ---
    NodeTypeDescriptor {
        kind: "script.branch",
        category: "flow",
        description: "Branch on condition (true/false).",
    },
    NodeTypeDescriptor {
        kind: "script.sequence",
        category: "flow",
        description: "Execute pins in sequence (Then 0, 1, 2...).",
    },
    NodeTypeDescriptor {
        kind: "script.for_loop",
        category: "flow",
        description: "For loop (First Index, Last Index, Loop Body).",
    },
    NodeTypeDescriptor {
        kind: "script.for_each_loop",
        category: "flow",
        description: "For each element in array.",
    },
    NodeTypeDescriptor {
        kind: "script.while_loop",
        category: "flow",
        description: "While condition is true.",
    },
    NodeTypeDescriptor {
        kind: "script.return",
        category: "flow",
        description: "Return / terminate execution path.",
    },
    NodeTypeDescriptor {
        kind: "script.delay",
        category: "flow",
        description: "Delay execution for duration.",
    },
    NodeTypeDescriptor {
        kind: "script.do_once",
        category: "flow",
        description: "Execute only once (then skip).",
    },
    NodeTypeDescriptor {
        kind: "script.gate",
        category: "flow",
        description: "Gate (open/close to allow flow).",
    },
    // --- Call ---
    NodeTypeDescriptor {
        kind: "script.call_function",
        category: "call",
        description: "Call a named function.",
    },
    NodeTypeDescriptor {
        kind: "script.call_pure",
        category: "call",
        description: "Call pure function (no execution pin).",
    },
    NodeTypeDescriptor {
        kind: "script.print",
        category: "call",
        description: "Print string to log.",
    },
    // --- Variable ---
    NodeTypeDescriptor {
        kind: "script.get_variable",
        category: "variable",
        description: "Get variable value.",
    },
    NodeTypeDescriptor {
        kind: "script.set_variable",
        category: "variable",
        description: "Set variable value.",
    },
    NodeTypeDescriptor {
        kind: "script.promote_to_variable",
        category: "variable",
        description: "Promote value to variable.",
    },
    // --- Logic ---
    NodeTypeDescriptor {
        kind: "script.equal",
        category: "logic",
        description: "A == B.",
    },
    NodeTypeDescriptor {
        kind: "script.not_equal",
        category: "logic",
        description: "A != B.",
    },
    NodeTypeDescriptor {
        kind: "script.greater",
        category: "logic",
        description: "A > B.",
    },
    NodeTypeDescriptor {
        kind: "script.less",
        category: "logic",
        description: "A < B.",
    },
    NodeTypeDescriptor {
        kind: "script.and",
        category: "logic",
        description: "Boolean AND.",
    },
    NodeTypeDescriptor {
        kind: "script.or",
        category: "logic",
        description: "Boolean OR.",
    },
    NodeTypeDescriptor {
        kind: "script.not",
        category: "logic",
        description: "Boolean NOT.",
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
