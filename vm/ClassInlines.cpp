#include "ClassInlines.h"
#include "vm/Class.h"
#include "vm/Exception.h"
#include "vm/Method.h"
#include "vm/RCW.h"
#include "gc/GCHandle.h"
#include "metadata/GenericMethod.h"
#include <cstdio>
#include "os/Directory.h"
#include "os/Environment.h"
#include "hybridclr/metadata/MetadataUtil.h"

namespace il2cpp
{
namespace vm
{
    Il2CppClass* ClassInlines::InitFromCodegenSlow(Il2CppClass *klass)
    {
        IL2CPP_ASSERT(klass != il2cpp_defaults.il2cpp_fully_shared_type);

        Class::Init(klass);

        if (klass->initializationExceptionGCHandle)
            il2cpp::vm::Exception::Raise((Il2CppException*)gc::GCHandle::GetTarget(klass->initializationExceptionGCHandle));

        return klass;
    }

    Il2CppClass* ClassInlines::InitFromCodegenSlow(Il2CppClass *klass, bool throwOnError)
    {
        IL2CPP_ASSERT(klass != il2cpp_defaults.il2cpp_fully_shared_type);

        if (throwOnError)
            return InitFromCodegenSlow(klass);

        Class::Init(klass);

        if (klass->initializationExceptionGCHandle)
            return NULL;

        return klass;
    }

    const MethodInfo* ClassInlines::InitRgctxFromCodegenSlow(const MethodInfo* method)
    {
        il2cpp::metadata::GenericMethod::InflateRGCTX(method);
        return method;
    }

    NORETURN static void RaiseExceptionForNotFoundInterface(const Il2CppClass* klass, const Il2CppClass* itf, Il2CppMethodSlot slot)
    {
        std::string message;
        message = "Attempt to access method '" + Type::GetName(&itf->byval_arg, IL2CPP_TYPE_NAME_FORMAT_IL) + "." + Method::GetName(itf->methods[slot])
            + "' on type '" + Type::GetName(&klass->byval_arg, IL2CPP_TYPE_NAME_FORMAT_IL) + "' failed.";
        Exception::Raise(il2cpp::vm::Exception::GetMethodAccessException(message.c_str()));
    }

    const VirtualInvokeData* ClassInlines::GetInterfaceInvokeDataFromVTableSlowPath(const Il2CppClass* klass, const Il2CppClass* itf, Il2CppMethodSlot slot)
    {
        if (itf->generic_class != NULL)
        {
            for (uint16_t i = 0; i < klass->interface_offsets_count; ++i)
            {
                const Il2CppRuntimeInterfaceOffsetPair* pair = klass->interfaceOffsets + i;
                if (Class::IsGenericClassAssignableFromVariance(itf, pair->interfaceType, klass))
                {
                    IL2CPP_ASSERT(pair->offset + slot < klass->vtable_count);
                    return &klass->vtable[pair->offset + slot];
                }
            }
        }

        return NULL;
    }

    const VirtualInvokeData& ClassInlines::GetInterfaceInvokeDataFromVTableSlowPath(Il2CppObject* obj, const Il2CppClass* itf, Il2CppMethodSlot slot)
    {
        const Il2CppClass* klass = obj->klass;
        const VirtualInvokeData* data;

        data = GetInterfaceInvokeDataFromVTableSlowPath(klass, itf, slot);
        if (data)
            return *data;

        if (klass->is_import_or_windows_runtime)
        {
            Il2CppComObject* rcw = static_cast<Il2CppComObject*>(obj);

            // It might be null if it's called on a dead (already released) or fake object
            if (rcw->identity != NULL)
            {
                const VirtualInvokeData* invokeData = RCW::GetComInterfaceInvokeData(rcw, itf, slot);
                if (invokeData != NULL)
                {
                    // Nothing will be referencing these types directly, so we need to initialize them here
                    Class::Init(invokeData->method->klass);
                    return *invokeData;
                }
            }
        }

        RaiseExceptionForNotFoundInterface(klass, itf, slot);
        IL2CPP_UNREACHABLE;
    }

    // ==={{ AssemblyReloadReuse: diagnostic for HasParentUnsafe failures
    void ClassInlines::HasParentUnsafeFailSlowPath(const Il2CppClass* klass, const Il2CppClass* parent)
    {
        // Only log for types whose name contains "Operation" to reduce noise
        const char* kName = klass->name ? klass->name : "";
        const char* pName = parent->name ? parent->name : "";
        if (!strstr(kName, "Operation") && !strstr(pName, "Operation"))
            return;

        const std::string tmpCache = il2cpp::os::Environment::GetEnvironmentVariable("UNITY_TEMPORARY_CACHE_PATH");
        std::string dirStr = !tmpCache.empty() ? tmpCache : "log";
        int createError = 0;
        il2cpp::os::Directory::Create(dirStr, &createError);
        FILE* fp = fopen((dirStr + "/assembly_reload_reuse.log").c_str(), "a");
        if (fp) {
            fprintf(fp, "[ReuseDiag] HasParentUnsafe FAIL: parent='%s.%s'(d=%d,h=%p) klass='%s.%s'(d=%d,h=%p,gc=%p,klass_img=%p,parent_img=%p)\n",
                parent->namespaze ? parent->namespaze : "", pName,
                (int)parent->typeHierarchyDepth, (void*)parent->typeHierarchy,
                klass->namespaze ? klass->namespaze : "", kName,
                (int)klass->typeHierarchyDepth, (void*)klass->typeHierarchy,
                (void*)klass->generic_class,
                (void*)klass->image, (void*)parent->image);
            fclose(fp);
        }
    }
    // ===}} AssemblyReloadReuse
}
}
