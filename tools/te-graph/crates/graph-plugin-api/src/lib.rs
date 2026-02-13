use graph_spec::Node;

#[derive(Debug, Clone, PartialEq, Eq)]
pub struct NodeTypeDescriptor {
    pub kind: &'static str,
    pub category: &'static str,
    pub description: &'static str,
}

pub trait DomainPlugin: Send + Sync {
    fn domain_name(&self) -> &'static str;
    fn node_types(&self) -> &'static [NodeTypeDescriptor];

    fn supports_kind(&self, kind: &str) -> bool {
        self.node_types().iter().any(|x| x.kind == kind)
    }

    fn validate_node(&self, node: &Node) -> Result<(), String> {
        if self.supports_kind(&node.kind) {
            Ok(())
        } else {
            Err(format!(
                "node kind '{}' is not supported in domain '{}'",
                node.kind,
                self.domain_name()
            ))
        }
    }
}

#[derive(Default)]
pub struct PluginRegistry {
    plugins: Vec<Box<dyn DomainPlugin>>,
}

impl PluginRegistry {
    pub fn new() -> Self {
        Self { plugins: vec![] }
    }

    pub fn register(&mut self, plugin: Box<dyn DomainPlugin>) {
        self.plugins.push(plugin);
    }

    pub fn plugins(&self) -> &[Box<dyn DomainPlugin>] {
        &self.plugins
    }

    pub fn validate_node_with_any_plugin(&self, node: &Node) -> Result<(), String> {
        for plugin in &self.plugins {
            if plugin.supports_kind(&node.kind) {
                return plugin.validate_node(node);
            }
        }
        Err(format!(
            "node kind '{}' is not handled by any plugin",
            node.kind
        ))
    }
}
