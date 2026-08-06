#include "il2cpp-config.h"
#include "il2cpp-class-internals.h"
#include "il2cpp-object-internals.h"
#include "icalls/mscorlib/System/Delegate.h"
#include "gc/WriteBarrier.h"
#include "vm/Class.h"
#include "vm/Method.h"
#include "vm/Object.h"
#include "vm/Reflection.h"
#include "vm/Runtime.h"
#include "vm/Type.h"
#include <cstdio>
#include "os/Directory.h"
#include "os/Environment.h"

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

        // ==={{ AssemblyReloadReuse: diagnostic for Delegate.CreateDelegate
        if (delegate_class && method && delegate_class->name && method->name &&
            method->klass && method->klass->name)
        {
            static int s_logCount = 0;
            if (s_logCount < 50)
            {
                s_logCount++;
                const std::string tmpCache = il2cpp::os::Environment::GetEnvironmentVariable("UNITY_TEMPORARY_CACHE_PATH");
                std::string dirStr = !tmpCache.empty() ? tmpCache : "log";
                int createError = 0;
                il2cpp::os::Directory::Create(dirStr, &createError);
                FILE* fp = fopen((dirStr + "/assembly_reload_reuse.log").c_str(), "a");
                if (fp)
                {
                    // Find Invoke method in delegate class
                    il2cpp::vm::Class::Init(delegate_class);
                    const MethodInfo* invokeMethod = nullptr;
                    for (int i = 0; i < delegate_class->method_count; i++)
                    {
                        if (strcmp(delegate_class->methods[i]->name, "Invoke") == 0)
                        {
                            invokeMethod = delegate_class->methods[i];
                            break;
                        }
                    }
                    fprintf(fp, "[ReuseDiag] CreateDelegate_internal: delegate=%s method=%s.%s\n",
                        delegate_class->name, method->klass->name, method->name);
                    if (invokeMethod)
                    {
                        fprintf(fp, "  delegate Invoke: paramCount=%u\n", (unsigned)invokeMethod->parameters_count);
                        for (uint16_t i = 0; i < invokeMethod->parameters_count && i < 6; i++)
                        {
                            const Il2CppType* pt = invokeMethod->parameters[i];
                            if (pt)
                                fprintf(fp, "    dParam[%u]: type=%u handle=%p\n", (unsigned)i, (unsigned)pt->type, (void*)pt->data.typeHandle);
                        }
                    }
                    fprintf(fp, "  target method: paramCount=%u\n", (unsigned)method->parameters_count);
                    for (uint16_t i = 0; i < method->parameters_count && i < 6; i++)
                    {
                        const Il2CppType* pt = method->parameters[i];
                        if (pt)
                            fprintf(fp, "    mParam[%u]: type=%u handle=%p\n", (unsigned)i, (unsigned)pt->type, (void*)pt->data.typeHandle);
                    }
                    fclose(fp);
                }
            }
        }
        // ===}} AssemblyReloadReuse

        IL2CPP_ASSERT(delegate_class->parent == il2cpp_defaults.multicastdelegate_class);

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
