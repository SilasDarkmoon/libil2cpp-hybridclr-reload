# 长期记忆 (MEMORY.md)

## 用户偏好 / 项目约束

### 跨平台代码约束（重要）
- libil2cpp 下的 C++ 改动必须能在 iOS 与 Android 上编译运行（Windows 只是当前编辑器调试平台）。
- 禁止「仅 Windows 编译的功能性分支」；允许 `#ifdef _MSC_VER` 包 `#pragma warning` 抑制 MSVC 告警。
- 优先标准 C/C++（如 `std::gmtime`）保证单一可移植路径。

## vtable 混合布局方案（il2cpp-class-internals.h / vm/Class.cpp）
- `IL2CPP_MAX_VTABLE_SLOT_COUNT = 256`，`IL2CPP_PRESERVED_VTABLE_SLOT_COUNT = 32`；`vtable_count > 224` 走变长 struct 布局（分配 `vtable_count + 32` 槽），否则内联 256 槽。
- `Il2CppClass.vtable_allocated_count` 记录实际分配槽数。溢出日志写控制台 + `vtable_overflow.log`。

## Assembly 重载 Il2CppClass/MethodInfo 复用方案（核心架构）
- 复用数据结构在新 InterpreterImage 上：`_reuseClassMap`（fullName→旧 Il2CppClass*）、`_reuseMethodMap`（签名→旧 MethodInfo*）。
- 采集时机：`Assembly::Create()` 中 `InitRuntimeMetadatas()` 之前，从旧 image 的 `_classList` 采集。
- 还原：`InitRuntimeMetadatas()` 后调 `RestoreReusedClasses()` 分 Pass 处理——Pass 1 更新外部指针/counts/parent；Pass 2 重置懒加载字段与 init 标志；Pass 3 `Class::Init` 重建。
- 方法复用：`SetupMethodsLocked`（Class.cpp）按签名（`ClassFullName:MethodName(Params)->Return`）匹配旧 MethodInfo 原地更新。
- 泛型实例复用：`GenericClass::RestoreCachedGenericClasses`（Pass1/2/3）遍历 `s_GenericClassSet`。
- vtable 超出已分配槽：MSVC 试 `_expand()` 原地扩容，其他平台放弃复用。
- `TypeToSigString` 不触发类加载，直接读 TypeDef nameIndex/namespaceIndex。

## 复用方案关键修复点（按时间，均已合入）
- **ShouldRestoreGenericClass 回退检查（07-28）**：typeDef 来自旧 image 时，经旧 image `GetTypeInfoFromTypeDefinitionRawIndex` 解析 klass，查其 `image` 是否已指向 newImage。
- **s_GenericMethodMap 陈旧 token + methodPointerCallByInterp（07-29）**：`RestoreReusedClasses()` 开头 `GenericMethod::ClearStatics()`；泛型复用路径从 inflated method 继承 `methodPointerCallByInterp`/`initInterpCallMethodPointer`（非置空）；`interpData` 置空强制重新 Transform。背景：`IsImplementedByInterpreter` 不覆盖解释器程序集。
- **klass->image 误设 newImage（07-29）**：Pass 1 中 `defIsInterp` 时应设为泛型定义类的 `defKlass->image`。
- **rehash 三个缓存（07-29/07-30）**：Pass 1 更新 `byval_arg.data.typeHandle` 后，`Il2CppTypeHash` 对 CLASS/VALUETYPE 按 typeHandle 裸指针 hash → 必须 rehash `s_GenericInstSet` + 两个 `s_GenericClassSet`，且 rehash 时**先归一化陈旧指针**（`type_argv[i]`/`gclass->type` 更新为 `&klass->byval_arg`）再重算 hash。
- **class_inst 的 type_argv 也要归一化（08-06）**：AOT 创建的 class_inst 不在 `s_GenericInstSet`，`RehashGenericClassSet`/`RehashGenericTypeSet` 须对 `gclass->context.class_inst` 调 `NormalizeGenericInstTypeArgv`，否则 hash 陈旧 → 重复条目 → `Expression1<T>→Expression<T>` InvalidCastException。
- **MonoBehaviour 序列化字段 NRE 根因（08-21 实锤）+ image 复用修复（08-24 成功）**：**根因**——Unity MonoManager 的 `m_ScriptImages` 启动时经 `il2cpp_domain_assembly_open` 按 image 指针注册一次、重载不刷新；HybridCLR 重载新建 image 使复用类 `klass->image` 变成未知新指针 → `GetAssemblyIndexFromImage` 返回 -1 → `CanTransferTypeAsNestedObject` false → 嵌套 managed 对象字段（如 `UIVariableArray.s_currVariable`）被序列化指令静默剔除 → prefab 反序列化跳过 → NRE（实锤探针 `ImageRegCheck`，il2cpp-api.cpp `il2cpp_class_get_flags`）。
- **最终修复（已验证：不崩 + NRE 消失）= 复用 image 指针 + token 延迟翻转**：①Assembly.cpp 重载路径 `ass=oldAss; image2=oldAss->image`（复用旧程序集/旧 image 指针，使 `klass->image` = MonoManager 注册的旧指针）；只 free `image2->name`（独立分配），不 free `image2->nameNoExt`（指向 raw image 字符串堆）。②**关键配套**：`BuildIl2CppImage` 不再设 `image2->token`（保留旧索引），`Assembly::Create` 在**收集块之后、`InitRuntimeMetadatas` 之前**才设 `image2->token = image->EncodeWithIndex(0)`（新索引）。**原因**：`GetImage(image2)` 按 `image2->token` 单索引路由；太早翻转会令收集阶段 `GetImage(oldImage2)` 路由到新 image（803）而非旧（768），`CollectReusableObjects` 从错误 image 收集、旧类元数据路由全乱 → 崩。延迟翻转后：收集阶段路由到旧 image（768），收集完路由到新 image（803），各代重载均成立。

## Delegate.CreateDelegate "method arguments are incompatible" 根因与修复（08-06，当前方案）
- 异常来自**托管** mscorlib 预检；枚举等值类型参数只走 `Type == Type`（RuntimeType 引用相等），不过 icall。
- `s_TypeMap` 键按内容深比较，但 CLASS/VALUETYPE 只比 `data.typeHandle` 裸指针 → 陈旧 typeHandle 产生第二个 RuntimeType。
- 根因：跨程序集泛型实例（如 `Action\`2<BackpackTabType,BackpackSubTabType>`）在各程序集重载时只归一化部分 type_argv；gclass 若从 class set rehash 漏网，methods 永不重置 → 半陈旧 inflate 参数。
- 修复（`vm/GenericClass.cpp`）：`EnableReloadArgvNormalization()` 重载后启用；`SetupMethods/Fields/Properties/Events` 入口归一化 type_argv；`CreateClass` find 前归一化；`GetClass` 挂 `VerifyGenericInstanceMethodsFreshForReload`——**原地指针修补**直接 VAR 参数（`ReloadExpectedVarType` 返回 type_argv[num]；GENERICINST 等复合类型不校验，Inflate 每次新分配 Il2CppType 会误报），**不重置任何数组**（重置方案在异步链上崩溃）；跳过非解释器引用实例；`s_ReloadVerifiedGenericClasses` 先标记防递归；`Begin/EndReloadRestore` 在 restore Pass 期间静默。
- 成员懒惰重建门：methods/properties/events 按指针门；**fields 按 `size_inited` 标志门**（置 NULL 必须连标志清）。

## 静态字段保留方案——已回滚（08-10）
- 曾改为静态布局不变时保留 `static_fields`+cctor 状态（修 NRE），**用户要求回滚**：Pass 2（InterpreterImage.cpp 与 GenericClass.cpp）恢复**无条件** `static_fields = nullptr` + 重置 cctor 状态。
- 保留：NRE 定位日志（`Interpreter_Execute.cpp` catch 块打印 `[ReloadDiag] NRE at <类>::<方法> ipOffset= token= image=`）。
- NRE 归因存疑：用户认为应是实例字段而非静态字段问题，待日志验证。

## 诊断设施
- `hybridclr/ReloadDiagLog.h`（header-only）：`ReloadDiagLog(fmt,...)` 文件+Unity 日志双写（UTC 时间戳，路径 `$RELOAD_DIAG_LOG_PATH`→`$UNITY_TEMPORARY_CACHE_PATH/reload_diag.log`→CWD）；`ReloadDiagEnabled()` 重载后才启用；`ReloadDiagTryEnter/Leave` thread_local 重入保护。
- 教训：诊断代码里做类解析在 VM 启动期会崩溃；日志前缀 `[ReloadDiag]`，分布在 RuntimeTypeHandle.cpp/Delegate.cpp/Reflection.cpp/GenericClass.cpp/InterpreterImage.cpp/Interpreter_Execute.cpp。
