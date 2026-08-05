#include "il2cpp-config.h"
#include "il2cpp-class-internals.h"
#include "il2cpp-object-internals.h"
#include "icalls/mscorlib/System/Delegate.h"
#include "gc/WriteBarrier.h"
#include "os/Directory.h"
#include "os/Environment.h"
#include "vm/Class.h"
#include "vm/GlobalMetadata.h"
#include "vm/Method.h"
#include "vm/Object.h"
#include "vm/Reflection.h"
#include "vm/Runtime.h"
#include "vm/Type.h"

namespace il2cpp
{
namespace icalls
{
namespace mscorlib
{
namespace System
{
    Il2CppDelegate * Delegate::CreateDelegate_internal(Il2CppReflectionType *__type, Il2CppObject *target, Il2CppReflectionMethod *info, bool throwOnBindFailure)
    {
        Il2CppClass *delegate_class = il2cpp::vm::Class::FromIl2CppType(__type->type);
        const MethodInfo *method = info->method;

        IL2CPP_ASSERT(delegate_class->parent == il2cpp_defaults.multicastdelegate_class);

        // ==={{ AssemblyReloadReuse: diagnostic for delegate compatibility
        if (method && method->name && method->klass && method->klass->name &&
            (strstr(method->name, "OnClick") || strstr(method->name, "DoExecute")))
        {
            const std::string tmpCache = il2cpp::os::Environment::GetEnvironmentVariable("UNITY_TEMPORARY_CACHE_PATH");
            std::string dirStr = !tmpCache.empty() ? tmpCache : "log";
            int createError = 0;
            il2cpp::os::Directory::Create(dirStr, &createError);
            FILE* fp = fopen((dirStr + "/assembly_reload_reuse.log").c_str(), "a");
            if (fp) {
                fprintf(fp, "[ReuseDiag] CreateDelegate: method='%s.%s' is_inflated=%d is_generic=%d paramCount=%u delegate_class='%s.%s'\n",
                    method->klass->namespaze ? method->klass->namespaze : "", method->name,
                    (int)method->is_inflated, (int)method->is_generic,
                    (unsigned)method->parameters_count,
                    delegate_class->namespaze ? delegate_class->namespaze : "", delegate_class->name ? delegate_class->name : "?");
                for (uint16_t i = 0; i < method->parameters_count && i < 4; i++)
                {
                    const Il2CppType* pt = method->parameters[i];
                    fprintf(fp, "  method param[%u]: type=%u typeHandle=%p", (unsigned)i, (unsigned)pt->type, (void*)pt->data.typeHandle);
                    if (pt->type == IL2CPP_TYPE_CLASS || pt->type == IL2CPP_TYPE_VALUETYPE)
                    {
                        Il2CppClass* pk = il2cpp::vm::GlobalMetadata::GetTypeInfoFromHandle(pt->data.typeHandle);
                        fprintf(fp, " klass=%p klass->byval_arg.data.typeHandle=%p", (void*)pk, (void*)pk->byval_arg.data.typeHandle);
                    }
                    fprintf(fp, "\n");
                }
                // Print delegate invoke method parameters
                const MethodInfo* invokeMethod = il2cpp::vm::Class::GetMethodFromName(delegate_class, "Invoke", method->parameters_count);
                if (invokeMethod)
                {
                    for (uint16_t i = 0; i < invokeMethod->parameters_count && i < 4; i++)
                    {
                        const Il2CppType* pt = invokeMethod->parameters[i];
                        fprintf(fp, "  delegate Invoke param[%u]: type=%u typeHandle=%p", (unsigned)i, (unsigned)pt->type, (void*)pt->data.typeHandle);
                        if (pt->type == IL2CPP_TYPE_CLASS || pt->type == IL2CPP_TYPE_VALUETYPE)
                        {
                            Il2CppClass* pk = il2cpp::vm::GlobalMetadata::GetTypeInfoFromHandle(pt->data.typeHandle);
                            fprintf(fp, " klass=%p klass->byval_arg.data.typeHandle=%p", (void*)pk, (void*)pk->byval_arg.data.typeHandle);
                        }
                        fprintf(fp, "\n");
                    }
                }
                fclose(fp);
            }
        }
        // ===}} AssemblyReloadReuse

        Il2CppObject* delegate = il2cpp::vm::Object::New(delegate_class);
        il2cpp::vm::Type::ConstructDelegate((Il2CppDelegate*)delegate, target, method);

        return (Il2CppDelegate*)delegate;
    }

    void Delegate::SetMulticastInvoke(Il2CppDelegate * delegate)
    {
#if IL2CPP_TINY
        IL2CPP_NOT_IMPLEMENTED_ICALL(Delegate::SetMulticastInvoke);
#else
#endif
    }

    Il2CppMulticastDelegate* Delegate::AllocDelegateLike_internal(Il2CppDelegate* d)
    {
#if IL2CPP_TINY
        IL2CPP_NOT_IMPLEMENTED_ICALL(Delegate::AllocDelegateLike_internal);
        return NULL;
#else
        IL2CPP_ASSERT(d->object.klass->parent == il2cpp_defaults.multicastdelegate_class);

        Il2CppMulticastDelegate *ret = (Il2CppMulticastDelegate*)il2cpp::vm::Object::New(d->object.klass);

        IL2CPP_OBJECT_SETREF((&ret->delegate), invoke_impl_this, (Il2CppObject*)ret);

        // extra_arg stores the multicast_invoke_impl
        ret->delegate.invoke_impl = (Il2CppMethodPointer)d->extraArg;
        ret->delegate.extraArg = d->extraArg;

        return ret;
#endif
    }

    Il2CppReflectionMethod* Delegate::GetVirtualMethod_internal(Il2CppDelegate* _this)
    {
#if IL2CPP_TINY
        IL2CPP_NOT_IMPLEMENTED_ICALL(Delegate::GetVirtualMethod_internal);
        return NULL;
#else
        const MethodInfo* resolvedMethod = _this->target != NULL ? il2cpp::vm::Object::GetVirtualMethod(_this->target, _this->method) : _this->method;
        return il2cpp::vm::Reflection::GetMethodObject(resolvedMethod, NULL);
#endif
    }
} /* namespace System */
} /* namespace mscorlib */
} /* namespace icalls */
} /* namespace il2cpp */
