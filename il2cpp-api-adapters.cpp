#include "il2cpp-api-adapters.h"

#include "il2cpp-runtime-metadata.h"

#include <vector>

namespace il2cpp
{
namespace api
{
    typedef AdapterMap<Il2CppImage, Il2CppImageAdapter> ImageAdapterMap;

    static ImageAdapterMap& GetImageAdapterMap()
    {
        // C++11 magic static，初始化线程安全；实例常驻不释放。
        static ImageAdapterMap s_ImageAdapterMap;
        return s_ImageAdapterMap;
    }

    const Il2CppImageAdapter* WrapImage(const Il2CppImage* real)
    {
        return GetImageAdapterMap().Wrap(real);
    }

    const Il2CppImage* UnwrapImage(const Il2CppImageAdapter* adapter)
    {
        return ImageAdapterMap::Unwrap(adapter);
    }

    void RemapImageReal(const Il2CppImage* oldReal, const Il2CppImage* newReal)
    {
        GetImageAdapterMap().RemapReal(oldReal, newReal);
    }

    // ---- Il2CppAssembly ----

    typedef AdapterMap<Il2CppAssembly, Il2CppAssemblyAdapter> AssemblyAdapterMap;

    static AssemblyAdapterMap& GetAssemblyAdapterMap()
    {
        static AssemblyAdapterMap s_AssemblyAdapterMap;
        return s_AssemblyAdapterMap;
    }

    const Il2CppAssemblyAdapter* WrapAssembly(const Il2CppAssembly* real)
    {
        return GetAssemblyAdapterMap().Wrap(real);
    }

    const Il2CppAssembly* UnwrapAssembly(const Il2CppAssemblyAdapter* adapter)
    {
        return AssemblyAdapterMap::Unwrap(adapter);
    }

    void RemapAssemblyReal(const Il2CppAssembly* oldReal, const Il2CppAssembly* newReal)
    {
        GetAssemblyAdapterMap().RemapReal(oldReal, newReal);
    }

    const Il2CppAssemblyAdapter* const* WrapAssemblyArray(const Il2CppAssembly* const* assemblies, size_t count)
    {
        // 缓冲区按需增长，常驻不释放；元素是地址稳定的 adapter 指针。
        static std::vector<const Il2CppAssemblyAdapter*> s_Buffer;
        s_Buffer.clear();
        for (size_t i = 0; i < count; i++)
            s_Buffer.push_back(GetAssemblyAdapterMap().Wrap(assemblies[i]));
        return count > 0 ? &s_Buffer[0] : NULL;
    }

    // ---- FieldInfo ----

    typedef AdapterMap<FieldInfo, FieldInfoAdapter> FieldAdapterMap;

    static FieldAdapterMap& GetFieldAdapterMap()
    {
        static FieldAdapterMap s_FieldAdapterMap;
        return s_FieldAdapterMap;
    }

    FieldInfoAdapter* WrapField(FieldInfo* real)
    {
        return GetFieldAdapterMap().Wrap(real);
    }

    FieldInfo* UnwrapField(const FieldInfoAdapter* adapter)
    {
        // field API 签名多为非 const FieldInfo*，此处统一交出可变指针。
        return const_cast<FieldInfo*>(FieldAdapterMap::Unwrap(adapter));
    }

    void RemapFieldReal(FieldInfo* oldReal, FieldInfo* newReal)
    {
        GetFieldAdapterMap().RemapReal(oldReal, newReal);
    }

    // ---- MethodInfo ----

    typedef AdapterMap<MethodInfo, MethodInfoAdapter> MethodAdapterMap;

    static MethodAdapterMap& GetMethodAdapterMap()
    {
        static MethodAdapterMap s_MethodAdapterMap;
        return s_MethodAdapterMap;
    }

    const MethodInfoAdapter* WrapMethod(const MethodInfo* real)
    {
        return GetMethodAdapterMap().Wrap(real);
    }

    const MethodInfo* UnwrapMethod(const MethodInfoAdapter* adapter)
    {
        return MethodAdapterMap::Unwrap(adapter);
    }

    void RemapMethodReal(const MethodInfo* oldReal, const MethodInfo* newReal)
    {
        GetMethodAdapterMap().RemapReal(oldReal, newReal);
    }

    // ---- 栈帧边界转换 ----

    void WrapStackFrameInfo(const Il2CppStackFrameInfo& from, Il2CppStackFrameInfoAdapter& to)
    {
        to.method = WrapMethod(from.method);
        to.raw_ip = from.raw_ip;
        to.sourceCodeLineNumber = from.sourceCodeLineNumber;
        to.ilOffset = from.ilOffset;
        to.filePath = from.filePath;
    }

    // ---- PropertyInfo / EventInfo ----

    typedef AdapterMap<PropertyInfo, PropertyInfoAdapter> PropertyAdapterMap;
    typedef AdapterMap<EventInfo, EventInfoAdapter> EventAdapterMap;

    static PropertyAdapterMap& GetPropertyAdapterMap()
    {
        static PropertyAdapterMap s_PropertyAdapterMap;
        return s_PropertyAdapterMap;
    }

    static EventAdapterMap& GetEventAdapterMap()
    {
        static EventAdapterMap s_EventAdapterMap;
        return s_EventAdapterMap;
    }

    PropertyInfoAdapter* WrapProperty(const PropertyInfo* real)
    {
        return GetPropertyAdapterMap().Wrap(real);
    }

    EventInfoAdapter* WrapEvent(const EventInfo* real)
    {
        return GetEventAdapterMap().Wrap(real);
    }

    PropertyInfo* UnwrapProperty(const PropertyInfoAdapter* adapter)
    {
        return const_cast<PropertyInfo*>(PropertyAdapterMap::Unwrap(adapter));
    }

    EventInfo* UnwrapEvent(const EventInfoAdapter* adapter)
    {
        return const_cast<EventInfo*>(EventAdapterMap::Unwrap(adapter));
    }

    void RemapPropertyReal(PropertyInfo* oldReal, PropertyInfo* newReal)
    {
        GetPropertyAdapterMap().RemapReal(oldReal, newReal);
    }

    void RemapEventReal(EventInfo* oldReal, EventInfo* newReal)
    {
        GetEventAdapterMap().RemapReal(oldReal, newReal);
    }

    // ---- Il2CppDomain / Il2CppException ----

    typedef AdapterMap<Il2CppDomain, Il2CppDomainAdapter> DomainAdapterMap;
    typedef AdapterMap<Il2CppException, Il2CppExceptionAdapter> ExceptionAdapterMap;

    static DomainAdapterMap& GetDomainAdapterMap()
    {
        static DomainAdapterMap s_DomainAdapterMap;
        return s_DomainAdapterMap;
    }

    static ExceptionAdapterMap& GetExceptionAdapterMap()
    {
        static ExceptionAdapterMap s_ExceptionAdapterMap;
        return s_ExceptionAdapterMap;
    }

    Il2CppDomainAdapter* WrapDomain(const Il2CppDomain* real)
    {
        return GetDomainAdapterMap().Wrap(real);
    }

    Il2CppExceptionAdapter* WrapException(Il2CppException* real)
    {
        return GetExceptionAdapterMap().Wrap(real);
    }

    const Il2CppDomain* UnwrapDomain(const Il2CppDomainAdapter* adapter)
    {
        return DomainAdapterMap::Unwrap(adapter);
    }

    Il2CppException* UnwrapException(const Il2CppExceptionAdapter* adapter)
    {
        return const_cast<Il2CppException*>(ExceptionAdapterMap::Unwrap(adapter));
    }

    void RemapDomainReal(const Il2CppDomain* oldReal, const Il2CppDomain* newReal)
    {
        GetDomainAdapterMap().RemapReal(oldReal, newReal);
    }

    void RemapExceptionReal(Il2CppException* oldReal, Il2CppException* newReal)
    {
        GetExceptionAdapterMap().RemapReal(oldReal, newReal);
    }

    // ---- Il2CppClass ----

    typedef AdapterMap<Il2CppClass, Il2CppClassAdapter> ClassAdapterMap;

    static ClassAdapterMap& GetClassAdapterMap()
    {
        static ClassAdapterMap s_ClassAdapterMap;
        return s_ClassAdapterMap;
    }

    Il2CppClassAdapter* WrapClass(const Il2CppClass* real)
    {
        return GetClassAdapterMap().Wrap(real);
    }

    Il2CppClass* UnwrapClass(const Il2CppClassAdapter* adapter)
    {
        return const_cast<Il2CppClass*>(ClassAdapterMap::Unwrap(adapter));
    }

    void RemapClassReal(const Il2CppClass* oldReal, const Il2CppClass* newReal)
    {
        GetClassAdapterMap().RemapReal(oldReal, newReal);
    }

    void* GetClassUserdata(const Il2CppClassAdapter* adapter)
    {
        return adapter == NULL ? NULL : adapter->userdata;
    }

    void SetClassUserdata(Il2CppClassAdapter* adapter, void* userdata)
    {
        if (adapter != NULL)
            adapter->userdata = userdata;
    }

    // ---- Il2CppType（内容键映射）----
    // 键 = (type 枚举, byref, attrs, data 裸指针值)。num_mods/pinned 恒 0 忽略；
    // valuetype 位由 type 枚举推导。同一逻辑类型（即使 Il2CppType 实例地址不同，
    // 如多次 inflate 的结果）返回同一 adapter。

    namespace
    {
        struct TypeContentKey
        {
            uint32_t type;
            uint32_t byref;
            uint32_t attrs;
            const void* data;

            bool operator==(const TypeContentKey& other) const
            {
                return type == other.type && byref == other.byref && attrs == other.attrs && data == other.data;
            }
        };

        struct TypeContentKeyHash
        {
            size_t operator()(const TypeContentKey& key) const
            {
                // data 裸指针是主身份；type/byref/attrs 混入防碰撞
                size_t h = std::hash<const void*>()(key.data);
                h = h * 31 + key.type;
                h = h * 31 + key.byref;
                h = h * 31 + key.attrs;
                return h;
            }
        };

        // adapter 与键双索引：键->adapter 供 find-or-create；adapter->键 供 Remap 反查。
        std::unordered_map<TypeContentKey, Il2CppTypeAdapter*, TypeContentKeyHash> s_TypeByKey;
        std::unordered_map<const Il2CppTypeAdapter*, TypeContentKey> s_KeyByAdapter;
        baselib::ReentrantLock s_TypeMapMutex;

        TypeContentKey MakeTypeKey(const Il2CppType* t)
        {
            TypeContentKey key;
            key.type = t->type;
            key.byref = t->byref;
            key.attrs = t->attrs;
            key.data = t->data.dummy; // union 首字段按字节读裸指针值
            return key;
        }
    }

    const Il2CppTypeAdapter* WrapType(const Il2CppType* real)
    {
        if (real == NULL)
            return NULL;

        il2cpp::os::FastAutoLock lock(&s_TypeMapMutex);
        TypeContentKey key = MakeTypeKey(real);
        std::unordered_map<TypeContentKey, Il2CppTypeAdapter*, TypeContentKeyHash>::iterator it = s_TypeByKey.find(key);
        if (it != s_TypeByKey.end())
            return it->second;

        Il2CppTypeAdapter* adapter = (Il2CppTypeAdapter*)IL2CPP_CALLOC(1, sizeof(Il2CppTypeAdapter));
        adapter->real = real;
        s_TypeByKey.insert(std::make_pair(key, adapter));
        s_KeyByAdapter.insert(std::make_pair(adapter, key));
        return adapter;
    }

    const Il2CppType* UnwrapType(const Il2CppTypeAdapter* adapter)
    {
        return adapter == NULL ? NULL : adapter->real;
    }

    void RemapTypeReal(const Il2CppType* oldReal, const Il2CppType* newReal)
    {
        if (oldReal == NULL || newReal == NULL || oldReal == newReal)
            return;

        il2cpp::os::FastAutoLock lock(&s_TypeMapMutex);
        TypeContentKey oldKey = MakeTypeKey(oldReal);
        std::unordered_map<TypeContentKey, Il2CppTypeAdapter*, TypeContentKeyHash>::iterator it = s_TypeByKey.find(oldKey);
        if (it == s_TypeByKey.end())
            return;

        Il2CppTypeAdapter* adapter = it->second;
        adapter->real = newReal;
        s_TypeByKey.erase(it);
        s_TypeByKey.insert(std::make_pair(MakeTypeKey(newReal), adapter));
        s_KeyByAdapter[adapter] = MakeTypeKey(newReal);
    }
}
}
