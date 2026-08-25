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

#include "os/Mutex.h"
#include "utils/Memory.h"

struct Il2CppImage;

// Il2CppImage 的边界适配器。交给 Unity 的 "const Il2CppImage*" 实际上是本结构体的地址。
struct Il2CppImageAdapter
{
    const Il2CppImage* real;
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

    // 出方向：真实 image -> adapter，以 API 声明类型（const Il2CppImage*）返回。
    const Il2CppImage* WrapImage(const Il2CppImage* real);

    // 入方向：边界传回的 "image"（实为 adapter）-> 真实 image。
    const Il2CppImage* UnwrapImage(const Il2CppImage* adapterLike);

    // 热重载归并：oldReal/newReal 共用 adapter，adapter 当前 real 切到 newReal。
    void RemapImageReal(const Il2CppImage* oldReal, const Il2CppImage* newReal);
}
}
