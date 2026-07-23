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
