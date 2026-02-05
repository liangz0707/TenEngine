# US-lifecycle-004：全部矩阵计算、数学计算、数学运算工具、四元数、矩阵空间变换均有统一数学库并支持全局调用

- **标题**：引擎内**全部**矩阵计算、数学计算、数学运算工具、四元数、矩阵空间变换均来自**统一数学库**，通过**统一命名空间**（如 TenEngine::math）**全局调用**；不按模块或功能拆成多套数学 API。
- **编号**：US-lifecycle-004

---

## 1. 角色/触发

- **角色**：渲染、物理、动画、实体、编辑器等所有需要数学运算的模块
- **触发**：需要做矩阵运算、向量运算、四元数、空间变换、通用数学工具（clamp、lerp、角度弧度转换等）时，**统一**从 **TenEngine::math**（或等价统一命名空间）调用；**不**使用分散的 math_xxx、各模块自带的矩阵实现等多套接口。

---

## 2. 端到端流程

1. **统一数学库**：引擎提供**唯一**数学库，包含：
   - **矩阵**：Matrix3、Matrix4 等；乘法、逆、转置、单位矩阵等。
   - **向量**：Vector2、Vector3、Vector4；点积、叉积、归一化、长度等。
   - **四元数**：Quaternion（或 Quat）；slerp、fromEuler、toMatrix、乘法等。
   - **矩阵空间变换**：平移、旋转、缩放、LookAt、透视/正交投影等；局部到世界、世界到视口等组合变换。
   - **数学运算工具**：clamp、lerp、min/max、deg2rad、rad2deg、sqrt、sin/cos 等通用工具。
2. **全局调用**：上述能力均通过 **TenEngine::math** 命名空间（或单一头文件集）暴露，**全局可调用**——任意模块 include 统一头文件（如 TenEngine/core/Math.h 或 TenEngine/math/Matrix.h、Vector.h、Quat.h、Transform.h、MathUtils.h）后即可使用，无需额外初始化或获取句柄。
3. **不分散**：不提供多套互不兼容的数学实现（如渲染用一套矩阵、物理用另一套）；所有矩阵计算、四元数、空间变换均出自该统一库，保证一致性（左手/右手系、行列序等由库统一约定）。

---

## 3. 涉及模块

| 模块 | 职责摘要 |
|------|----------|
| 001-Core | 提供**统一数学库**：TenEngine::math 命名空间；Matrix3/4、Vector2/3/4、Quaternion；矩阵运算、四元数运算、空间变换（平移/旋转/缩放、LookAt、投影）；数学工具（clamp、lerp、deg2rad 等）；全局可调用（include 即用） |

---

## 4. 每模块职责与 I/O

### 001-Core

- **职责**：实现并暴露**统一数学库**，包含矩阵类型与运算、向量、四元数、矩阵空间变换、通用数学工具；所有符号置于 **TenEngine::math**（或 TenEngine::core::math）；通过统一头文件（如 Math.h、Matrix.h、Vector.h、Quaternion.h、Transform.h、MathUtils.h）暴露，**全局可调用**，无单例或显式初始化；左手/右手系、行列序等由库文档统一约定。
- **输入**：各模块 include 数学头文件后直接使用 TenEngine::math::*。
- **输出**：Matrix3、Matrix4、Vector2、Vector3、Vector4、Quaternion；identity、inverse、transpose、multiply；slerp、fromEuler、toMatrix；translate、rotate、scale、lookAt、perspective、ortho；clamp、lerp、deg2rad、rad2deg 等；全部在 TenEngine::math 下全局可调用。

---

## 5. 派生接口（ABI 条目）

以下按 `docs/engine-abi-interface-generation-spec.md` 书写；仅列代表符号，完整 API 以实现为准。

### 001-Core

| 模块名 | 命名空间 | 类名 | 导出形式 | 接口说明 | 头文件 | 符号 | 说明 |
|--------|----------|------|----------|----------|--------|------|------|
| 001-Core | TenEngine::math | — | struct | 4x4 矩阵 | TenEngine/core/Math/Matrix.h | Matrix4 | 行/列序由库统一约定；identity()、inverse()、transpose()、multiply() 等；**统一库全局调用** |
| 001-Core | TenEngine::math | — | struct | 3D 向量 | TenEngine/core/Math/Vector.h | Vector3 | dot、cross、normalize、length 等；**统一库全局调用** |
| 001-Core | TenEngine::math | — | struct | 四元数 | TenEngine/core/Math/Quaternion.h | Quaternion | slerp、fromEuler、toMatrix、multiply 等；**统一库全局调用** |
| 001-Core | TenEngine::math | — | 自由函数 | 空间变换：平移/旋转/缩放 | TenEngine/core/Math/Transform.h | translate, rotate, scale | Matrix4 translate(Vector3 const&); Matrix4 rotate(Quaternion const&); Matrix4 scale(Vector3 const&); 等；**统一库全局调用** |
| 001-Core | TenEngine::math | — | 自由函数 | 空间变换：视点/投影 | TenEngine/core/Math/Transform.h | lookAt, perspective, ortho | Matrix4 lookAt(...); Matrix4 perspective(...); Matrix4 ortho(...); **统一库全局调用** |
| 001-Core | TenEngine::math | — | 自由函数 | 数学运算工具 | TenEngine/core/Math/MathUtils.h | clamp, lerp, deg2Rad, rad2Deg | 通用工具；**统一库全局调用** |
| 001-Core | TenEngine::core | — | 约定 | 数学统一库与全局调用 | TenEngine/core/Math/ | — | **全部**矩阵计算、数学运算、四元数、矩阵空间变换均来自 TenEngine::math，**统一库**，**全局可调用**（include 即用）；不提供多套分散数学 API |

---

## 6. 参考（可选）

- **Unity**：Unity.Mathematics（float3, float4x4, quaternion, math::lerp 等）；单一命名空间全局调用。
- **Unreal**：FVector、FMatrix、FQuat、FTransform；FMath 工具函数；统一在 Engine 数学模块。
- **通用**：单一数学库、统一命名空间、全局可调用，避免多套实现导致的不一致与重复。

---

*本故事派生出的 ABI 条目将同步到 `specs/_contracts/001-core-ABI.md`。*
