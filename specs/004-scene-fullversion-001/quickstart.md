# Quickstart: 004-Scene 完整功能集

**Branch**: 004-scene-fullversion-001 | **Date**: 2026-01-29

## 前置条件

- C++17 编译器、CMake 3.16+
- 001-Core、002-Object 已构建并可用（数学类型、容器、日志等）
- 实现前从 T0-contracts 拉取最新契约：`git fetch origin T0-contracts` 后 `git merge origin/T0-contracts`

## 构建

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

（具体 CMake 子工程名与依赖路径以仓库根目录的 CMake 配置为准；004-Scene 为静态库或动态库，由主工程或下游模块链接。）

## 使用顺序

1. **初始化**：在 Core、Object 可用之后，获取或创建当前 World（如默认创建一个 World 或通过 AddWorld 创建）。
2. **场景图**：在当前 World 下创建根节点与子节点（返回 NodeId）；设置 Parent/Children、LocalTransform；修改 LocalTransform 后调用 SetDirty，再在帧末或合适时机调用 UpdateTransforms。
3. **层级**：使用 Traverse 或 FindByName/FindByType 遍历/查找节点；用 GetPath、GetId 获取路径与 ID；用 SetActive 设置节点/子树激活状态。
4. **多 World**：AddWorld 增加 World；SetActiveWorld 切换当前 World（下一帧或下一次 UpdateTransforms 生效）；GetCurrentWorld 查询当前活动 WorldRef。
5. **Level**：在与 013-Resource 约定句柄与加载时机后，调用 LoadLevel/UnloadLevel；Level 资源句柄按 Resource 契约管理。
6. **Entity 根挂接**：在与 005-Entity 约定接口后，在当前（或指定）World 上挂接根实体；下游基于 SceneRef/WorldRef、NodeId 与 Entity 协作。

## 约束

- HierarchyIterator 单次有效，遍历结束后不得再使用。
- 父子成环或非法层级会令相应 API 返回错误，不修改场景图。
- SetActiveWorld 后，本帧内 GetCurrentWorld 仍返回旧 WorldRef；下一帧或下一次 UpdateTransforms 后为新 WorldRef。

## 测试

- 单元测试：节点创建、Parent/Children、LocalTransform/WorldTransform、SetDirty/UpdateTransforms、成环拒绝、GetPath/GetId、SetActive。
- 集成测试：多 World、SetActiveWorld 生效时机、Traverse/Find 与迭代器单次有效、LoadLevel/UnloadLevel 与 013-Resource 对接（若已约定）。
