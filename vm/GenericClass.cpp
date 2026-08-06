#include "il2cpp-config.h"
#include "metadata/GenericMetadata.h"
#include "metadata/Il2CppGenericClassHash.h"
#include "metadata/Il2CppGenericClassCompare.h"
#include "os/Atomic.h"
#include "os/Directory.h"
#include "os/Environment.h"
#include "os/Mutex.h"
#include "utils/Memory.h"
#include "utils/Il2CppHashSet.h"
#include "vm/Class.h"
#include "vm/GenericClass.h"
#include "vm/Exception.h"
#include "vm/MetadataAlloc.h"
#include "vm/MetadataCache.h"
#include "vm/MetadataLock.h"
#include "vm/Type.h"
#include "hybridclr/metadata/MetadataUtil.h"
#include "hybridclr/metadata/MetadataModule.h"
#include "hybridclr/ReloadDiagLog.h"
#include "il2cpp-class-internals.h"
#include "il2cpp-runtime-metadata.h"
#include "il2cpp-runtime-stats.h"
#include <vector>
#include <string.h>

namespace il2cpp
{
namespace vm
{
    void GenericClass::SetupMethods(Il2CppClass* genericInstanceType)
    {
        Il2CppClass* genericTypeDefinition = GenericClass::GetTypeDefinition(genericInstanceType->generic_class);
        uint16_t methodCount = genericTypeDefinition->method_count;
        IL2CPP_ASSERT(genericTypeDefinition->method_count == genericInstanceType->method_count);

        if (methodCount == 0)
        {
            genericInstanceType->methods = NULL;
            return;
        }

        const MethodInfo** methods = (const MethodInfo**)MetadataCalloc(methodCount, sizeof(MethodInfo*));

        // ==={{ AssemblyReloadReuse: try to reuse old MethodInfo by signature
        hybridclr::metadata::InterpreterImage* reuseImage = nullptr;
        if (hybridclr::metadata::IsInterpreterType(genericInstanceType))
        {
            reuseImage = hybridclr::metadata::MetadataModule::GetImage(genericInstanceType);
            if (reuseImage && !reuseImage->HasReuseData())
                reuseImage = nullptr;
        }
        // ===}} AssemblyReloadReuse

        for (uint16_t methodIndex = 0; methodIndex < methodCount; ++methodIndex)
        {
            const MethodInfo* methodDefinition = genericTypeDefinition->methods[methodIndex];

            // ==={{ AssemblyReloadReuse
            MethodInfo* reused = nullptr;
            if (reuseImage)
            {
                // First inflate to get correct parameter/return types
                const Il2CppGenericContext* ctx = GenericClass::GetContext(genericInstanceType->generic_class);

                const MethodInfo* inflated = metadata::GenericMetadata::Inflate(
                    methodDefinition, ctx);

                // Build parameter type array from INFLATED method (not methodDefinition)
                const Il2CppType** paramTypes = nullptr;
                if (inflated->parameters_count > 0)
                {
                    paramTypes = (const Il2CppType**)alloca(inflated->parameters_count * sizeof(Il2CppType*));
                    for (uint16_t pi = 0; pi < inflated->parameters_count; pi++)
                        paramTypes[pi] = inflated->parameters[pi];
                }
                reused = reuseImage->TryReuseMethodFromMetadata(
                    genericInstanceType, inflated->name,
                    inflated->return_type,
                    paramTypes, inflated->parameters_count);

                // If reused, we already have the inflated MethodInfo
                if (reused)
                {
                    // Update the old MethodInfo with inflated data
                    reused->name = inflated->name;
                    reused->klass = genericInstanceType;
                    reused->return_type = inflated->return_type;
                    reused->parameters_count = inflated->parameters_count;
                    const Il2CppType** params = (const Il2CppType**)MetadataCalloc(
                        inflated->parameters_count, sizeof(Il2CppType*));
                    for (uint16_t pi = 0; pi < inflated->parameters_count; pi++)
                        params[pi] = inflated->parameters[pi];
                    reused->parameters = params;
                    reused->flags = inflated->flags;
                    reused->iflags = inflated->iflags;
                    reused->slot = inflated->slot;
                    reused->token = inflated->token;
                    reused->methodMetadataHandle = inflated->methodMetadataHandle;
                    reused->genericContainerHandle = inflated->genericContainerHandle;
                    reused->is_generic = inflated->is_generic;
                    reused->is_inflated = inflated->is_inflated;
                    if (inflated->genericMethod)
                        reused->genericMethod = inflated->genericMethod;
                    reused->methodPointer = inflated->methodPointer;
                    reused->virtualMethodPointer = inflated->virtualMethodPointer;
                    // ==={{ AssemblyReloadReuse: inherit interp call pointers from
                    // inflated method instead of resetting to nullptr. Resetting
                    // would force re-init via InitAndGetInterpreterDirectlyCallMethodPointerSlow,
                    // but IsImplementedByInterpreter returns false for interpreter
                    // assemblies (no AOTHomologousImage), so re-init would fail
                    // and methodPointerCallByInterp would stay null, causing
                    // RaiseAOTGenericMethodNotInstantiatedException. ===
                    reused->methodPointerCallByInterp = inflated->methodPointerCallByInterp;
                    reused->virtualMethodPointerCallByInterp = inflated->virtualMethodPointerCallByInterp;
                    reused->initInterpCallMethodPointer = inflated->initInterpCallMethodPointer;
                    // ===}} AssemblyReloadReuse
                    reused->interpData = nullptr;
                    reused->invoker_method = inflated->invoker_method;
                    reused->isInterpterImpl = inflated->isInterpterImpl;

                    methods[methodIndex] = reused;
                    continue;
                }
            }
            // ===}} AssemblyReloadReuse

            const MethodInfo* inflated2 = metadata::GenericMetadata::Inflate(methodDefinition, GenericClass::GetContext(genericInstanceType->generic_class));

            methods[methodIndex] = inflated2;
        }

        genericInstanceType->methods = methods;

        il2cpp_runtime_stats.method_count += methodCount;
    }

    static void InflatePropertyDefinition(const PropertyInfo* propertyDefinition, PropertyInfo* newProperty, Il2CppClass* declaringClass, Il2CppGenericContext* context)
    {
        newProperty->attrs = propertyDefinition->attrs;
        newProperty->parent = declaringClass;
        newProperty->name = propertyDefinition->name;
        newProperty->token = propertyDefinition->token;

        if (propertyDefinition->get)
            newProperty->get = metadata::GenericMetadata::Inflate(propertyDefinition->get, context);
        if (propertyDefinition->set)
            newProperty->set = metadata::GenericMetadata::Inflate(propertyDefinition->set, context);
    }

    void GenericClass::SetupProperties(Il2CppClass* genericInstanceType)
    {
        Il2CppClass* genericTypeDefinition = GenericClass::GetTypeDefinition(genericInstanceType->generic_class);
        uint16_t propertyCount = genericTypeDefinition->property_count;
        IL2CPP_ASSERT(genericTypeDefinition->property_count == genericInstanceType->property_count);

        if (propertyCount == 0)
        {
            genericInstanceType->properties = NULL;
            return;
        }

        PropertyInfo* properties = (PropertyInfo*)MetadataCalloc(propertyCount, sizeof(PropertyInfo));
        PropertyInfo* property = properties;

        for (uint16_t propertyIndex = 0; propertyIndex < propertyCount; ++propertyIndex)
        {
            InflatePropertyDefinition(genericTypeDefinition->properties + propertyIndex, property, genericInstanceType, GenericClass::GetContext(genericInstanceType->generic_class));
            property++;
        }

        genericInstanceType->properties = properties;
    }

    static void InflateEventDefinition(const EventInfo* eventDefinition, EventInfo* newEvent, Il2CppClass* declaringClass, Il2CppGenericContext* context)
    {
        newEvent->eventType = metadata::GenericMetadata::InflateIfNeeded(eventDefinition->eventType, context, false);
        newEvent->name = eventDefinition->name;
        newEvent->parent = declaringClass;
        newEvent->token = eventDefinition->token;

        if (eventDefinition->add)
            newEvent->add = metadata::GenericMetadata::Inflate(eventDefinition->add, context);
        if (eventDefinition->raise)
            newEvent->raise = metadata::GenericMetadata::Inflate(eventDefinition->raise, context);
        if (eventDefinition->remove)
            newEvent->remove = metadata::GenericMetadata::Inflate(eventDefinition->remove, context);
    }

    void GenericClass::SetupEvents(Il2CppClass* genericInstanceType)
    {
        Il2CppClass* genericTypeDefinition = GenericClass::GetTypeDefinition(genericInstanceType->generic_class);
        uint16_t eventCount = genericTypeDefinition->event_count;
        IL2CPP_ASSERT(genericTypeDefinition->event_count == genericInstanceType->event_count);

        if (eventCount == 0)
        {
            genericInstanceType->events = NULL;
            return;
        }

        EventInfo* events = (EventInfo*)MetadataCalloc(eventCount, sizeof(EventInfo));
        EventInfo* event = events;

        for (uint16_t eventIndex = 0; eventIndex < eventCount; ++eventIndex)
        {
            InflateEventDefinition(genericTypeDefinition->events + eventIndex, event, genericInstanceType, GenericClass::GetContext(genericInstanceType->generic_class));
            event++;
        }

        genericInstanceType->events = events;
    }

    static FieldInfo* InflateFieldDefinition(const FieldInfo* fieldDefinition, FieldInfo* newField, Il2CppClass* declaringClass, Il2CppGenericContext* context)
    {
        newField->type = metadata::GenericMetadata::InflateIfNeeded(fieldDefinition->type, context, false);
        newField->name = fieldDefinition->name;
        newField->parent = declaringClass;
        newField->offset = fieldDefinition->offset;
        newField->token = fieldDefinition->token;

        return newField;
    }

    void GenericClass::SetupFields(Il2CppClass* genericInstanceType)
    {
        Il2CppClass* genericTypeDefinition = GenericClass::GetTypeDefinition(genericInstanceType->generic_class);
        uint16_t fieldCount = genericTypeDefinition->field_count;
        IL2CPP_ASSERT(genericTypeDefinition->field_count == genericInstanceType->field_count);

        if (fieldCount == 0)
        {
            genericInstanceType->fields = NULL;
            return;
        }

        FieldInfo* fields = (FieldInfo*)MetadataCalloc(fieldCount, sizeof(FieldInfo));
        FieldInfo* field = fields;

        for (uint16_t fieldIndex = 0; fieldIndex < fieldCount; ++fieldIndex)
        {
            InflateFieldDefinition(genericTypeDefinition->fields + fieldIndex, field, genericInstanceType, GenericClass::GetContext(genericInstanceType->generic_class));
            field++;
        }

        genericInstanceType->fields = fields;
    }

    Il2CppClass* GenericClass::GetClass(Il2CppGenericClass* gclass, bool throwOnError)
    {
        Il2CppClass* cachedClass = os::Atomic::LoadPointerRelaxed(&gclass->cached_class);
        if (cachedClass)
            return cachedClass;
        return CreateClass(gclass, throwOnError);
    }

    typedef Il2CppHashSet < Il2CppGenericClass*, il2cpp::metadata::Il2CppGenericClassHash, il2cpp::metadata::Il2CppGenericClassCompare > Il2CppGenericClassSet;
    static Il2CppGenericClassSet s_GenericClassSet;

    Il2CppClass* GenericClass::CreateClass(Il2CppGenericClass *gclass, bool throwOnError)
    {
        Il2CppClass* definition = GetTypeDefinition(gclass);
        if (definition == NULL)
        {
            if (throwOnError)
                vm::Exception::Raise(vm::Exception::GetMaximumNestedGenericsException());
            return NULL;
        }

        os::FastAutoLock lock(&g_MetadataLock);
        Il2CppGenericClassSet::const_iterator iter = s_GenericClassSet.find(gclass);
        if (iter != s_GenericClassSet.end())
        {
            Il2CppGenericClass* cacheGclass = *iter;
            IL2CPP_ASSERT(cacheGclass->cached_class);
            il2cpp::os::Atomic::ExchangePointer(&gclass->cached_class, cacheGclass->cached_class);
            return gclass->cached_class;
        }
        // ==={{ AssemblyReloadDiag
        // Cache miss -> a NEW generic instance class will be created. If any
        // generic argument is a stale (old-image) Il2CppType of a reloaded
        // interpreter type, this creates a duplicate class whose inflated
        // method parameters keep the stale typeHandle -> Type == Type fails.
        bool reloadDiagEntered = hybridclr::ReloadDiagEnabled() && hybridclr::ReloadDiagTryEnter();
        if (reloadDiagEntered && !gclass->cached_class && gclass->context.class_inst)
        {
            const Il2CppGenericInst* inst = gclass->context.class_inst;
            for (uint32_t ai = 0; ai < inst->type_argc; ++ai)
            {
                const Il2CppType* at = inst->type_argv[ai];
                if (at == NULL || at->byref || (at->type != IL2CPP_TYPE_VALUETYPE && at->type != IL2CPP_TYPE_CLASS))
                    continue;
                Il2CppClass* ak = MetadataCache::GetTypeInfoFromType(at);
                if (ak == NULL || ak->name == NULL)
                    continue;
                if (strstr(ak->name, "Backpack") == NULL && strstr(ak->name, "HomeWindow") == NULL)
                    continue;
                Il2CppClass* defKlass = GetTypeDefinition(gclass);
                hybridclr::ReloadDiagLog(
                    "[ReloadDiag] GenericClassCacheMiss: def=%s.%s gclass=%p argv%u il2cppType=%p typeHandle=%p argKlass=%s.%s %p argKlassByval=%p argKlassByvalHandle=%p isByvalOfKlass=%d\n",
                    defKlass && defKlass->namespaze ? defKlass->namespaze : "", defKlass ? defKlass->name : "?",
                    (void*)gclass, ai, (const void*)at, (void*)at->data.typeHandle,
                    ak->namespaze ? ak->namespaze : "", ak->name, (void*)ak,
                    (void*)&ak->byval_arg, (void*)ak->byval_arg.data.typeHandle,
                    at == &ak->byval_arg ? 1 : 0);
            }
        }
        if (reloadDiagEntered)
            hybridclr::ReloadDiagLeave();
        // ===}} AssemblyReloadDiag
        if (!gclass->cached_class)
        {
            // Il2CppClass uses a fixed-length layout (IL2CPP_MAX_VTABLE_SLOT_COUNT inline vtable slots) for types
            // whose vtable fits, otherwise a variable-length allocation of (vtable_count + IL2CPP_PRESERVED_VTABLE_SLOT_COUNT)
            // slots. ComputeVTableAllocatedSlotCount returns the allocated slot count and logs the variable case.
            const uint32_t vtableCount = definition->vtable_count;
            const uint32_t vtableAllocated = il2cpp::vm::Class::ComputeVTableAllocatedSlotCount(vtableCount, definition->namespaze, definition->name);
            const size_t extraVTableBytes = (vtableAllocated > IL2CPP_MAX_VTABLE_SLOT_COUNT)
                ? (vtableAllocated - IL2CPP_MAX_VTABLE_SLOT_COUNT) * sizeof(VirtualInvokeData)
                : 0;
            Il2CppClass* klass = (Il2CppClass*)MetadataCalloc(1, sizeof(Il2CppClass) + extraVTableBytes);
            klass->klass = klass;
            klass->vtable_count = (uint16_t)vtableCount;
            klass->vtable_allocated_count = (uint16_t)vtableAllocated;

            klass->name = definition->name;
            klass->namespaze = definition->namespaze;

            klass->image = definition->image;
            klass->flags = definition->flags;
            //klass->type_token = definition->type_token;
            klass->generic_class = gclass;

            Il2CppClass* genericTypeDefinition = GenericClass::GetTypeDefinition(klass->generic_class);
            Il2CppGenericContext* context = &klass->generic_class->context;

            if (genericTypeDefinition->parent)
                klass->parent = Class::FromIl2CppType(metadata::GenericMetadata::InflateIfNeeded(&genericTypeDefinition->parent->byval_arg, context, false));

            if (genericTypeDefinition->declaringType)
                klass->declaringType = Class::FromIl2CppType(metadata::GenericMetadata::InflateIfNeeded(&genericTypeDefinition->declaringType->byval_arg, context, false));

            klass->this_arg.type = klass->byval_arg.type = IL2CPP_TYPE_GENERICINST;
            klass->this_arg.data.generic_class = klass->byval_arg.data.generic_class = gclass;
            klass->this_arg.byref = true;
            klass->byval_arg.valuetype = genericTypeDefinition->byval_arg.valuetype;

            klass->event_count = definition->event_count;
            klass->field_count = definition->field_count;
            klass->interfaces_count = definition->interfaces_count;
            klass->method_count = definition->method_count;
            klass->property_count = definition->property_count;

            klass->enumtype = definition->enumtype;
            klass->element_class = klass->castClass = klass;

            klass->has_cctor = definition->has_cctor;
            klass->cctor_finished_or_no_cctor = !definition->has_cctor;

            klass->has_finalize = definition->has_finalize;
            klass->native_size = klass->thread_static_fields_offset = -1;
            klass->token = definition->token;
            klass->interopData = MetadataCache::GetInteropDataForType(&klass->byval_arg);

            if (GenericClass::GetTypeDefinition(klass->generic_class) == il2cpp_defaults.generic_nullable_class)
            {
                klass->element_class = klass->castClass = Class::FromIl2CppType(klass->generic_class->context.class_inst->type_argv[0]);
                klass->nullabletype = true;
            }

            if (klass->enumtype)
                klass->element_class = klass->castClass = definition->element_class;

            klass->is_import_or_windows_runtime = definition->is_import_or_windows_runtime;
            // Do not update gclass->cached_class until `klass` is fully initialized
            // And do so with an atomic barrier so no threads observer the writes out of order
            il2cpp::os::Atomic::ExchangePointer(&gclass->cached_class, klass);
            Il2CppGenericClass* cloneGclass = (Il2CppGenericClass*)IL2CPP_MALLOC_ZERO(sizeof(Il2CppGenericClass));
            *cloneGclass = *gclass;
            s_GenericClassSet.insert(cloneGclass);
        }

        return gclass->cached_class;
    }

    // ==={{ AssemblyReloadReuse
    static bool NormalizeGenericInstTypeArgv(const Il2CppGenericInst* inst)
    {
        bool changed = false;
        for (uint32_t i = 0; i < inst->type_argc; i++)
        {
            const Il2CppType* t = inst->type_argv[i];
            if (t && (t->type == IL2CPP_TYPE_CLASS || t->type == IL2CPP_TYPE_VALUETYPE))
            {
                Il2CppClass* k = MetadataCache::GetTypeInfoFromType(t);
                if (k && &k->byval_arg != t &&
                    k->byval_arg.data.typeHandle != t->data.typeHandle)
                {
                    const_cast<Il2CppGenericInst*>(inst)->type_argv[i] = &k->byval_arg;
                    changed = true;
                }
            }
        }
        return changed;
    }

    void GenericClass::RehashGenericTypeSet()
    {
        // Collect all entries, clear the set, and re-insert them.
        // This recomputes hashes with the updated byval_arg.data.typeHandle.
        // Also update gclass->type to &klass->byval_arg for CLASS/VALUETYPE
        // types whose class was reused, so hash/compare use the new typeHandle.
        std::vector<Il2CppGenericClass*> entries;
        for (Il2CppGenericClassSet::const_iterator it = s_GenericClassSet.begin(); it != s_GenericClassSet.end(); ++it)
            entries.push_back((*it).key);
        s_GenericClassSet.clear();
        for (Il2CppGenericClass* gclass : entries)
        {
            const Il2CppType* defType = gclass->type;
            if (defType && (defType->type == IL2CPP_TYPE_CLASS || defType->type == IL2CPP_TYPE_VALUETYPE))
            {
                Il2CppClass* klass = MetadataCache::GetTypeInfoFromType(defType);
                if (klass && &klass->byval_arg != defType &&
                    klass->byval_arg.data.typeHandle != defType->data.typeHandle)
                {
                    gclass->type = &klass->byval_arg;
                }
            }
            // Also normalize stale type_argv in context.class_inst.
            // class_inst may be AOT-created (not in s_GenericInstSet), so
            // RehashGenericInstSet didn't update it. Without this, the hash
            // is computed with stale type_argv, and later lookups (which use
            // normalized type_argv) won't find the entry -> duplicate entries
            // -> InvalidCastException.
            if (gclass->context.class_inst)
            {
                bool changed = NormalizeGenericInstTypeArgv(gclass->context.class_inst);
                // Also check if RehashGenericInstSet already updated this inst.
                if (!changed && MetadataCache::WasGenericInstChanged(gclass->context.class_inst))
                    changed = true;
                if (changed)
                {
                    // type_argv changed: the inflated method parameters
                    // (method->parameters) still hold the old Il2CppType*
                    // pointers. Reset methods so SetupMethods re-inflates
                    // them with the normalized type_argv.
                    if (gclass->cached_class)
                        gclass->cached_class->methods = nullptr;
                }
            }
            s_GenericClassSet.insert(gclass);
        }
    }
    // ===}} AssemblyReloadReuse

    Il2CppGenericContext* GenericClass::GetContext(Il2CppGenericClass *gclass)
    {
        return &gclass->context;
    }

    Il2CppClass* GenericClass::GetTypeDefinition(Il2CppGenericClass *gclass)
    {
        return MetadataCache::GetTypeInfoFromType(gclass->type);
    }

    bool GenericClass::IsEnum(Il2CppGenericClass *gclass)
    {
        return Type::IsEnum(gclass->type);
    }

    // ==={{ AssemblyReloadReuse

    // Recursively check if a type depends on the reloaded interpreter image.
    // Returns true if the type is from newImage, or if it's a generic instance
    // whose definition or any generic argument depends on newImage.
    static bool ShouldRestoreType(const Il2CppType* type, const Il2CppImage* newImage);

    static bool ShouldRestoreGenericClass(Il2CppClass* cachedClass, Il2CppGenericClass* gclass, const Il2CppImage* newImage)
    {
        if (!gclass || !gclass->type)
            return false;

        // Fast path: if the cached class's image already points to newImage,
        // it was already restored (or is from newImage).  This catches the
        // common case where RestoreReusedClasses has already updated the
        // generic type definition's Il2CppClass in-place.
        if (cachedClass && cachedClass->image == newImage)
            return true;

        // Check generic type definition WITHOUT triggering class resolution
        // (GetTypeDefinition would call Class::FromIl2CppType which may
        // trigger Class::Init during restore). Instead, check the type's
        // typeHandle directly.
        const Il2CppType* defType = gclass->type;

        if (defType->type == IL2CPP_TYPE_CLASS || defType->type == IL2CPP_TYPE_VALUETYPE)
        {
            const Il2CppTypeDefinition* typeDef = (const Il2CppTypeDefinition*)defType->data.typeHandle;
            if (typeDef && hybridclr::metadata::IsInterpreterType(typeDef))
            {
                hybridclr::metadata::InterpreterImage* img =
                    hybridclr::metadata::MetadataModule::GetImage(typeDef);
                if (img && img->GetIl2CppImage() == newImage)
                    return true;

                // The typeDef may still point to the OLD image even though
                // the generic type definition's Il2CppClass was reused in
                // RestoreReusedClasses (which updates klass->image but not
                // gclass->type->data.typeHandle).  Resolve the class from
                // the old image's _classList and check if its image was
                // updated to newImage.
                if (img)
                {
                    uint32_t rawIndex = hybridclr::metadata::DecodeMetadataIndex(typeDef->byvalTypeIndex);
                    Il2CppClass* resolvedKlass = img->GetTypeInfoFromTypeDefinitionRawIndex(rawIndex);
                    if (resolvedKlass && resolvedKlass->image == newImage)
                        return true;
                }
            }
        }

        // Check generic arguments recursively
        const Il2CppGenericInst* inst = gclass->context.class_inst;
        if (inst)
        {
            for (uint32_t i = 0; i < inst->type_argc; i++)
            {
                if (ShouldRestoreType(inst->type_argv[i], newImage))
                    return true;
            }
        }

        return false;
    }

    static bool ShouldRestoreType(const Il2CppType* type, const Il2CppImage* newImage)
    {
        if (!type)
            return false;
        // Guard against invalid type values that could cause crashes
        uint8_t t = type->type;
        if (t == 0 || t > IL2CPP_TYPE_MVAR)
            return false;
        switch (t)
        {
        case IL2CPP_TYPE_CLASS:
        case IL2CPP_TYPE_VALUETYPE:
        {
            const Il2CppTypeDefinition* typeDef = (const Il2CppTypeDefinition*)type->data.typeHandle;
            if (!typeDef)
                return false;
            if (hybridclr::metadata::IsInterpreterType(typeDef))
            {
                hybridclr::metadata::InterpreterImage* img =
                    hybridclr::metadata::MetadataModule::GetImage(typeDef);
                if (img && img->GetIl2CppImage() == newImage)
                    return true;

                // typeDef may still point to the OLD image even though the
                // class was reused in RestoreReusedClasses.  Resolve the
                // class from the old image's _classList and check if its
                // image was updated to newImage.
                if (img)
                {
                    uint32_t rawIndex = hybridclr::metadata::DecodeMetadataIndex(typeDef->byvalTypeIndex);
                    Il2CppClass* resolvedKlass = img->GetTypeInfoFromTypeDefinitionRawIndex(rawIndex);
                    if (resolvedKlass && resolvedKlass->image == newImage)
                        return true;
                    // Also check if the class was reused by ANY reload (not just
                    // the current one). If byval_arg.data.typeHandle was updated
                    // to a different value than type->data.typeHandle, the class
                    // was reused and its type identity changed.
                    if (resolvedKlass && &resolvedKlass->byval_arg != type &&
                        resolvedKlass->byval_arg.data.typeHandle != type->data.typeHandle)
                        return true;
                    // If the Il2CppType* is NOT &klass->byval_arg, it's from an
                    // old type table. Even if typeHandle currently matches (because
                    // Pass 1 hasn't run yet), it WILL change after Pass 1 updates
                    // byval_arg.data.typeHandle. The inflated method parameters
                    // (which were copied from this old Il2CppType*) will then have
                    // stale typeHandle values that point to freed memory, causing
                    // Class::FromIl2CppType to return NULL and IsAssignableFrom
                    // to fail. So we must return TRUE to trigger method re-inflation.
                    if (resolvedKlass && &resolvedKlass->byval_arg != type)
                        return true;
                }
            }
            return false;
        }
        case IL2CPP_TYPE_GENERICINST:
        {
            const Il2CppGenericClass* gclass = type->data.generic_class;
            if (gclass)
                return ShouldRestoreGenericClass(nullptr, const_cast<Il2CppGenericClass*>(gclass), newImage);
            return false;
        }
        case IL2CPP_TYPE_SZARRAY:
            return ShouldRestoreType(type->data.type, newImage);
        case IL2CPP_TYPE_ARRAY:
            return ShouldRestoreType(type->data.array->etype, newImage);
        case IL2CPP_TYPE_PTR:
            return ShouldRestoreType(type->data.type, newImage);
        case IL2CPP_TYPE_BYREF:
            return ShouldRestoreType(type->data.type, newImage);
        default:
            return false;
        }
    }

    void GenericClass::CollectMethodsFromGenericClasses(const Il2CppImage* oldImage, CollectMethodCallback callback, void* userData)
    {
        os::FastAutoLock lock(&g_MetadataLock);
        for (Il2CppGenericClass* gclass : s_GenericClassSet)
        {
            if (!gclass->cached_class)
                continue;
            Il2CppClass* klass = gclass->cached_class;
            // Check if this generic instance depends on the old image.
            // Use the same recursive logic as ShouldRestoreGenericClass,
            // but check against oldImage (before RestoreReusedClasses updates
            // definition->image to newImage).
            bool shouldCollect = false;
            // Check if klass->image is the old image
            if (klass->image == oldImage)
                shouldCollect = true;
            // Also check if the generic type definition's image is oldImage
            if (!shouldCollect && gclass->type)
            {
                const Il2CppType* defType = gclass->type;
                if (defType->type == IL2CPP_TYPE_CLASS || defType->type == IL2CPP_TYPE_VALUETYPE)
                {
                    const Il2CppTypeDefinition* typeDef = (const Il2CppTypeDefinition*)defType->data.typeHandle;
                    if (typeDef && hybridclr::metadata::IsInterpreterType(typeDef))
                    {
                        hybridclr::metadata::InterpreterImage* img =
                            hybridclr::metadata::MetadataModule::GetImage(typeDef);
                        if (img && img->GetIl2CppImage() == oldImage)
                            shouldCollect = true;
                    }
                }
                // Check generic arguments recursively
                if (!shouldCollect)
                {
                    const Il2CppGenericInst* inst = gclass->context.class_inst;
                    if (inst)
                    {
                        for (uint32_t i = 0; i < inst->type_argc; i++)
                        {
                            const Il2CppType* argType = inst->type_argv[i];
                            if (argType && (argType->type == IL2CPP_TYPE_CLASS || argType->type == IL2CPP_TYPE_VALUETYPE))
                            {
                                const Il2CppTypeDefinition* argTypeDef = (const Il2CppTypeDefinition*)argType->data.typeHandle;
                                if (argTypeDef && hybridclr::metadata::IsInterpreterType(argTypeDef))
                                {
                                    hybridclr::metadata::InterpreterImage* argImg =
                                        hybridclr::metadata::MetadataModule::GetImage(argTypeDef);
                                    if (argImg && argImg->GetIl2CppImage() == oldImage)
                                    {
                                        shouldCollect = true;
                                        break;
                                    }
                                }
                            }
                            // Also check nested generic instances
                            if (argType && argType->type == IL2CPP_TYPE_GENERICINST)
                            {
                                const Il2CppGenericClass* nestedGclass = argType->data.generic_class;
                                if (nestedGclass && nestedGclass->cached_class &&
                                    nestedGclass->cached_class->image == oldImage)
                                {
                                    shouldCollect = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            if (!shouldCollect)
                continue;
            // Ensure methods are set up before collecting
            if (!klass->methods && klass->method_count > 0)
            {
                il2cpp::vm::Class::SetupMethods(klass);
            }
            if (!klass->methods || klass->method_count == 0)
                continue;
            // Log for debugging
            for (uint16_t m = 0; m < klass->method_count; m++)
            {
                const MethodInfo* method = klass->methods[m];
                if (method)
                    callback(method, userData);
            }
        }
    }

    // ==={{ AssemblyReloadReuse
    // Static vector to track classes that need restoration across passes.
    static std::vector<Il2CppClass*> s_GenericClassesToRestore;

    void GenericClass::RestoreCachedGenericClassesPass1(const Il2CppImage* oldImage, Il2CppImage* newImage)
    {
        os::FastAutoLock lock(&g_MetadataLock);
        s_GenericClassesToRestore.clear();

        for (Il2CppGenericClass* gclass : s_GenericClassSet)
        {
            if (!gclass->cached_class)
                continue;

            Il2CppClass* klass = gclass->cached_class;

            bool shouldRestore = ShouldRestoreGenericClass(klass, gclass, newImage);
            if (!shouldRestore)
                continue;

            // Update klass->image to point to the image of the generic type
            // definition (NOT newImage).  The class may be restored because
            // a generic argument depends on newImage, but klass->image must
            // always point to the image where the type definition lives so
            // that method tokens resolve correctly.
            bool defIsInterp = false;
            {
                const Il2CppType* defType = gclass->type;
                if (defType && (defType->type == IL2CPP_TYPE_CLASS || defType->type == IL2CPP_TYPE_VALUETYPE))
                {
                    const Il2CppTypeDefinition* typeDef = (const Il2CppTypeDefinition*)defType->data.typeHandle;
                    defIsInterp = typeDef && hybridclr::metadata::IsInterpreterType(typeDef);
                    if (defIsInterp)
                    {
                        // Set klass->image to the image of the generic type
                        // definition, not newImage.  The definition's class
                        // (already restored by RestoreReusedClasses) has the
                        // correct image.
                        Il2CppClass* defKlass = GetTypeDefinition(gclass);
                        if (defKlass && defKlass->image)
                            klass->image = defKlass->image;
                    }
                }
            }

            // NOTE: parent re-resolution is deferred to Pass 3 (before
            // Class::Init) because Class::FromIl2CppType and
            // GenericMetadata::InflateIfNeeded can trigger class creation,
            // which is unsafe during Pass 1 when the reuse list is still
            // being built and classes may not have correct external
            // pointers yet.

            s_GenericClassesToRestore.push_back(klass);
        }
    }

    void GenericClass::RestoreCachedGenericClassesPass2()
    {
        for (Il2CppClass* klass : s_GenericClassesToRestore)
        {
            // ==={{ AssemblyReloadReuse: normalize stale type_argv in class_inst
            // class_inst may be AOT-created (not in s_GenericInstSet), so
            // RehashGenericInstSet didn't update it. Normalize here so that
            // SetupMethods → Inflate → InflateGenericParameterIfNeeded uses
            // the new Il2CppType* (&klass->byval_arg) instead of stale ones.
            if (klass->generic_class && klass->generic_class->context.class_inst)
            {
                const Il2CppGenericInst* inst = klass->generic_class->context.class_inst;
                for (uint32_t i = 0; i < inst->type_argc; i++)
                {
                    const Il2CppType* t = inst->type_argv[i];
                    if (t && !t->byref &&
                        (t->type == IL2CPP_TYPE_CLASS || t->type == IL2CPP_TYPE_VALUETYPE))
                    {
                        Il2CppClass* k = MetadataCache::GetTypeInfoFromType(t);
                        if (k && &k->byval_arg != t &&
                            k->byval_arg.data.typeHandle != t->data.typeHandle)
                        {
                            const_cast<Il2CppGenericInst*>(inst)->type_argv[i] = &k->byval_arg;
                        }
                    }
                }
            }
            // ===}} AssemblyReloadReuse

            klass->fields = nullptr;
            klass->methods = nullptr;
            klass->properties = nullptr;
            klass->events = nullptr;
            klass->nestedTypes = nullptr;
            klass->implementedInterfaces = nullptr;
            klass->interfaceOffsets = nullptr;
            klass->static_fields = nullptr;
		klass->rgctx_data = nullptr;
		// NOTE: Do NOT reset klass->parent here.  InitLocked does not
		// rebuild it — it is set during class creation from the TypeDef's
		// parentIndex.  Pass 3 re-resolves parent and calls Class::Init
		// which rebuilds the type hierarchy via SetupTypeHierarchyLocked.
		klass->typeHierarchy = nullptr;
		klass->typeHierarchyDepth = 0;
		klass->gc_desc = nullptr;
            klass->initialized = 0;
            klass->initialized_and_no_error = 0;
            klass->init_pending = 0;
            klass->size_init_pending = 0;
            klass->size_inited = 0;
            klass->is_vtable_initialized = 0;
            klass->cctor_started = 0;
            klass->cctor_finished_or_no_cctor = !klass->has_cctor;
            klass->cctor_thread = 0;
            klass->genericRecursionDepth = 0;
            klass->initializationExceptionGCHandle = 0;
            klass->unity_user_data = nullptr;
        }
    }

    void GenericClass::RestoreCachedGenericClassesPass3()
    {
        int restoredCount = (int)s_GenericClassesToRestore.size();
        for (Il2CppClass* klass : s_GenericClassesToRestore)
        {
            // Re-resolve parent from the generic type definition before
            // Class::Init, so that inheritance changes after reload are
            // picked up.
            Il2CppGenericClass* gclass = klass->generic_class;
            if (gclass)
            {
                Il2CppClass* genericTypeDefinition = GetTypeDefinition(gclass);
                if (genericTypeDefinition && genericTypeDefinition->parent)
                {
                    Il2CppGenericContext* context = &gclass->context;
                    const Il2CppType* inflatedParentType = metadata::GenericMetadata::InflateIfNeeded(
                        &genericTypeDefinition->parent->byval_arg, context, false);
                    if (inflatedParentType)
                    {
                        Il2CppClass* newParent = Class::FromIl2CppType(inflatedParentType);
                        klass->parent = newParent;
                    }
                }
                else if (genericTypeDefinition && !genericTypeDefinition->parent)
                {
                    klass->parent = nullptr;
                }
            }

            il2cpp::vm::Class::Init(klass);
        }
        s_GenericClassesToRestore.clear();
    }

    void GenericClass::RestoreCachedGenericClasses(const Il2CppImage* oldImage, Il2CppImage* newImage)
    {
        RestoreCachedGenericClassesPass1(oldImage, newImage);
        RestoreCachedGenericClassesPass2();
        RestoreCachedGenericClassesPass3();
    }
    // ===}} AssemblyReloadReuse
    // ===}} AssemblyReloadReuse

} /* namespace vm */
} /* namespace il2cpp */
