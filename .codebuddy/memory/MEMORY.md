# 长期记忆 (MEMORY.md)

> 2026-08-24 重建：用户重置代码库时 .codebuddy/memory 被一并清除，此文件从会话上下文重建，只保留长期价值条目。

## 用户偏好 / 项目约束

### 跨平台代码约束（重要）
- libil2cpp 下的 C++ 改动必须能在 iOS 与 Android 上编译运行（Windows 只是当前编辑器调试平台）。
- 禁止「仅 Windows 编译的功能性分支」；允许 `#ifdef _MSC_VER` 包 `#pragma warning` 抑制 MSVC 告警。
- 优先标准 C/C++ 保证单一可移植路径。

## 项目背景：HybridCLR Assembly 热重载（dev-reload-2022 分支）
- 目标：Player（Windows exe）运行时重载解释器程序集，重载前存活的实例/托管对象必须继续用新代码工作（不能容忍 domain reload）。
- 核心架构：重载时**复用** Il2CppClass/MethodInfo/Il2CppImage 等指针（Pass1-3 恢复流程），保证三层指针身份唯一（Unity 引擎层/il2cpp API 层/托管层）。
- 关键机制认知：
  - Unity 引擎对 Il2CppClass/Il2CppImage/Il2CppType 全是 opaque，只经 il2cpp-api-functions.h（~243 个 DO_API）+ scripting_* 层访问；仅 2 个直访例外：对象头 2 指针（读 m_CachedPtr）、`unity_user_data` 偏移（运行时 offsetof 查询）。
  - 类型相等实质=比 Il2CppClass* 指针：Il2CppTypeCompare 对 CLASS/VALUETYPE 只比 data.typeHandle 裸指针（GENERICINST 递归后叶子同）。
  - RuntimeType 的 native 布局=Il2CppReflectionType{object; const Il2CppType* type;}；Unity 经 il2cpp_class_from_system_type API 取，托管堆内该指针由 icall 读写。
- 曾实锤的根因（2026-08-21）：重载新建 Il2CppImage → MonoManager m_ScriptImages（启动时缓存，不刷新）指针查找失败 → CanTransferTypeAsNestedObject=false → 嵌套 managed 字段被踢出序列化指令 → 反序列化丢字段 NRE。修复=重载路径复用旧 Il2CppAssembly/Il2CppImage 指针。

## Adapter 方案评估（2026-08-21，三轮，结论：不做全量 Adapter）
- 用户提议 il2cpp-api-functions.h 边界包 Il2CppClassAdapter/Il2CppTypeAdapter（TypeAdapter 持 ClassAdapter*，入边界解出真实 klass 填 typeHandle）。
- 结论：边界层自洽，但只覆盖 4 类指针逃逸边界的 1 个（引擎 API）；覆盖不了 ①obj->klass（GC/AOT/解释器编译期偏移直解引用）②托管反射句柄（RuntimeType.value/RuntimeMethodInfo.mhandle，icall 读写不经 API）③泛型缓存+AOT 游离 class_inst（typeHandle 裸指针 hash，枚举不全）。adapter 能替代的工作量≈10-20%。
- 2026-08-24：用户决定仍做**实验**，代码库已重置为全新状态；选型结论=Il2CppImage 是最佳入手点（见当日日志）。

## Adapter 实验进展（2026-08-26 更新）
- **已完成并验证通过**：Il2CppImage、Il2CppAssembly、FieldInfo（强类型版：api-functions.h 签名直接声明 Adapter*，extern "C" 按名解析故 ABI 不变；实现见 il2cpp-api-adapters.h/.cpp 的 `AdapterMap<RealT,AdapterT>` 模板：Wrap find-or-create / Unwrap / RemapReal 多 Real→一 Adapter、adapter 只持当前 real、泄漏换稳定）。
- **推广范式六步**：①api-types.h 加 opaque typedef ②adapters.h 加 struct+接口 ③api-functions.h 改签名 ④api.cpp 定义同步+包/解 ⑤mono-api.cpp 边界同步（注意直调 vm:: 和直解引用结构体字段的点）⑥全库搜调用方。
- **MethodInfo（API 层收敛版，2026-08-26 实施）**：首版改 Il2CppStackFrameInfo.method 字段类型波及 vm/hybridclr/codegen 等 18 文件**被用户回滚**。新方案（用户指定）：**平行结构**——内部继续用原 Il2CppStackFrameInfo（method=真实指针），API 出口转换成 Il2CppStackFrameInfoAdapter（布局镜像，method=MethodInfoAdapter*）；walk/profiler 回调用新 typedef（Il2CppFrameWalkFuncAdapter/Il2CppProfileMethodFuncAdapter），**旧 typedef 不动**（vm 头文件在用）；trampoline/bridge 全在 api.cpp。**新约束（长期）：改动限制在 API 接口层，不动 vm/ 等内部源文件；改动面过大先问再动手**。
- **路线图**：MethodInfo（进行中）→ Il2CppType/Il2CppClass（内容键，机制不同）。
- **已知欠账（adapter 够不着的边界，同类记录）**：托管堆内裸指针——Il2CppReflectionModule 持 Il2CppImage*、Il2CppReflectionAssembly 持 Il2CppAssembly*、Il2CppReflectionType 持 Il2CppType*、Il2CppReflectionMethod 持 MethodInfo*；gc/WriteBarrierValidation（默认关宏）与 debugger/（MONO_DEBUGGER 构建）两处非默认编译路径待同步。
