#include "il2cpp-api-adapters.h"

#include "il2cpp-runtime-metadata.h"
#include "gc/WriteBarrier.h"
#include "vm/GlobalMetadataFileInternals.h"
#include "vm/Class.h"
#include "hybridclr/metadata/InterpreterImage.h"
#include "hybridclr/metadata/MetadataModule.h"
#include "hybridclr/metadata/MetadataUtil.h"

#include <cstring>
#include <string>
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

    PropertyInfoAdapter* WrapProperty(const PropertyInfo* real)
    {
        return GetPropertyAdapterMap().Wrap(real);
    }

    EventInfoAdapter* WrapEvent(const EventInfo* real)
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

    // ---- Il2CppDomain / Il2CppException ----

    typedef AdapterMap<Il2CppDomain, Il2CppDomainAdapter> DomainAdapterMap;

    static DomainAdapterMap& GetDomainAdapterMap()
    {
        static DomainAdapterMap s_DomainAdapterMap;
        return s_DomainAdapterMap;
    }

    Il2CppDomainAdapter* WrapDomain(const Il2CppDomain* real)
    {
        return GetDomainAdapterMap().Wrap(real);
    }

    const Il2CppDomain* UnwrapDomain(const Il2CppDomainAdapter* adapter)
    {
        return DomainAdapterMap::Unwrap(adapter);
    }

    void RemapDomainReal(const Il2CppDomain* oldReal, const Il2CppDomain* newReal)
    {
        GetDomainAdapterMap().RemapReal(oldReal, newReal);
    }

    // ---- Il2CppClass ----

    typedef AdapterMap<Il2CppClass, Il2CppClassAdapter> ClassAdapterMap;

    static ClassAdapterMap& GetClassAdapterMap()
    {
        static ClassAdapterMap s_ClassAdapterMap;
        return s_ClassAdapterMap;
    }

    Il2CppClassAdapter* WrapClass(const Il2CppClass* real)
    {
        return GetClassAdapterMap().Wrap(real);
    }

    Il2CppClass* UnwrapClass(const Il2CppClassAdapter* adapter)
    {
        return const_cast<Il2CppClass*>(ClassAdapterMap::Unwrap(adapter));
    }

    void RemapClassReal(const Il2CppClass* oldReal, const Il2CppClass* newReal)
    {
        GetClassAdapterMap().RemapReal(oldReal, newReal);
    }

    // 热重载协调器专用：暴露 class map 供批量重绑（文件内使用）
    AdapterMap<Il2CppClass, Il2CppClassAdapter>& GetClassAdapterMapForReload()
    {
        return GetClassAdapterMap();
    }

    AdapterMap<FieldInfo, FieldInfoAdapter>& GetFieldAdapterMapForReload()
    {
        return GetFieldAdapterMap();
    }

    AdapterMap<MethodInfo, MethodInfoAdapter>& GetMethodAdapterMapForReload()
    {
        return GetMethodAdapterMap();
    }

    AdapterMap<PropertyInfo, PropertyInfoAdapter>& GetPropertyAdapterMapForReload()
    {
        return GetPropertyAdapterMap();
    }

    AdapterMap<EventInfo, EventInfoAdapter>& GetEventAdapterMapForReload()
    {
        return GetEventAdapterMap();
    }

    void* GetClassUserdata(const Il2CppClassAdapter* adapter)
    {
        return adapter == NULL ? NULL : adapter->userdata;
    }

    void SetClassUserdata(Il2CppClassAdapter* adapter, void* userdata)
    {
        if (adapter != NULL)
            adapter->userdata = userdata;
    }

    // ---- Il2CppType（内容键映射）----
    // 键 = (type 枚举, byref, attrs, data 裸指针值)。num_mods/pinned 恒 0 忽略；
    // valuetype 位由 type 枚举推导。同一逻辑类型（即使 Il2CppType 实例地址不同，
    // 如多次 inflate 的结果）返回同一 adapter。

    namespace
    {
        struct TypeContentKey
        {
            uint32_t type;
            uint32_t byref;
            uint32_t attrs;
            const void* data;

            bool operator==(const TypeContentKey& other) const
            {
                return type == other.type && byref == other.byref && attrs == other.attrs && data == other.data;
            }
        };

        struct TypeContentKeyHash
        {
            size_t operator()(const TypeContentKey& key) const
            {
                // data 裸指针是主身份；type/byref/attrs 混入防碰撞
                size_t h = std::hash<const void*>()(key.data);
                h = h * 31 + key.type;
                h = h * 31 + key.byref;
                h = h * 31 + key.attrs;
                return h;
            }
        };

        // adapter 与键双索引：键->adapter 供 find-or-create；adapter->键 供 Remap 反查。
        std::unordered_map<TypeContentKey, Il2CppTypeAdapter*, TypeContentKeyHash> s_TypeByKey;
        std::unordered_map<const Il2CppTypeAdapter*, TypeContentKey> s_KeyByAdapter;
        baselib::ReentrantLock s_TypeMapMutex;

        TypeContentKey MakeTypeKey(const Il2CppType* t)
        {
            TypeContentKey key;
            key.type = t->type;
            key.byref = t->byref;
            key.attrs = t->attrs;
            key.data = t->data.dummy; // union 首字段按字节读裸指针值
            return key;
        }
    }

    const Il2CppTypeAdapter* WrapType(const Il2CppType* real)
    {
        if (real == NULL)
            return NULL;

        il2cpp::os::FastAutoLock lock(&s_TypeMapMutex);
        TypeContentKey key = MakeTypeKey(real);
        std::unordered_map<TypeContentKey, Il2CppTypeAdapter*, TypeContentKeyHash>::iterator it = s_TypeByKey.find(key);
        if (it != s_TypeByKey.end())
            return it->second;

        Il2CppTypeAdapter* adapter = (Il2CppTypeAdapter*)IL2CPP_CALLOC(1, sizeof(Il2CppTypeAdapter));
        adapter->real = real;
        s_TypeByKey.insert(std::make_pair(key, adapter));
        s_KeyByAdapter.insert(std::make_pair(adapter, key));
        return adapter;
    }

    const Il2CppType* UnwrapType(const Il2CppTypeAdapter* adapter)
    {
        return adapter == NULL ? NULL : adapter->real;
    }

    void RemapTypeReal(const Il2CppType* oldReal, const Il2CppType* newReal)
    {
        if (oldReal == NULL || newReal == NULL || oldReal == newReal)
            return;

        il2cpp::os::FastAutoLock lock(&s_TypeMapMutex);
        TypeContentKey oldKey = MakeTypeKey(oldReal);
        std::unordered_map<TypeContentKey, Il2CppTypeAdapter*, TypeContentKeyHash>::iterator it = s_TypeByKey.find(oldKey);
        if (it == s_TypeByKey.end())
            return;

        Il2CppTypeAdapter* adapter = it->second;
        adapter->real = newReal;
        s_TypeByKey.erase(it);
        s_TypeByKey.insert(std::make_pair(MakeTypeKey(newReal), adapter));
        s_KeyByAdapter[adapter] = MakeTypeKey(newReal);
    }

    // ---- 热重载协调器 ----
    // 前置：新程序集已 fresh 载入并原地替换注册表条目；assembly/image 的 adapter
    // 已 Remap。本函数按全名配对旧/新 image 的类型并批量重绑成员/Type adapter。

    namespace
    {
        // 读元数据构造全名 "Ns.Outer/Inner"（不触发类加载）。
        // 嵌套链用 klass->declaringType（Class::FromName 构建时已解析好，无索引歧义；
        // typeDef.declaringTypeIndex 实际存的是 byvalTypeIndex，不能直接当 typeDef 索引用）。
        void BuildClassFullName(const Il2CppClass* klass, std::string& out)
        {
            out.clear();
            if (klass == NULL)
                return;
            const Il2CppImage* klassImage = klass->image;
            if (klassImage == NULL)
                return;
            hybridclr::metadata::InterpreterImage* image = (hybridclr::metadata::InterpreterImage*)hybridclr::metadata::MetadataModule::GetImage(klassImage);
            if (image == NULL)
                return;

            // 沿 klass->declaringType 向上收集（最多 32 层防御）
            const Il2CppClass* chain[32];
            int depth = 0;
            const Il2CppClass* cur = klass;
            while (cur != NULL && depth < 32)
            {
                chain[depth++] = cur;
                cur = cur->declaringType;
            }
            if (depth == 0)
                return;

            // 最外层命名空间（读其 typeDef 的 namespaceIndex，编码索引须 Decode）
            const Il2CppClass* outermost = chain[depth - 1];
            const Il2CppTypeDefinition* outerDef = (const Il2CppTypeDefinition*)outermost->typeMetadataHandle;
            if (outerDef != NULL)
            {
                const char* ns = image->GetStringFromRawIndex(
                    (StringIndex)hybridclr::metadata::DecodeMetadataIndex(outerDef->namespaceIndex));
                if (ns != NULL && *ns != '\0')
                {
                    out += ns;
                    out += '.';
                }
            }
            // 逐层 /Name（klass->name 已是解析后的名字指针）
            for (int i = depth - 1; i >= 0; i--)
            {
                out += chain[i]->name != NULL ? chain[i]->name : "";
                if (i > 0)
                    out += '/';
            }
        }

        struct ReloadPairContext
        {
            hybridclr::metadata::InterpreterImage* oldImage;
            hybridclr::metadata::InterpreterImage* newImage;
            const Il2CppImage* oldIl2Image;
            std::unordered_map<std::string, Il2CppClass*>* nameIndex; // 新 image 全名索引（惰性构建）
        };

        bool ClassBelongsToOldImage(const Il2CppClass* real, void* userData)
        {
            return real->image == ((ReloadPairContext*)userData)->oldIl2Image;
        }

        // 新 image 全名索引：遍历全部类型定义并**主动实例化**（GetTypeInfoFrom-
        // DefinitionRawIndex 触发构建）。关键：必须实例化全部新类，否则配对失败时
        // 新代码 typeof(T) 会 Wrap 出新 adapter（地址不同于引擎 MonoScript 缓存的
        // 旧 adapter）→ AddComponent 查不到 MonoScript 返回 null。
        std::unordered_map<std::string, Il2CppClass*>& GetNewClassNameIndex(ReloadPairContext& ctx)
        {
            if (ctx.nameIndex == NULL)
            {
                ctx.nameIndex = new std::unordered_map<std::string, Il2CppClass*>();
                const std::vector<Il2CppTypeDefinition>& typeDefs = ctx.newImage->GetTypeDefines();
                for (size_t i = 0; i < typeDefs.size(); i++)
                {
                    // 主动实例化（幂等：已实例化的直接返回）
                    Il2CppClass* klass = ctx.newImage->GetTypeInfoFromTypeDefinitionRawIndex((uint32_t)i);
                    if (klass == NULL)
                        continue;
                    std::string name;
                    BuildClassFullName(klass, name);
                    if (!name.empty())
                        (*ctx.nameIndex)[name] = klass;
                }
            }
            return *ctx.nameIndex;
        }

        // 类配对：oldKlass 全名 -> 新 image 同名已实例化类
        const Il2CppClass* PairClassByName(const Il2CppClass* oldReal, void* userData)
        {
            ReloadPairContext& ctx = *(ReloadPairContext*)userData;
            std::string fullName;
            BuildClassFullName(oldReal, fullName);
            if (fullName.empty())
                return NULL;
            std::unordered_map<std::string, Il2CppClass*>& index = GetNewClassNameIndex(ctx);
            std::unordered_map<std::string, Il2CppClass*>::iterator it = index.find(fullName);
            return it != index.end() ? it->second : NULL;
        }

        // ---- 成员配对（filter = parent 类属旧 image；pair = 新类内按名/签名找）----

        bool FieldBelongsToOldImage(const FieldInfo* real, void* userData)
        {
            return real->parent != NULL && real->parent->image == ((ReloadPairContext*)userData)->oldIl2Image;
        }

        bool MethodBelongsToOldImage(const MethodInfo* real, void* userData)
        {
            return real->klass != NULL && real->klass->image == ((ReloadPairContext*)userData)->oldIl2Image;
        }

        bool PropertyBelongsToOldImage(const PropertyInfo* real, void* userData)
        {
            return real->parent != NULL && real->parent->image == ((ReloadPairContext*)userData)->oldIl2Image;
        }

        bool EventBelongsToOldImage(const EventInfo* real, void* userData)
        {
            return real->parent != NULL && real->parent->image == ((ReloadPairContext*)userData)->oldIl2Image;
        }

        // 类配对结果查询：oldK -> newK（类 map 已重绑，oldK 的 adapter->real 即 newK）
        const Il2CppClass* ResolvePairedClass(const Il2CppClass* oldK)
        {
            return GetClassAdapterMapForReload().ResolveCurrentReal(oldK);
        }

        const FieldInfo* PairFieldByName(const FieldInfo* oldReal, void* userData)
        {
            const Il2CppClass* newParent = ResolvePairedClass(oldReal->parent);
            if (newParent == NULL)
                return NULL;
            // 新类可能尚未 SetupFields：触发惰性构建（重载流程可接受）
            Il2CppClass* newParentMutable = const_cast<Il2CppClass*>(newParent);
            il2cpp::vm::Class::SetupFields(newParentMutable);
            for (uint16_t i = 0; i < newParentMutable->field_count; i++)
            {
                if (std::strcmp(newParentMutable->fields[i].name, oldReal->name) == 0)
                    return &newParentMutable->fields[i];
            }
            return NULL;
        }

        const MethodInfo* PairMethodByNameAndSignature(const MethodInfo* oldReal, void* userData)
        {
            const Il2CppClass* newParent = ResolvePairedClass(oldReal->klass);
            if (newParent == NULL)
                return NULL;
            Il2CppClass* newParentMutable = const_cast<Il2CppClass*>(newParent);
            il2cpp::vm::Class::SetupMethods(newParentMutable);
            // 先按 名字+参数个数 粗筛（同名重载场景），再按参数/返回类型内容比对
            const MethodInfo* candidates[16];
            size_t candidateCount = 0;
            for (uint16_t i = 0; i < newParentMutable->method_count; i++)
            {
                const MethodInfo* m = newParentMutable->methods[i];
                if (m == NULL || std::strcmp(m->name, oldReal->name) != 0)
                    continue;
                if (m->parameters_count != oldReal->parameters_count)
                    continue;
                if (candidateCount < 16)
                    candidates[candidateCount++] = m;
            }
            if (candidateCount == 0)
                return NULL;
            if (candidateCount == 1)
                return candidates[0];
            // 同名同参数个数重载：按参数/返回类型内容比对（Il2CppType 内容等价 =
            // type 枚举 + data 指针指向同一 klass/genericClass——跨新旧 image 的
            // 类型指针不同，故仅比对 type 枚举与递归元素，指针不同的返回 NULL 保守跳过）
            for (size_t c = 0; c < candidateCount; c++)
            {
                const MethodInfo* m = candidates[c];
                bool match = true;
                for (uint8_t p = 0; p < oldReal->parameters_count && match; p++)
                {
                    const Il2CppType* a = oldReal->parameters[p];
                    const Il2CppType* b = m->parameters[p];
                    if (a->type != b->type || a->byref != b->byref)
                    {
                        match = false;
                        break;
                    }
                    // CLASS/VALUETYPE/GENERICINST 的 data 指针跨新旧不同——
                    // 解析到类后按全名比对（递归一层，成本可接受）
                    if (a->type == IL2CPP_TYPE_CLASS || a->type == IL2CPP_TYPE_VALUETYPE || a->type == IL2CPP_TYPE_GENERICINST)
                    {
                        const Il2CppClass* ka = il2cpp::vm::Class::FromIl2CppType(a);
                        const Il2CppClass* kb = il2cpp::vm::Class::FromIl2CppType(b);
                        if (ka == NULL || kb == NULL || ka == kb)
                        {
                            match = (ka == kb);
                            continue;
                        }
                        std::string nameA, nameB;
                        BuildClassFullName(ka, nameA);
                        BuildClassFullName(kb, nameB);
                        if (nameA != nameB)
                            match = false;
                    }
                }
                if (match)
                    return m;
            }
            return NULL;
        }

        const PropertyInfo* PairPropertyByName(const PropertyInfo* oldReal, void* userData)
        {
            const Il2CppClass* newParent = ResolvePairedClass(oldReal->parent);
            if (newParent == NULL)
                return NULL;
            Il2CppClass* newParentMutable = const_cast<Il2CppClass*>(newParent);
            il2cpp::vm::Class::SetupProperties(newParentMutable);
            for (uint16_t i = 0; i < newParentMutable->property_count; i++)
            {
                if (std::strcmp(newParentMutable->properties[i].name, oldReal->name) == 0)
                    return &newParentMutable->properties[i];
            }
            return NULL;
        }

        const EventInfo* PairEventByName(const EventInfo* oldReal, void* userData)
        {
            const Il2CppClass* newParent = ResolvePairedClass(oldReal->parent);
            if (newParent == NULL)
                return NULL;
            Il2CppClass* newParentMutable = const_cast<Il2CppClass*>(newParent);
            il2cpp::vm::Class::SetupEvents(newParentMutable);
            for (uint16_t i = 0; i < newParentMutable->event_count; i++)
            {
                if (std::strcmp(newParentMutable->events[i].name, oldReal->name) == 0)
                    return &newParentMutable->events[i];
            }
            return NULL;
        }
    }

    // 前置声明（定义在下方）：成员/Type 重绑
    void RemapMembersForReloadedClasses(ReloadPairContext& ctx);
    void RemapTypesForReloadedClasses(ReloadPairContext& ctx);

    void OnInterpreterAssemblyReloaded(const Il2CppAssembly* oldAssembly, const Il2CppAssembly* newAssembly)
    {
        if (oldAssembly == NULL || newAssembly == NULL)
            return;
        if (!hybridclr::metadata::IsInterpreterImage(oldAssembly->image) || !hybridclr::metadata::IsInterpreterImage(newAssembly->image))
            return;

        hybridclr::metadata::InterpreterImage* oldImage = (hybridclr::metadata::InterpreterImage*)hybridclr::metadata::MetadataModule::GetImage(oldAssembly->image);
        hybridclr::metadata::InterpreterImage* newImage = (hybridclr::metadata::InterpreterImage*)hybridclr::metadata::MetadataModule::GetImage(newAssembly->image);
        if (oldImage == NULL || newImage == NULL)
            return;

        ReloadPairContext ctx = { oldImage, newImage, oldAssembly->image, NULL };

        // 1. old 标志：给 real 属旧 image 的类 adapter 打标（安全网）
        GetClassAdapterMapForReload().MarkRealsWhere(ClassBelongsToOldImage, &ctx);

        // 2. 类批量重绑（按全名配对；RemapRealLocked 内清除 old 标志）
        GetClassAdapterMapForReload().RemapRealsWhere(ClassBelongsToOldImage, PairClassByName, &ctx);

        // 3. 成员重绑：字段/方法/属性/事件按 (parent 配对结果, 名字) 配对
        RemapMembersForReloadedClasses(ctx);

        // 4. Type 重绑：配对成功的类，byval_arg 内容键切换
        RemapTypesForReloadedClasses(ctx);

        // 5. 校验：仍带 old 标志的类 = 新版本中删除/改名，real 指向保活旧对象
        std::vector<const Il2CppClass*> staleClasses;
        GetClassAdapterMapForReload().GetStaleReals(staleClasses);
        for (size_t i = 0; i < staleClasses.size(); i++)
        {
            std::string name;
            BuildClassFullName(staleClasses[i], name);
            // TODO: 接日志输出未配对清单（stale adapter 优雅降级，real 保活）
            (void)name;
        }

        delete ctx.nameIndex;
    }

    // 成员重绑：四个成员 map 各自批量（filter=parent 属旧 image；pair=新类内按名/签名）
    void RemapMembersForReloadedClasses(ReloadPairContext& ctx)
    {
        GetFieldAdapterMapForReload().RemapRealsWhere(FieldBelongsToOldImage, PairFieldByName, &ctx);
        GetMethodAdapterMapForReload().RemapRealsWhere(MethodBelongsToOldImage, PairMethodByNameAndSignature, &ctx);
        GetPropertyAdapterMapForReload().RemapRealsWhere(PropertyBelongsToOldImage, PairPropertyByName, &ctx);
        GetEventAdapterMapForReload().RemapRealsWhere(EventBelongsToOldImage, PairEventByName, &ctx);
    }

    // Type 重绑：遍历旧 image 已实例化类，配对成功即切 byval_arg 内容键。
    // （Type map 是内容键，无法用 filter/pairFn 模式——旧 byval_arg 的 data 指针
    // 指旧 klass，直接按配对结果逐个 RemapTypeReal。）
    void RemapTypesForReloadedClasses(ReloadPairContext& ctx)
    {
        const std::vector<Il2CppClass*>& classList = ctx.oldImage->GetLoadedClassList();
        for (size_t i = 0; i < classList.size(); i++)
        {
            Il2CppClass* oldK = classList[i];
            if (oldK == NULL)
                continue;
            const Il2CppClass* newK = ResolvePairedClass(oldK);
            if (newK != NULL && newK != oldK)
                RemapTypeReal(&oldK->byval_arg, &newK->byval_arg);
        }
    }
}
}
