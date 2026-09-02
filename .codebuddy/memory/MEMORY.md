# 长期记忆 (MEMORY.md)

> 2026-08-24 重建（用户重置代码库时 memory 被清除）；2026-08-31 全面更新（Adapter 热重载初步验证通过）。

## 用户偏好 / 项目约束

### 跨平台代码约束（重要）
- libil2cpp 下的 C++ 改动必须能在 iOS 与 Android 上编译运行（Windows 只是当前编辑器调试平台）。
- 禁止「仅 Windows 编译的功能性分支」；允许 `#ifdef _MSC_VER` 包 `#pragma warning` 抑制 MSVC 告警。
- 优先标准 C/C++ 保证单一可移植路径。

### 工作方式约束（2026-08-26 定，长期有效）
- **改动限制在 API 接口层**（api-types.h / api-functions.h / api.cpp / adapters.h/.cpp / mono-api.cpp + 编译强制的 icalls/vm 小修）；不动 vm/ 等内部源文件。
- **改动面过大先问再动手**（用户回滚过 MethodInfo 18 文件版教训）。
- 诊断结论与用户观察冲突时，以用户观察为准先还原再重新定位（静态字段迁移误判案例）。

## 项目背景：HybridCLR Assembly 热重载（dev-reload-2022 分支）
- 目标：Player（Windows exe）运行时重载解释器程序集，重载前存活的实例/托管对象继续工作（不容忍 domain reload）。
- **当前架构（2026-08-28 起）：API 边界 Adapter 模式**——重载时 fresh 新建程序集/类，指针稳定性由 Adapter 层承担（替代了旧的 Pass1-3 复用方案）。
- 关键机制认知：
  - Unity 引擎对 Il2CppClass/Il2CppImage/Il2CppType 全是 opaque，只经 il2cpp-api-functions.h（~243 DO_API）+ mono-api 边界访问；仅 2 个直访例外：对象头 2 指针、`unity_user_data` 偏移（运行时 offsetof 查询——已适配为 adapter 槽偏移）。
  - 类型相等实质=比 Il2CppClass* 指针（Il2CppTypeCompare 叶子比 data.typeHandle）。
  - **指针能否 adapter 化的判据：Unity 侧是否把它"当对象用"**（写屏障/反射传参/ToObject）——Il2CppObject 系（Exception 等）不可 adapter（退出崩溃教训）；纯元数据句柄可以。

## Adapter 实验最终版图（2026-08-27 完成，全部构建验证通过）
- **九类完成**：Image/Assembly/FieldInfo/MethodInfo（含 Il2CppStackFrameInfoAdapter 平行结构+walk/profiler 回调 bridge）/PropertyInfo/EventInfo/Il2CppDomain/Il2CppClass（含 userdata 槽+for_each/alloc 回调 bridge）/Il2CppType（**内容键**映射：键=(type,byref,attrs,data裸指针)，双索引 map，RemapTypeReal 换键）。
- **Exception 整体回退**（2026-08-27）：Unity 把异常当托管对象用（ScriptingReferenceWrapper 写屏障→GC_dirty 查页表崩/反射 AddObject 无对象头崩）——退出时偶发崩溃根因，全家族回退真实指针。
- 实现载体：`il2cpp-api-adapters.h/.cpp`——`AdapterMap<RealT,AdapterT>` 模板（Wrap find-or-create/Unwrap/RemapReal/RemapRealsWhere 批量/MarkRealsWhere+stale 标志安全网/ResolveCurrentReal）。
- 推广范式六步：①api-types.h opaque typedef ②adapters.h struct+接口 ③api-functions.h 改签名 ④api.cpp 定义同步（**与③同批成对**——参数不一致=静默 C++ 重载=C 符号丢失运行期 not found）⑤mono-api.cpp 边界同步 ⑥全库搜调用方（目录清单：vm/icalls/hybridclr/mono/codegen/debugger/gc）。
- const 规则：Wrap 入参 const RealT*、返回 AdapterT*（跟随 API 签名）；Unwrap 入参 const AdapterT*、可变需求出口 const_cast。

## 热重载实现（2026-08-28~31，初步测试通过）
- **PlaceHolder 重载路径**（hybridclr/metadata/Assembly.cpp）：token!=0 分支 fresh 创建 newAss/newImage → `MetadataCache::ReplaceInterpreterAssembly`（s_cliAssemblies 原地替换，索引稳定）+ `vm::Assembly::ReplaceAssembly`（版本号自增触发快照重建）→ s_placeHolderAssembies 更新 → RemapAssembly/ImageReal → `OnInterpreterAssemblyReloaded` 协调器。
- **协调器流程**（adapters.cpp）：MarkRealsWhere 打标 → 类按全名批量重绑 → 成员重绑（Field/Property/Event 按名+parent 配对；Method 名字+参数个数粗筛+同名重载按参数类型全名比对；新类惰性 Setup）→ Type 重绑（配对类 byval_arg 内容键切换）→ stale 清单。
- **关键修复（AddComponent null 根因）**：GetNewClassNameIndex 必须**遍历全部类型定义主动实例化**（GetTypeInfoFromTypeDefinitionRawIndex 幂等）——只收已实例化的会导致配对失败→新旧 adapter 分裂→引擎 MonoScript 缓存（key=旧 adapter 地址）查不到→AddComponent 返回 null。
- **BuildClassFullName**：嵌套链走 klass->declaringType（已解析）；名字用 klass->name；仅最外层命名空间读 typeDef（编码索引须 DecodeMetadataIndex）。**教训：_typesDefines 的 declaringTypeIndex 是 byvalTypeIndex 语义，不能当 typeDef 索引**。
- **静态字段迁移已还原待重做**：fresh 新类 static_fields 全零是真实缺口（单例/容器跨重载状态丢失）——主线验证后以独立补丁加回（设计存档 2026-08-28 日志：按名+offset 配对、只迁引用类型 CLASS/STRING/SZARRAY/ARRAY/OBJECT/GENERICINST、WriteBarrier::GenericStore 写入、VALUETYPE/基元/thread-static 跳过）。

## 待办（2026-08-31 收敛，基于用户豁免约定）
- **使用约定（项目级，需写进文档）**：重载后不得使用重载前创建的托管对象（含反射对象、泛型集合、事件订阅、静态缓存）——新代码从自身初始化路径重建全部状态（框架初始化幂等已验证）。
- **已豁免**：①托管堆裸指针 5 处（旧反射对象不用则旧指针不被解引用）；②静态字段迁移（旧静态状态随旧世界退役；设计存档 2026-08-28 日志可取回）；③泛型实例 pass（引擎缓存类全非泛型——MonoBehaviour 不可能泛型；AOT 死条目不命中；**唯一残余=序列化指令缓存嵌的泛型元素类型，触发条件=热更类型布局变化+作 List<T>/数组元素序列化，症状=该字段数据错位，届时再做**）。
- **剩余**：stale 清单日志+泛型实例观测日志（合并一次改动）→ 保活策略（最低优先级）。

## 已知欠账（adapter 够不着的边界）
- 托管堆内裸指针：Il2CppReflectionModule.image / Il2CppReflectionAssembly.assembly / Il2CppReflectionType.type / Il2CppReflectionMethod.method(mhandle) / RuntimeFieldInfo.fieldHandle——icall 写入不经 API 边界（豁免约定下无害）。
- 非默认编译路径已修复（WriteBarrierValidation/debugger 全量同步，2026-08-27）。
