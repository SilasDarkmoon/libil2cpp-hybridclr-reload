#include "il2cpp-api-adapters.h"

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

    PropertyInfoAdapter* WrapProperty(PropertyInfo* real)
    {
        return GetPropertyAdapterMap().Wrap(real);
    }

    EventInfoAdapter* WrapEvent(EventInfo* real)
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
}
}
