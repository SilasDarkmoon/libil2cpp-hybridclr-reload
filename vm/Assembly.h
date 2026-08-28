#pragma once

#include <stdint.h>
#include <vector>
#include "il2cpp-config.h"
struct Il2CppAssembly;
struct Il2CppAssemblyName;
struct Il2CppImage;
struct Il2CppArray;

namespace il2cpp
{
namespace vm
{
    typedef std::vector<const Il2CppAssembly*> AssemblyVector;
    typedef std::vector<const Il2CppAssemblyName*> AssemblyNameVector;

    class LIBIL2CPP_CODEGEN_API Assembly
    {
// exported
    public:
        static Il2CppImage* GetImage(const Il2CppAssembly* assembly);
        static void GetReferencedAssemblies(const Il2CppAssembly* assembly, AssemblyNameVector* target);
    public:
        static AssemblyVector* GetAllAssemblies();
        static void GetAllAssemblies(AssemblyVector& assemblies);
        static const Il2CppAssembly* GetLoadedAssembly(const char* name);
        static const Il2CppAssembly* Load(const char* name);
        static void Register(const Il2CppAssembly* assembly);
        // 热重载：s_Assemblies 中原地替换条目（索引稳定，快照失效经版本号触发）
        static void ReplaceAssembly(const Il2CppAssembly* oldAssembly, const Il2CppAssembly* newAssembly);
        static void InvalidateAssemblyList();
        static void ClearAllAssemblies();
        static void Initialize();

    private:
    };
} /* namespace vm */
} /* namespace il2cpp */
