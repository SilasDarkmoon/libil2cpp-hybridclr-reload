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

## s_GenericMethodMap 缓存陈旧 token + methodPointerCallByInterp 置空修复（2026-07-29）
- **问题**：reload 后泛型实例方法出现 `GetMethodBody OOB` 和 `RaiseAOTGenericMethodNotInstantiatedException`。有两个层面：
  1. `s_GenericMethodMap` 缓存了 reload 前的 inflated `MethodInfo`，其 `token` 来自旧 image。`GenericClass::SetupMethods` 调用 `GenericMetadata::Inflate` 时可能命中旧缓存，返回旧 token → `GetMethodBody(newImage, oldToken)` → OOB
  2. 泛型复用路径中 `reused->methodPointerCallByInterp = nullptr` + `initInterpCallMethodPointer = false`，强制走 `InitAndGetInterpreterDirectlyCallMethodPointerSlow` 重新初始化。但 `IsImplementedByInterpreter` 对解释器程序集返回 false（因为 `AOTHomologousImage::FindImageByAssembly` 找不到解释器程序集），导致重新初始化失败 → `methodPointerCallByInterp` 保持 null → `RaiseAOTGenericMethodNotInstantiatedException`
- **关键背景**：`IsImplementedByInterpreter` 只检查 `AOTHomologousImage::FindImageByAssembly`，不覆盖解释器程序集。解释器方法的 `methodPointerCallByInterp` 在 `CreateMethodLocked` 中通过 `isInterpMethod` 检查设置，不走 `InitAndGetInterpreterDirectlyCallMethodPointerSlow`。非泛型复用路径 (`SetupMethodsLocked`) 正确设置了 `methodPointerCallByInterp = methodPointer` + `initInterpCallMethodPointer = true`。
- **修复**：
  1. `RestoreReusedClasses()` 开头调用 `GenericMethod::ClearStatics()` 清除 `s_GenericMethodMap`，强制重新 inflate
  2. 泛型复用路径从 inflated method 继承 `methodPointerCallByInterp`/`virtualMethodPointerCallByInterp`/`initInterpCallMethodPointer`，而非置空
  3. `interpData` 仍置空以强制重新 Transform
- **诊断日志**：
  - `GetMethodBody` OOB 日志增加 `imageName` 字段
  - `HiTransform::Transform` 在 methodBody 为 null 时打印 klass/method/token/image/isInterpType/isInterpImpl/is_inflated/is_generic
  - `GenericClass::SetupMethods` 在 `inflated->token != methodDefinition->token` 时打印 TokenMismatch 日志（复用和非复用路径都有）

## typeHierarchy 在 Pass 2 重置、Pass 3 通过 Class::Init 重建（2026-07-29）
- **问题**：Pass 2 重置 `typeHierarchy = nullptr` 和 `typeHierarchyDepth = 0` 后，`IsInst` → `IsAssignableFrom` → `Class::Init` → `SetupTypeHierarchyLocked` 重建继承树
- **关键发现**：`HasParentUnsafe` 使用 `typeHierarchy` 和 `typeHierarchyDepth`，`IsAssignableFrom` 调用 `Class::Init` 确保重建
- **泛型 Pass 3 重新解析 parent**：从 generic type definition 的 parent inflate，调用 `Class::FromIl2ptype`
- **Pass 1 后 rehash 三个缓存**：`s_GenericInstSet`、`GenericMetadata::s_GenericClassSet`、`GenericClass::s_GenericClassSet`
  - 原因：Pass 1 更新 `byval_arg.data.typeHandle`，`Il2CppTypeHash::Hash` 对 CLASS/VALUETYPE 用 `typeHandle` 指针值做 hash。旧条目 hash 失效，新 lookup hash 不同 → 创建新类 → `ParentMismatch` → `InvalidCastException`
  - 修复：Pass 1 后清除并重新插入所有条目，用新 `typeHandle` 重新计算 hash

## klass->image 被错误设为 newImage（2026-07-29）
- **问题**：`ShouldRestoreGenericClass` 递归检查泛型参数。如果泛型参数来自正在 reload 的 DLL，返回 true。但 Pass 1 `klass->image = newImage` 把 image 设成了当前 reload 的 DLL，而非泛型定义所在的 DLL。导致 `GetMethodBody` 在错误 image 上用 token 查找 → OOB
- **修复**：Pass 1 中，当 `defIsInterp` 为 true 时，把 `klass->image` 设为泛型定义类的 `image`（`defKlass->image`），而非 `newImage`

## rehash 仅重算 hash 但未更新陈旧指针（2026-07-30）
- **问题**：Pass 1 更新 `klass->byval_arg.data.typeHandle` 指向新 `Il2CppTypeDefinition`，但 `Il2CppGenericClass->type` 和 `Il2CppGenericInst->type_argv[i]` 可能指向旧 image 类型表中的 `Il2CppType` 对象（其 `data.typeHandle` 未被 Pass 1 更新）。rehash 仅重算 hash，但用的仍是旧 `typeHandle` → 新 lookup 用新 `typeHandle` → hash 不匹配 → 创建重复条目 → `ParentMismatch`
- **根因分析**：
  - `Il2CppTypeHash::Hash` 对 CLASS/VALUETYPE 用 `data.typeHandle` 指针值
  - `Il2CppGenericInstHash` 调 `Il2CppTypeHash::Hash(type_argv[i])`
  - `Il2CppGenericClassHash` 调 `Il2CppTypeHash::Hash(item->type)` + `Il2CppGenericContextHash::Hash(&item->context)`
  - 如果 `type_argv[i]` 或 `gclass->type` 指向旧 image 类型表条目，其 `typeHandle` 是旧值
  - rehash 重算 hash 用旧值；新 lookup（从 `&klass->byval_arg`）用新值 → 不匹配
- **修复**：在三个 rehash 函数中，遍历条目时更新陈旧指针：
  1. `RehashGenericInstSet`（MetadataCache.cpp）：对每个 `Il2CppGenericInst`，遍历 `type_argv[i]`，若 CLASS/VALUETYPE 且 `klass->byval_arg.data.typeHandle != t->data.typeHandle`，则更新 `type_argv[i] = &klass->byval_arg`
  2. `RehashGenericClassSet`（GenericMetadata.cpp）：对每个 `Il2CppGenericClass`，若 `gclass->type` 是 CLASS/VALUETYPE 且类被复用，更新 `gclass->type = &klass->byval_arg`
  3. `RehashGenericTypeSet`（GenericClass.cpp）：同上
  - 查找类用 `MetadataCache::GetTypeInfoFromType(defType)` → `GlobalMetadata::GetTypeInfoFromType` → `GetTypeInfoFromHandle(typeHandle)` → `GetIndexForTypeDefinitionInternal` → `GetTypeInfoFromTypeDefinitionIndex`
  - 即使 `typeHandle` 是旧值，`IsInterpreterType` 仍能识别，`GetTypeEncodeIndex` 仍能返回编码索引，最终返回被复用的 `Il2CppClass*`（其 `byval_arg.data.typeHandle` 已是新值）
