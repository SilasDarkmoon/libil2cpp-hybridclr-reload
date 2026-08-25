#include "il2cpp-api-adapters.h"

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

    const Il2CppImage* WrapImage(const Il2CppImage* real)
    {
        return reinterpret_cast<const Il2CppImage*>(GetImageAdapterMap().Wrap(real));
    }

    const Il2CppImage* UnwrapImage(const Il2CppImage* adapterLike)
    {
        return ImageAdapterMap::Unwrap(reinterpret_cast<const Il2CppImageAdapter*>(adapterLike));
    }

    void RemapImageReal(const Il2CppImage* oldReal, const Il2CppImage* newReal)
    {
        GetImageAdapterMap().RemapReal(oldReal, newReal);
    }
}
}
