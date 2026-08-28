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
struct Il2CppDomain;
struct Il2CppException;
struct Il2CppClass;
struct Il2CppType;
struct FieldInfo;
struct MethodInfo;
struct PropertyInfo;
struct EventInfo;

// Il2CppImage 的边界适配器。交给 Unity 的 "const Il2CppImage*" 实际上是本结构体的地址。
struct Il2CppImageAdapter
{
    const Il2CppImage* real;
    bool stale; // 热重载 old 标志：real 属旧版本且尚未配对成功（安全网，见 AdapterMap）
};

// Il2CppAssembly 的边界适配器，语义同 Il2CppImageAdapter。
struct Il2CppAssemblyAdapter
{
    const Il2CppAssembly* real;
    bool stale;
};

// FieldInfo 的边界适配器，语义同上。注意 FieldInfo 嵌入在 Il2CppClass::fields
// 数组内按值分配，其地址稳定性依赖类；热重载归并时需按 (parent, 字段名) 配对
// 新旧 FieldInfo（见 RemapFieldReal 注释）。real 为 const 指针以满足模板约定，
// UnwrapField 交出时 const_cast 成可变指针（field API 签名多为非 const FieldInfo*）。
struct FieldInfoAdapter
{
    const FieldInfo* real;
    bool stale;
};

// MethodInfo 的边界适配器，语义同上。
struct MethodInfoAdapter
{
    const MethodInfo* real;
    bool stale;
};

// PropertyInfo / EventInfo 的边界适配器，语义同 FieldInfoAdapter（嵌入类体分配，
// 热重载归并需按 (parent 类, 成员名) 配对）。
struct PropertyInfoAdapter
{
    const PropertyInfo* real;
    bool stale;
};

struct EventInfoAdapter
{
    const EventInfo* real;
    bool stale;
};

// Il2CppType 的边界适配器。与指针键类型不同：Il2CppType 按值嵌入（byval_arg、
// type_argv、方法签名等）无独立身份，映射按内容键 (type, byref, attrs, data)
// find-or-create——同一逻辑类型永远返回同一 adapter。热重载后经 RemapTypeReal
// 把 real 从旧实例（如 &oldKlass->byval_arg）切到新实例。
struct Il2CppTypeAdapter
{
    const Il2CppType* real;
};

// Il2CppDomain 的边界适配器，语义同 Il2CppImageAdapter。
// 注意：Il2CppException 不做 Adapter 化——Unity 把异常当托管对象使用
// （写屏障/反射/ToObject），adapter 无对象头会崩（2026-08-27 回退）。
struct Il2CppDomainAdapter
{
    const Il2CppDomain* real;
    bool stale;
};

// Il2CppClass 的边界适配器。除 real 外含 userdata 槽：Unity 经
// il2cpp_class_get_userdata_offset() 取偏移后按偏移直读直写（GetComponent 优化），
// 该槽存 Unity::Type*（引擎侧对象），热重载 RemapReal 换 real 时槽保留不动。
struct Il2CppClassAdapter
{
    const Il2CppClass* real;
    void* userdata;
    bool stale;
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
            RemapRealLocked(oldReal, newReal);
        }

        // 热重载批量重绑（单次持锁）：遍历所有 real 满足 filter 的条目，调
        // pairFn(oldReal) 求配对结果——非 NULL 则重绑到该 newReal（内部走
        // RemapRealLocked 语义），NULL 则跳过。返回成功重绑条数。
        // userData 透传给两个回调；回调不得再进入本 map 的锁。
        typedef bool(*RealFilterFn)(const RealT* real, void* userData);
        typedef const RealT*(*RealPairFn)(const RealT* oldReal, void* userData);
        size_t RemapRealsWhere(RealFilterFn filter, RealPairFn pairFn, void* userData)
        {
            size_t rebound = 0;
            il2cpp::os::FastAutoLock lock(&m_Mutex);
            // 收集待处理项：回调期间修改 map 会失效迭代器
            std::vector<const RealT*> pending;
            for (typename RealToAdapterMap::iterator it = m_RealToAdapter.begin(); it != m_RealToAdapter.end(); ++it)
            {
                if (it->second->real == it->first && filter(it->first, userData))
                    pending.push_back(it->first);
            }
            for (size_t i = 0; i < pending.size(); i++)
            {
                const RealT* newReal = pairFn(pending[i], userData);
                if (newReal != NULL)
                {
                    RemapRealLocked(pending[i], newReal);
                    rebound++;
                }
            }
            return rebound;
        }

        // old 标志安全网：pass 开始前给属于旧 image 的 adapter 打标（MarkRealsWhere），
        // 配对成功即清除；pass 后仍带标的 adapter 的 real 指向保活的旧对象（新版本中
        // 已删除的类型），GetStaleReals 输出清单供日志/降级。
        void MarkRealsWhere(RealFilterFn filter, void* userData)
        {
            il2cpp::os::FastAutoLock lock(&m_Mutex);
            for (typename RealToAdapterMap::iterator it = m_RealToAdapter.begin(); it != m_RealToAdapter.end(); ++it)
            {
                if (it->second->real == it->first && filter(it->first, userData))
                    it->second->stale = true;
            }
        }

        void ClearStaleMark(const RealT* real)
        {
            il2cpp::os::FastAutoLock lock(&m_Mutex);
            typename RealToAdapterMap::iterator it = m_RealToAdapter.find(real);
            if (it != m_RealToAdapter.end())
                it->second->stale = false;
        }

        void GetStaleReals(std::vector<const RealT*>& out) const
        {
            il2cpp::os::FastAutoLock lock(const_cast<baselib::ReentrantLock*>(&m_Mutex));
            for (typename RealToAdapterMap::const_iterator it = m_RealToAdapter.begin(); it != m_RealToAdapter.end(); ++it)
            {
                if (it->second->stale)
                    out.push_back(it->first);
            }
        }

    private:
        // 调用方须已持有 m_Mutex
        void RemapRealLocked(const RealT* oldReal, const RealT* newReal)
        {
            typename RealToAdapterMap::iterator oldIt = m_RealToAdapter.find(oldReal);
            typename RealToAdapterMap::iterator newIt = m_RealToAdapter.find(newReal);

            if (oldIt != m_RealToAdapter.end())
            {
                AdapterT* adapter = oldIt->second;
                adapter->real = newReal;
                adapter->stale = false; // 重绑成功清除 old 标志
                m_RealToAdapter[newReal] = adapter;
                // 若 newReal 此前已被单独包装过（newIt != end 且 adapter 不同），
                // 那个 adapter 可能已被 Unity 持有，不能释放；其 real 本就是 newReal，
                // 行为仍然正确，只需让后续查找统一归并到 oldReal 的 adapter。
            }
            else if (newIt != m_RealToAdapter.end())
            {
                newIt->second->stale = false;
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

    // ---- 热重载协调器 ----
    // 程序集 fresh 载入并原地替换注册表后调用（hybridclr Assembly::Create 内）。
    // 职责：按全名配对旧/新 image 的类型，批量重绑 Class/Field/Method/Property/
    // Event/Type 的 adapter；old 标志校验输出未配对清单。泛型实例 pass 由
    // ReloadGenericInstances 单独触发（后续接入）。
    void OnInterpreterAssemblyReloaded(const Il2CppAssembly* oldAssembly, const Il2CppAssembly* newAssembly);

    // ---- PropertyInfo / EventInfo 边界接口 ----

    // 出方向：真实成员 -> adapter，find-or-create。
    PropertyInfoAdapter* WrapProperty(const PropertyInfo* real);
    EventInfoAdapter* WrapEvent(const EventInfo* real);

    // 入方向：边界传回的 adapter -> 当前真实成员。
    PropertyInfo* UnwrapProperty(const PropertyInfoAdapter* adapter);
    EventInfo* UnwrapEvent(const EventInfoAdapter* adapter);

    // 热重载归并：oldReal/newReal 共用 adapter，adapter 当前 real 切到 newReal。
    // 与 FieldInfo 同：嵌入类体分配，新旧配对须由重载流程按 (parent 类, 成员名) 完成。
    void RemapPropertyReal(PropertyInfo* oldReal, PropertyInfo* newReal);
    void RemapEventReal(EventInfo* oldReal, EventInfo* newReal);

    // ---- Il2CppDomain / Il2CppException 边界接口 ----

    // 出方向：真实对象 -> adapter，find-or-create。
    Il2CppDomainAdapter* WrapDomain(const Il2CppDomain* real);

    // 入方向：边界传回的 adapter -> 当前真实对象。
    const Il2CppDomain* UnwrapDomain(const Il2CppDomainAdapter* adapter);

    // 热重载归并（domain 通常不换对象，接口备用）。
    void RemapDomainReal(const Il2CppDomain* oldReal, const Il2CppDomain* newReal);

    // ---- Il2CppClass 边界接口 ----

    // 出方向：真实类 -> adapter，find-or-create。
    Il2CppClassAdapter* WrapClass(const Il2CppClass* real);

    // 入方向：边界传回的 adapter -> 当前真实类（可变指针，API 签名多为 Il2CppClass*）。
    Il2CppClass* UnwrapClass(const Il2CppClassAdapter* adapter);

    // 热重载归并：oldReal/newReal 共用 adapter，adapter 当前 real 切到 newReal。
    // userdata 槽不动（Unity::Type* 跨重载保留）。
    void RemapClassReal(const Il2CppClass* oldReal, const Il2CppClass* newReal);

    // userdata 槽读写（il2cpp_class_set_userdata / 偏移直读配套）。
    void* GetClassUserdata(const Il2CppClassAdapter* adapter);
    void SetClassUserdata(Il2CppClassAdapter* adapter, void* userdata);

    // ---- Il2CppType 边界接口（内容键映射，机制不同于指针键类型）----

    // 出方向：真实 Il2CppType -> adapter，按内容键 find-or-create。
    const Il2CppTypeAdapter* WrapType(const Il2CppType* real);

    // 入方向：边界传回的 adapter -> 当前真实 Il2CppType。
    const Il2CppType* UnwrapType(const Il2CppTypeAdapter* adapter);

    // 热重载归并：real==oldReal 的 adapter 切到 newReal（如 &oldK->byval_arg
    // -> &newK->byval_arg；泛型实例按 genericClass 配对同理）。O(n) 遍历，
    // 重载低频可接受。
    void RemapTypeReal(const Il2CppType* oldReal, const Il2CppType* newReal);
}
}
