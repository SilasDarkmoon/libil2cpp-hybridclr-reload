# 长期记忆 (MEMORY.md)

## 用户偏好 / 项目约束

### 跨平台代码约束（重要）
- 用户明确要求：**libil2cpp 下的 C++ 改动必须能在 iOS 与 Android 上编译运行**（Windows 只是当前编辑器调试平台）。
- 禁止出现「仅在 Windows 才编译的功能性分支」（如 `#ifdef _WIN32` 包裹真正的行为差异代码）。
- 允许的例外：仅用于抑制 MSVC 弃用告警的 `#pragma warning(...)`，且需包在 `#ifdef _MSC_VER` 内（GCC/Clang 会忽略，对 iOS/Android 无副作用）。
- 优先使用标准 C/C++（如 `std::gmtime` 而非 `gmtime_s`/`gmtime_r` 分流）以保证单一可移植路径。

## vtable 混合布局方案（il2cpp-class-internals.h / vm/Class.cpp 等）
- `IL2CPP_MAX_VTABLE_SLOT_COUNT = 256`，`IL2CPP_PRESERVED_VTABLE_SLOT_COUNT = 32`。
- `vtable_count + 32 > 256`（即 >224）时走变长 struct 布局，分配槽数 = `vtable_count + 32`；否则内联 256 槽。
- `Il2CppClass` 有 `vtable_allocated_count` 字段记录实际分配槽数，写/读边界据此判断。
- 溢出类型日志写控制台 + `vtable_overflow.log`（UNITY_TEMPORARY_CACHE_PATH 或 `log/`），时间戳为 UTC `%Y-%m-%d %H:%M:%S`。

## Assembly 重载 Il2CppClass/MethodInfo 复用方案
- 复用数据结构存放在新 InterpreterImage 上：`_reuseClassMap`（fullName→旧 Il2CppClass*）、`_reuseMethodMap`（签名→旧 MethodInfo*）。
- 采集时机：`Assembly::Create()` 中 `InitRuntimeMetadatas()` 之前，从旧 InterpreterImage 的 `_classList` 采集。
- 还原时机：`InitRuntimeMetadatas()` 之后、程序集注册之前，调 `RestoreReusedClasses()` 批量更新旧 Il2CppClass 的外部指针（image/typeMetadataHandle/byval_arg 等）并置空懒加载字段。
- 方法复用：`SetupMethodsLocked`（Class.cpp）中对 Interpreter 类型按方法签名（`ClassFullName:MethodName(ParamType1,...)->ReturnType`）匹配旧 MethodInfo，命中则原地更新字段。
- 泛型实例复用：`GenericClass::RestoreCachedGenericClasses` 遍历 `s_GenericClassSet`，更新 `cached_class->image` 指向新 image 并重置懒加载字段。
- vtable 变化处理：new vtable_count ≤ vtable_allocated_count → 正常复用；超出 → MSVC 上试 `_expand()` 原地扩容，其他平台放弃复用并写日志。
- 类型签名构建（`TypeToSigString`）：不触发类加载，直接读 TypeDef 的 nameIndex/namespaceIndex。

## ShouldRestoreGenericClass 修复（2026-07-28）
- **问题**：`RestoreReusedClasses` 更新了泛型定义类的 `klass->image` 指向新 image，但 `gclass->type->data.typeHandle` 仍指向旧 image 的 `Il2CppTypeDefinition`。导致 `ShouldRestoreGenericClass` 无法识别这些泛型实例类需要恢复 → 泛型实例类的 `klass->image` 未更新 → `GetUnderlyingInterpreterImage` 返回旧 image 但 method token 来自新 image → `GetMethodBody` 在旧 image 上用新 token 查找 → 读到错误方法体 → 崩溃。
- **修复**：`ShouldRestoreGenericClass` 和 `ShouldRestoreType` 增加回退检查：当 `typeDef` 来自旧 image 时，通过旧 image 的 `GetTypeInfoFromTypeDefinitionRawIndex` 解析 `Il2CppClass*`，检查其 `image` 是否已被更新为 `newImage`。
