#include "il2cpp-config.h"
#include "il2cpp-class-internals.h"
#include "il2cpp-object-internals.h"
#include "icalls/mscorlib/System/Delegate.h"
#include "gc/WriteBarrier.h"
#include "hybridclr/ReloadDiagLog.h"
#include "vm/Class.h"
#include "vm/Method.h"
#include "vm/Object.h"
#include "vm/Reflection.h"
#include "vm/Runtime.h"
#include "vm/Type.h"

#include <string.h>

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

        // ==={{ AssemblyReloadDiag
        // If the managed pre-checks in Delegate.CreateDelegate pass, we reach
        // here. Absence of this log for a failing method means the managed
        // checks (DeclaringType/param type identity) rejected the bind.
        if (method != NULL && method->name != NULL && strstr(method->name, "OnClickBackPackBtn") != NULL)
        {
            Il2CppClass* declaring = method->klass;
            Il2CppClass* targetKlass = target != NULL ? target->klass : NULL;
            hybridclr::ReloadDiagLog(
                "[ReloadDiag] CreateDelegate_internal reached: delegate=%s.%s klass=%p image=%p(%s) | method=%s declaring=%s.%s klass=%p image=%p(%s) | targetKlass=%s.%s %p\n",
                delegate_class->namespaze ? delegate_class->namespaze : "", delegate_class->name,
                (void*)delegate_class, (void*)delegate_class->image,
                delegate_class->image ? delegate_class->image->name : "?",
                method->name,
                declaring && declaring->namespaze ? declaring->namespaze : "", declaring ? declaring->name : "?",
                (void*)declaring, declaring ? (void*)declaring->image : NULL,
                declaring && declaring->image ? declaring->image->name : "?",
                targetKlass && targetKlass->namespaze ? targetKlass->namespaze : "", targetKlass ? targetKlass->name : "?",
                (void*)targetKlass);
        }
        // ===}} AssemblyReloadDiag

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
