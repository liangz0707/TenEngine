# 模块依赖图（Module Dependency Map）

用于多 Agent 协作时快速查看：**谁依赖谁**、**改某个模块会影响谁**。接口边界以 `specs/_contracts/` 下契约为准。

## 依赖关系图（文字版）

```
                     ┌─────────────────────────────────────┐
                     │  003-editor-system                  │
                     │  (依赖: RCI, Core/平台)              │
                     └──────────────┬──────────────────────┘
                                    │
┌───────────────────┐               │               ┌───────────────────┐
│ 004-resource-system│               │               │ 005-shader-system  │
│ (依赖: Core/平台)  │               │               │ (依赖: RCI, Core)  │
└─────────┬─────────┘               │               └─────────┬─────────┘
          │                         │                         │
          │                         ▼                         │
          │              ┌─────────────────────┐             │
          │              │ 006-render-pipeline  │             │
          │              │ (依赖: RCI 契约)     │◄────────────┘
          │              │ 输出: 命令缓冲→RCI    │
          │              └──────────┬────────────┘
          │                         │
          │                         ▼
          │              ┌─────────────────────┐
          └─────────────►│ 002-rendering-rci   │
                         │ (依赖: Core/平台)   │
                         │ 契约: rci-public-api│
                         └──────────┬──────────┘
                                    │
                                    ▼
                         ┌─────────────────────┐
                         │ 001-engine-core     │
                         │ 契约: core-public-api│
                         └─────────────────────┘
```

## 依赖表（下游 → 上游）

| 模块 (spec) | 依赖的上游模块 | 依赖的契约文件 |
|-------------|----------------|----------------|
| 001-engine-core-module | — | 无（根模块） |
| 002-rendering-rci-interface | Core | core-public-api.md |
| 003-editor-system | Core, RCI | core-public-api.md, rci-public-api.md |
| 004-resource-system | Core | core-public-api.md |
| 005-shader-system | Core, RCI | core-public-api.md, rci-public-api.md |
| 006-render-pipeline-system | RCI（命令缓冲消费方） | rci-public-api.md, pipeline-to-rci.md |
| 006-thirdparty-integration-tool | 视实现而定 | 按实际依赖填写 |

## 谁被谁依赖（上游 → 下游）

修改某模块的**对外接口**时，需考虑以下**下游**模块的兼容性：

| 提供方模块 | 依赖它的下游 |
|------------|--------------|
| 001-engine-core-module | 002, 003, 004, 005 |
| 002-rendering-rci-interface | 003, 005, 006 |
| 006-render-pipeline-system | 002（通过 pipeline-to-rci 契约） |

**流程**：修改 Core 的公开 API → 更新 `core-public-api.md` → 检查 002/003/004/005 的 spec 或实现是否需要同步修改。

---

*本文件与 specs 下各 spec 的 Dependencies 应保持一致；任一 Agent 修改依赖关系时请同步更新此表。*
