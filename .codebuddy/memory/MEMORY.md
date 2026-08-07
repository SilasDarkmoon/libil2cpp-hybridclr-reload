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

## rehash 仅重算 hash 但未更新陈旧指针（2026-07-30，已修复）
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

## rehash 未更新 gclass->context.class_inst->type_argv 导致 Expression1<T>→Expression<T> InvalidCastException（2026-08-06，已修复）
- **问题**：`RehashGenericClassSet` 和 `RehashGenericTypeSet` 更新了 `gclass->type` 但未更新 `gclass->context.class_inst->type_argv`。`Il2CppGenericClassHash` 的 hash 同时依赖 `gclass->type` 和 `gclass->context.class_inst->type_argv`。rehash 用陈旧 `type_argv` 重算 hash；之后 Pass 2 归一化 `type_argv` 但不 rehash → hash 再次陈旧 → Pass 3 lookup 用新 `type_argv` → hash 不匹配 → 创建重复 `Expression<T>` 条目 → `Expression1<T>->parent` 指向旧 `Expression<T>`，cast 目标是新 `Expression<T>` → `HasParentUnsafe` 失败 → `InvalidCastException: Unable to cast Expression1<T> to Expression<T>`
- **关键**：AOT 创建的 `class_inst`（如 `Expression1<T>` 的 `class_inst`）不在 `s_GenericInstSet` 中，所以 `RehashGenericInstSet` 不会更新它的 `type_argv`。Pass 2 只更新 `s_GenericClassesToRestore` 中的类。如果 rehash 在 Pass 2 之前运行，rehash 用的 `type_argv` 是陈旧的。
- **修复**：在 `RehashGenericClassSet`（GenericMetadata.cpp）和 `RehashGenericTypeSet`（GenericClass.cpp）中，增加 `NormalizeGenericInstTypeArgv(gclass->context.class_inst)` 调用，在 rehash 时同时归一化 `type_argv`，确保 hash 用归一化后的 `type_argv` 计算。

## Delegate.CreateDelegate "method arguments are incompatible" 的关键机制（2026-08-06）
- 异常来自**托管** mscorlib `Delegate.CreateDelegate(Type, object, MethodInfo, bool, bool)` 的预检；原生 icall `CreateDelegate_internal` 不做校验、不会失败。
- 预检顺序：返回类型匹配 → closed-instance 检查（`method.DeclaringType.IsAssignableFrom/IsInstanceOfType(target)`）→ 参数个数 → 参数类型 `delArgType == argType`（仅两者都是引用类型时才回退 IsAssignableFrom）。
- **枚举等值类型参数只走 `Type == Type`（RuntimeType 引用相等），不过任何 icall** —— 排查盲区。
- `s_TypeMap`（Reflection.cpp）键是 `Il2CppType*` 但按内容深比较（`Il2CppTypeHash`/`Il2CppTypeEqualityComparer`）；CLASS/VALUETYPE 只比 `data.typeHandle` 裸指针。typeHandle 陈旧（旧 image typeDef）即产生第二个 RuntimeType → `==` 失败。
- `g_MetadataLock` 是 `baselib::ReentrantLock`（可重入），GetTypeObject 内调 `Class::FromIl2CppType` 安全。
- 注意：早前日志记录过的 `ClearTypeMapForReload` 修复在当前 dev-reload-2022 工作区不存在（疑似清理日志时被还原）。

## 泛型实例逃逸 reload pass 导致签名陈旧 + 三层修复（2026-08-06 深夜，Delegate 绑定失败根因）
- **根因**：跨程序集泛型实例（如 `Action\`2<BackpackTabType, BackpackSubTabType>`）在每个程序集重载时只归一化属于该程序集的 type_argv；`RehashGenericInstSet` 归一化共享 inst 但不重置 methods。若 gclass 从 `s_GenericClassSet`（vm/metadata 两个）漏网（rehash 重插入去重挤出），其 `cached_class->methods` 永不被重置 → 保留半陈旧 inflate 参数 → 反射 `Type == Type` 失败 → `Delegate.CreateDelegate` 抛 "method arguments are incompatible"。
- **修复**（`vm/GenericClass.cpp`，`EnableReloadArgvNormalization()` 在 `Assembly::Create` 的 `RestoreReusedClasses()` 之后调用，未重载时零开销）：
  1. `SetupMethods/Fields/Properties/Events` 入口先 `NormalizeGenericInstTypeArgv(context.class_inst)`——inflate 永远用当前 type_argv；
  2. `CreateClass` 缓存 `find` 前归一化 `gclass->type` 与 type_argv——陈旧参数的查找命中已有条目，不再制造重复实例类；
  3. `GetClass` 快速路径 `VerifyGenericInstanceMethodsFreshForReload`：每次重载后对每个泛型实例类验证一次——当前 context 重新 `Inflate` 并与现有 `parameters`/`return_type` 指针对比，不一致则重置 methods/fields/properties/events（惰性重新 inflate）。
- **诊断设施**：`hybridclr/ReloadDiagLog.h`（header-only，文件+Unity 日志回调双写，UTC 时间戳，路径 `$RELOAD_DIAG_LOG_PATH`→`$UNITY_TEMPORARY_CACHE_PATH/reload_diag.log`→CWD）；`ReloadDiagEnabled()` 开关（重载后才启用，防启动期类解析崩溃）；`ReloadDiagTryEnter/Leave` 重入保护（防诊断内类解析递归）。诊断日志前缀 `[ReloadDiag]`，分布在 RuntimeTypeHandle.cpp/Delegate.cpp/Reflection.cpp/GenericClass.cpp/InterpreterImage.cpp。
- **教训**：诊断代码里做类解析（GetTypeInfoFromType/FromIl2CppType）在 VM 启动期（CreateClass 等路径）会崩溃；`s_TypeMap` 键按内容比较但 CLASS/VALUETYPE 只比 typeHandle 裸指针；托管 `Delegate.CreateDelegate` 对值类型参数只走 `Type == Type` 引用相等。
