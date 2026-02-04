# US-lifecycle-003：所有内容分配与现成分配均有统一接口

- **标题**：引擎内**所有内容分配**均通过**统一接口**进行；**现成分配**（默认分配器、池、预分配块等）也通过**同一套统一接口**暴露，调用方仅依赖 IAllocator，不依赖具体实现。
- **编号**：US-lifecycle-003

---

## 1. 角色/触发

- **角色**：引擎各模块、资源加载、渲染、实体等
- **触发**：需要分配内存或获取「现成」分配器（默认堆、池、预分配块）时，**统一**通过 **IAllocator** 抽象操作；不按场景拆成多套分配 API，也不直接依赖 DefaultAllocator、PoolAllocator 等具体类型。

---

## 2. 端到端流程

1. **统一分配接口**：引擎内**所有内容分配**（堆块、缓冲、临时内存、池内块等）均通过 **IAllocator** 的 **alloc(size, alignment)**、**free(ptr)**（及可选 **realloc**）进行；**不**提供多套互不兼容的 Alloc/Free、PoolAlloc、StackAlloc 等分散 API，仅通过 **IAllocator** 这一统一抽象。
2. **现成分配也有统一接口**：**现成分配**（默认分配器、线程局部分配器、对象池、预分配块等）也通过**同一套** **IAllocator** 暴露——例如 **getDefaultAllocator()** 返回 **IAllocator***，**getThreadLocalAllocator()** 返回 **IAllocator***，池或预分配块通过 **IAllocator** 实现并对外提供 **IAllocator***；调用方只依赖 **IAllocator**，不关心背后是堆、池还是预分配。
3. 各模块（Resource、Pipeline、Entity 等）在需要分配时，获取 **IAllocator***（默认或注入），统一调用 **alloc**/**free**；便于替换实现、统计与调试。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 001-Core | 定义 **IAllocator** 统一接口（alloc、free、可选 realloc）；提供 **getDefaultAllocator()** 返回 IAllocator*（现成分配）；全局 **alloc**/**free** 可委托给默认 IAllocator；池、线程局部分配器等现成实现均实现 IAllocator 并可通过统一接口获取 |

---

## 4. 每模块职责与 I/O

### 001-Core

- **职责**：提供 **IAllocator** 作为**唯一**分配抽象；**所有内容分配**均通过 IAllocator::alloc / free 进行；**现成分配**（默认分配器、池、预分配等）也通过 **IAllocator*** 暴露——**getDefaultAllocator()**、**getThreadLocalAllocator()**（可选）、池的 **getAllocator()**（可选）等均返回 **IAllocator***；全局 **alloc**/**free** 与默认 IAllocator 行为一致（可委托给 getDefaultAllocator()）。
- **输入**：各模块调用 IAllocator::alloc(size, alignment)、free(ptr)；或 getDefaultAllocator() 等获取现成 IAllocator*。
- **输出**：IAllocator 接口；getDefaultAllocator()；可选 getThreadLocalAllocator()、池/预分配 的 IAllocator 实现；全局 alloc/free 与 IAllocator 语义统一。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写。

### 001-Core

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 001-Core | TenEngine::core | IAllocator | 抽象接口 | **统一**分配接口 | TenEngine/core/Allocator.h | IAllocator::alloc, IAllocator::free | void* alloc(size_t size, size_t alignment); void free(void* ptr); **所有内容分配**均通过此接口；失败返回 nullptr；free(nullptr) 为 no-op |
| 001-Core | TenEngine::core | — | 自由函数/单例 | **现成分配**：获取默认分配器 | TenEngine/core/Allocator.h | getDefaultAllocator | IAllocator* getDefaultAllocator(); 返回默认堆分配器，**统一接口**；调用方不拥有指针 |
| 001-Core | TenEngine::core | — | 自由函数 | 全局分配（委托默认分配器） | TenEngine/core/Allocator.h | alloc, free | void* alloc(size_t size, size_t alignment); void free(void* ptr); 与 getDefaultAllocator()->alloc/free 语义一致，便于无注入时使用；**统一**语义 |
| 001-Core | TenEngine::core | — | 约定 | 现成分配均实现 IAllocator | TenEngine/core/Allocator.h | — | 池、线程局部分配器、预分配块等**现成分配**均实现 IAllocator，并通过 getXxxAllocator() 或类似方式返回 IAllocator*，**统一接口** |

---

## 6. 参考（可选）

- **Unreal**：FMemory、FMalloc、FMemStack 等通过统一抽象分配。
- **Unity**：Unity 内存分配与自定义分配器接口。
- **通用**：所有分配路径通过同一抽象（IAllocator），便于替换、统计与调试。

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/001-core-ABI.md`。*
