#include "il2cpp-config.h"
#include "il2cpp-class-internals.h"
#include "Il2CppTypeCompare.h"
#include <cstdio>
#include "os/Directory.h"
#include "os/Environment.h"
#include "vm/Class.h"

namespace il2cpp
{
namespace metadata
{
    template<typename T>
    static inline int Compare(const T& left, const T& right)
    {
        if (left == right)
            return 0;

        if (left < right)
            return -1;

        return 1;
    }

    static int Compare(const Il2CppType* t1, const Il2CppType* t2)
    {
        int result = Compare(t1->type, t2->type);
        if (result != 0)
            return result;

        result = Compare(t1->byref, t2->byref);
        if (result != 0)
            return result;

        switch (t1->type)
        {
            case IL2CPP_TYPE_VALUETYPE:
            case IL2CPP_TYPE_CLASS:
                return Compare(t1->data.typeHandle, t2->data.typeHandle);

            case IL2CPP_TYPE_PTR:
            case IL2CPP_TYPE_SZARRAY:
                return Compare(t1->data.type, t2->data.type);

            case IL2CPP_TYPE_ARRAY:
            {
                result = Compare(t1->data.array->rank, t2->data.array->rank);
                if (result != 0)
                    return result;

                return Compare(t1->data.array->etype, t2->data.array->etype);
            }
            case IL2CPP_TYPE_GENERICINST:
            {
                const Il2CppGenericInst *i1 = t1->data.generic_class->context.class_inst;
                const Il2CppGenericInst *i2 = t2->data.generic_class->context.class_inst;

                // this happens when maximum generic recursion is hit
                if (i1 == NULL || i2 == NULL)
                {
                    if (i1 == i2)
                        return 0;
                    return (i1 == NULL) ? -1 : 1;
                }

                result = Compare(i1->type_argc, i2->type_argc);
                if (result != 0)
                    return result;

                result = Compare(t1->data.generic_class->type, t2->data.generic_class->type);
                if (result != 0)
                    return result;

                /* FIXME: we should probably just compare the instance pointers directly.  */
                for (uint32_t i = 0; i < i1->type_argc; ++i)
                {
                    result = Compare(i1->type_argv[i], i2->type_argv[i]);
                    if (result != 0)
                        return result;
                }

                return 0;
            }
            case IL2CPP_TYPE_VAR:
            case IL2CPP_TYPE_MVAR:
                return Compare(t1->data.genericParameterHandle, t2->data.genericParameterHandle);
            default:
                return 0;
        }

        IL2CPP_NOT_IMPLEMENTED(Il2CppTypeEqualityComparer::compare);
        return Compare(static_cast<const void*>(t1), static_cast<const void*>(t2));
    }

    bool Il2CppTypeEqualityComparer::AreEqual(const Il2CppType* t1, const Il2CppType* t2)
    {
        bool result = Compare(t1, t2) == 0;

        // ==={{ AssemblyReloadReuse: diagnostic for type comparison failures
        if (!result && t1 && t2 &&
            (t1->type == IL2CPP_TYPE_CLASS || t1->type == IL2CPP_TYPE_VALUETYPE) &&
            (t2->type == IL2CPP_TYPE_CLASS || t2->type == IL2CPP_TYPE_VALUETYPE) &&
            t1->type == t2->type && !t1->byref && !t2->byref &&
            t1->data.typeHandle != t2->data.typeHandle)
        {
            Il2CppClass* k1 = il2cpp::vm::Class::FromIl2CppType(t1);
            Il2CppClass* k2 = il2cpp::vm::Class::FromIl2CppType(t2);
            // Log when both resolve to same class (stale handle) or same name (duplicate class)
            bool shouldLog = false;
            const char* reason = "";
            if (k1 && k2 && k1 == k2 && k1->name)
            {
                shouldLog = true;
                reason = "STALE-HANDLE";
            }
            else if (k1 && k2 && k1->name && k2->name && strcmp(k1->name, k2->name) == 0)
            {
                shouldLog = true;
                reason = "DUP-CLASS";
            }
            if (shouldLog)
            {
                static int s_failCount = 0;
                if (s_failCount < 50)
                {
                    s_failCount++;
                    const std::string tmpCache = il2cpp::os::Environment::GetEnvironmentVariable("UNITY_TEMPORARY_CACHE_PATH");
                    std::string dirStr = !tmpCache.empty() ? tmpCache : "log";
                    int createError = 0;
                    il2cpp::os::Directory::Create(dirStr, &createError);
                    FILE* fp = fopen((dirStr + "/assembly_reload_reuse.log").c_str(), "a");
                    if (fp)
                    {
                        fprintf(fp, "[ReuseDiag] AreEqual %s: name=%s k1=%p k2=%p t1=%p h1=%p bh1=%p  t2=%p h2=%p bh2=%p\n",
                            reason, k1->name, (void*)k1, (void*)k2,
                            (void*)t1, (void*)t1->data.typeHandle, k1 ? (void*)k1->byval_arg.data.typeHandle : NULL,
                            (void*)t2, (void*)t2->data.typeHandle, k2 ? (void*)k2->byval_arg.data.typeHandle : NULL);
                        fclose(fp);
                    }
                }
            }
        }
        // ===}} AssemblyReloadReuse

        return result;
    }

    bool Il2CppTypeLess::operator()(const Il2CppType * t1, const Il2CppType * t2) const
    {
        return Compare(t1, t2) < 0;
    }
} /* namespace vm */
} /* namespace il2cpp */
