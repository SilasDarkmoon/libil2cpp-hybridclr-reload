#pragma once

// ============================================================================
// il2cpp API 边界 Adapter（实验）
//
// 目的：il2cpp-api-functions.h 导出给 Unity 引擎的指针对 Unity 而言是 opaque
// 的（il2cpp-api-types.h 里只有前向声明，ABI 上就是一个地址）。因此可以在
// API 实现层把真实内部对象替换成地址稳定的 Adapter，而无需修改
// il2cpp-api-functions.h 的声明签名——Unity 侧（含预编译的引擎二进制）零改动。
//
// 映射规则：
//   - 多个 Real（如热重载前后的新旧对象）可映射到同一个 Adapter；
//   - 一个 Adapter 内只保存一个"当前" Real 指针（RemapReal 后指向新对象）；
//   - Adapter 一旦创建不释放（泄漏换稳定），保证地址永久有效。
//
// 注意：Il2CppType 后续不走本机制——它的"指针值=身份"且内存内嵌在
// Il2CppClass.byval_arg / Il2CppGenericInst.type_argv 里，需要按内容键
// find-or-create 的独立映射方案。
// ============================================================================

#include <stddef.h>
#include <unordered_map>

#include "il2cpp-api-types.h"
#include "os/Mutex.h"
#include "utils/Memory.h"

struct Il2CppImage;
struct Il2CppAssembly;
struct FieldInfo;
struct MethodInfo;

// Il2CppImage 的边界适配器。交给 Unity 的 "const Il2CppImage*" 实际上是本结构体的地址。
struct Il2CppImageAdapter
{
    const Il2CppImage* real;
};

// Il2CppAssembly 的边界适配器，语义同 Il2CppImageAdapter。
struct Il2CppAssemblyAdapter
{
    const Il2CppAssembly* real;
};

// FieldInfo 的边界适配器，语义同上。注意 FieldInfo 嵌入在 Il2CppClass::fields
// 数组内按值分配，其地址稳定性依赖类；热重载归并时需按 (parent, 字段名) 配对
// 新旧 FieldInfo（见 RemapFieldReal 注释）。real 为 const 指针以满足模板约定，
// UnwrapField 交出时 const_cast 成可变指针（field API 签名多为非 const FieldInfo*）。
struct FieldInfoAdapter
{
    const FieldInfo* real;
};

// MethodInfo 的边界适配器，语义同上。
struct MethodInfoAdapter
{
    const MethodInfo* real;
};

namespace il2cpp
{
namespace api
{
    // 通用 Real -> Adapter 映射表。
    // 要求 AdapterT 含有 `const RealT* real;` 成员（且语义上只含这一个当前指针）。
    template<typename RealT, typename AdapterT>
    class AdapterMap
    {
    public:
        // 出方向：real -> adapter，find-or-create（real 为 NULL 时返回 NULL）。
        // 同一 real 永远返回同一个 adapter，保证 Unity 侧指针相等性比较成立。
        AdapterT* Wrap(const RealT* real)
        {
            if (real == NULL)
                return NULL;

            il2cpp::os::FastAutoLock lock(&m_Mutex);
            typename RealToAdapterMap::iterator it = m_RealToAdapter.find(real);
            if (it != m_RealToAdapter.end())
                return it->second;

            AdapterT* adapter = (AdapterT*)IL2CPP_CALLOC(1, sizeof(AdapterT));
            adapter->real = real;
            m_RealToAdapter.insert(std::make_pair(real, adapter));
            return adapter;
        }

        // 入方向：adapter -> 当前 real（adapter 为 NULL 时返回 NULL）。
        static const RealT* Unwrap(const AdapterT* adapter)
        {
            return adapter == NULL ? NULL : adapter->real;
        }

        // 热重载后归并：让 oldReal 与 newReal 映射到同一个 adapter（多个 real 对应
        // 一个 adapter），且 adapter 内保存的"当前" real 切换为 newReal。
        // 优先复用 oldReal 的 adapter——Unity 在重载前缓存的就是它。
        void RemapReal(const RealT* oldReal, const RealT* newReal)
        {
            if (oldReal == NULL || newReal == NULL || oldReal == newReal)
                return;

            il2cpp::os::FastAutoLock lock(&m_Mutex);
            typename RealToAdapterMap::iterator oldIt = m_RealToAdapter.find(oldReal);
            typename RealToAdapterMap::iterator newIt = m_RealToAdapter.find(newReal);

            if (oldIt != m_RealToAdapter.end())
            {
                AdapterT* adapter = oldIt->second;
                adapter->real = newReal;
                m_RealToAdapter[newReal] = adapter;
                // 若 newReal 此前已被单独包装过（newIt != end 且 adapter 不同），
                // 那个 adapter 可能已被 Unity 持有，不能释放；其 real 本就是 newReal，
                // 行为仍然正确，只需让后续查找统一归并到 oldReal 的 adapter。
            }
            else if (newIt != m_RealToAdapter.end())
            {
                m_RealToAdapter[oldReal] = newIt->second;
            }
            else
            {
                AdapterT* adapter = (AdapterT*)IL2CPP_CALLOC(1, sizeof(AdapterT));
                adapter->real = newReal;
                m_RealToAdapter[oldReal] = adapter;
                m_RealToAdapter[newReal] = adapter;
            }
        }

    private:
        typedef std::unordered_map<const RealT*, AdapterT*> RealToAdapterMap;
        RealToAdapterMap m_RealToAdapter;
        baselib::ReentrantLock m_Mutex;
    };

    // ---- Il2CppImage 边界接口（供 il2cpp-api.cpp / il2cpp-mono-api.cpp 使用）----
    // 与 il2cpp-api-functions.h 中 image 相关 API 的声明类型保持一致
    // （const Il2CppImageAdapter*），消除 reinterpret_cast 伪装。

    // 出方向：真实 image -> adapter，find-or-create。
    const Il2CppImageAdapter* WrapImage(const Il2CppImage* real);

    // 入方向：边界传回的 adapter -> 当前真实 image。
    const Il2CppImage* UnwrapImage(const Il2CppImageAdapter* adapter);

    // 热重载归并：oldReal/newReal 共用 adapter，adapter 当前 real 切到 newReal。
    void RemapImageReal(const Il2CppImage* oldReal, const Il2CppImage* newReal);

    // ---- Il2CppAssembly 边界接口 ----

    // 出方向：真实 assembly -> adapter，find-or-create。
    const Il2CppAssemblyAdapter* WrapAssembly(const Il2CppAssembly* real);

    // 入方向：边界传回的 adapter -> 当前真实 assembly。
    const Il2CppAssembly* UnwrapAssembly(const Il2CppAssemblyAdapter* adapter);

    // 热重载归并：oldReal/newReal 共用 adapter，adapter 当前 real 切到 newReal。
    void RemapAssemblyReal(const Il2CppAssembly* oldReal, const Il2CppAssembly* newReal);

    // 出方向（数组）：把当前 assembly 列表包装成 adapter 指针数组。
    // 返回的缓冲区归 AdapterMap 所有，下次调用前有效；adapter 本身地址稳定。
    const Il2CppAssemblyAdapter* const* WrapAssemblyArray(const Il2CppAssembly* const* assemblies, size_t count);

    // ---- FieldInfo 边界接口 ----
    // 注意：FieldInfo API 签名多为非 const FieldInfo*，UnwrapField 返回 FieldInfo*。

    // 出方向：真实 FieldInfo -> adapter，find-or-create。
    FieldInfoAdapter* WrapField(FieldInfo* real);

    // 入方向：边界传回的 adapter -> 当前真实 FieldInfo。
    FieldInfo* UnwrapField(const FieldInfoAdapter* adapter);

    // 热重载归并：oldReal/newReal 共用 adapter，adapter 当前 real 切到 newReal。
    // FieldInfo 嵌入类体分配，新旧配对须由重载流程按 (parent 类, 字段名) 完成。
    void RemapFieldReal(FieldInfo* oldReal, FieldInfo* newReal);

    // ---- MethodInfo 边界接口 ----

    // 出方向：真实 MethodInfo -> adapter，find-or-create。
    const MethodInfoAdapter* WrapMethod(const MethodInfo* real);

    // 入方向：边界传回的 adapter -> 当前真实 MethodInfo。
    const MethodInfo* UnwrapMethod(const MethodInfoAdapter* adapter);

    // 热重载归并：oldReal/newReal 共用 adapter，adapter 当前 real 切到 newReal。
    void RemapMethodReal(const MethodInfo* oldReal, const MethodInfo* newReal);

    // ---- 栈帧边界转换 ----
    // 内部（vm/ 等）继续使用 Il2CppStackFrameInfo（method 为真实指针）；
    // 仅 API 出口经本函数转换成 adapter 版再投递给 Unity。

    void WrapStackFrameInfo(const Il2CppStackFrameInfo& from, Il2CppStackFrameInfoAdapter& to);
}
}
