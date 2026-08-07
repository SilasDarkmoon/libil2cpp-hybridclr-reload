#pragma once

#include <stdint.h>
#include "il2cpp-config.h"
#include "vm/Type.h"

struct Il2CppClass;
struct Il2CppGenericClass;
struct Il2CppGenericContext;

namespace il2cpp
{
namespace vm
{
    class LIBIL2CPP_CODEGEN_API GenericClass
    {
    public:
        // exported

    public:
        //internal
        static Il2CppClass* GetClass(Il2CppGenericClass *gclass, bool throwOnError = true);
        static Il2CppGenericContext* GetContext(Il2CppGenericClass *gclass);
        static Il2CppClass* GetTypeDefinition(Il2CppGenericClass *gclass);
        static bool IsEnum(Il2CppGenericClass *gclass);

        inline static bool IsValueType(Il2CppGenericClass* gclass)
        {
            return Type::IsValueType(gclass->type);
        }

        static void SetupEvents(Il2CppClass* genericInstanceType);
        static void SetupFields(Il2CppClass* genericInstanceType);
        static void SetupMethods(Il2CppClass* genericInstanceType);
        static void SetupProperties(Il2CppClass* genericInstanceType);

        // ==={{ AssemblyReloadReuse
        // Iterate the internal generic-class cache and for every entry whose
        // cached_class->image matches oldImage, update the image pointer to
        // newImage and reset lazily-initialised fields so they get
        // re-computed from the new metadata.
        static void RestoreCachedGenericClasses(const Il2CppImage* oldImage, Il2CppImage* newImage);

        // Split versions for interleaved execution with RestoreReusedClasses:
        //   Pass 1: update image pointers (run after non-generic Pass 1)
        //   Pass 2: reset lazy-init fields (run after non-generic Pass 2)
        //   Pass 3: call Class::Init (run after non-generic Pass 3)
        static void RestoreCachedGenericClassesPass1(const Il2CppImage* oldImage, Il2CppImage* newImage);
        static void RestoreCachedGenericClassesPass2();
        static void RestoreCachedGenericClassesPass3();

        // Collect reusable MethodInfo objects from generic instance classes
        // whose image matches oldImage. Calls callback for each method.
        typedef void (*CollectMethodCallback)(const MethodInfo* method, void* userData);
        static void CollectMethodsFromGenericClasses(const Il2CppImage* oldImage, CollectMethodCallback callback, void* userData);

        // Rehash the internal generic-class cache after byval_arg.data.typeHandle
        // has been updated by Pass 1. The hash of Il2CppGenericClass entries depends
        // on typeHandle, so after updating it we must rehash to keep lookups working.
        static void RehashGenericTypeSet();

        // Enable in-place normalization of stale (old-image) type_argv entries
        // when inflating generic instance members (SetupMethods/SetupFields/
        // SetupProperties/SetupEvents) and when looking up the generic-class
        // cache in CreateClass. Enabled after the first assembly reload;
        // before that it is a no-op so startup behaviour is unchanged.
        static void EnableReloadArgvNormalization();
        // ===}} AssemblyReloadReuse

        static bool HasSameGenericTypeDefinition(const Il2CppGenericClass* gclass1, const Il2CppGenericClass* gclass2)
        {
            IL2CPP_ASSERT(gclass1->type->type == IL2CPP_TYPE_VALUETYPE || gclass1->type->type == IL2CPP_TYPE_CLASS);
            IL2CPP_ASSERT(gclass2->type->type == IL2CPP_TYPE_VALUETYPE || gclass2->type->type == IL2CPP_TYPE_CLASS);

            return gclass1->type->data.typeHandle == gclass2->type->data.typeHandle;
        }

    private:
        static Il2CppClass* CreateClass(Il2CppGenericClass *gclass, bool throwOnError = true);
    };
} /* namespace vm */
} /* namespace il2cpp */
