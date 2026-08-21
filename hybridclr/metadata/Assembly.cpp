
#include "Assembly.h"

#include <cstring>
#include <iostream>
#include <vector>

#include "os/File.h"
#include "utils/MemoryMappedFile.h"
#include "vm/Assembly.h"
#include "vm/Image.h"
#include "vm/Class.h"
#include "vm/String.h"
#include "vm/MetadataLock.h"
#include "vm/MetadataCache.h"
#include "vm/GenericClass.h"

#include "Image.h"
#include "MetadataModule.h"
#include "MetadataUtil.h"
#include "ConsistentAOTHomologousImage.h"
#include "SuperSetAOTHomologousImage.h"
#include "../ReloadDiagLog.h"

namespace hybridclr
{
namespace metadata
{

    std::vector<Il2CppAssembly*> s_placeHolderAssembies;

#if ENABLE_PLACEHOLDER_DLL == 1

    static const char* CreateAssemblyNameWithoutExt(const char* assemblyName)
    {
        const char* extStr = std::strstr(assemblyName, ".dll");
        if (extStr)
        {
            size_t nameLen = extStr - assemblyName;
            char* name = (char*)HYBRIDCLR_MALLOC(nameLen + 1);
            std::strncpy(name, assemblyName, nameLen);
            name[nameLen] = '\0';
            return name;
        }
        else
        {
            return CopyString(assemblyName);
        }
    }

    static Il2CppAssembly* CreatePlaceHolderAssembly(const char* assemblyName)
    {
        auto ass = new (HYBRIDCLR_MALLOC_ZERO(sizeof(Il2CppAssembly))) Il2CppAssembly;
        auto image2 = new (HYBRIDCLR_MALLOC_ZERO(sizeof(Il2CppImage))) Il2CppImage;
        ass->image = image2;
        ass->image->name = CopyString(assemblyName);
        ass->image->nameNoExt = ass->aname.name = CreateAssemblyNameWithoutExt(assemblyName);
        image2->assembly = ass;
        s_placeHolderAssembies.push_back(ass);
        return ass;
    }

    static Il2CppAssembly* FindPlaceHolderAssembly(const char* assemblyNameNoExt)
    {
        for (Il2CppAssembly* ass : s_placeHolderAssembies)
        {
            if (std::strcmp(ass->image->nameNoExt, assemblyNameNoExt) == 0)
            {
                return ass;
            }
        }
        return nullptr;
    }

    static void ReplacePlaceHolderAssembly(Il2CppAssembly* oldAss, Il2CppAssembly* newAss)
    {
        for (size_t i = 0; i < s_placeHolderAssembies.size(); ++i)
        {
            if (s_placeHolderAssembies[i] == oldAss)
            {
                s_placeHolderAssembies[i] = newAss;
                return;
            }
        }
    }
#else
    static Il2CppAssembly* FindPlaceHolderAssembly(const char* assemblyNameNoExt)
    {
        return nullptr;
    }

    static void ReplacePlaceHolderAssembly(Il2CppAssembly* oldAss, Il2CppAssembly* newAss)
    {
    }
#endif

    void Assembly::InitializePlaceHolderAssemblies()
    {
        for (const char** ptrPlaceHolderName = g_placeHolderAssemblies; *ptrPlaceHolderName; ++ptrPlaceHolderName)
        {
            const char* nameWithExtension = ConcatNewString(*ptrPlaceHolderName, ".dll");
            Il2CppAssembly* placeHolderAss = CreatePlaceHolderAssembly(nameWithExtension);
            HYBRIDCLR_FREE((void*)nameWithExtension);
            il2cpp::vm::MetadataCache::RegisterInterpreterAssembly(placeHolderAss);
        }
    }

    static void RunModuleInitializer(Il2CppImage* image)
    {
        Il2CppClass* moduleKlass = il2cpp::vm::Image::ClassFromName(image, "", "<Module>");
        if (!moduleKlass)
        {
            return;
        }
        il2cpp::vm::Runtime::ClassInit(moduleKlass);
    }

    Il2CppAssembly* Assembly::LoadFromBytes(const void* assemblyData, uint64_t length, const void* rawSymbolStoreBytes, uint64_t rawSymbolStoreLength)
    {
        Il2CppAssembly* ass = Create((const byte*)assemblyData, length, (const byte*)rawSymbolStoreBytes, rawSymbolStoreLength);
        RunModuleInitializer(ass->image);
        return ass;
    }

    Il2CppAssembly* Assembly::Create(const byte* assemblyData, uint64_t length, const byte* rawSymbolStoreBytes, uint64_t rawSymbolStoreLength)
    {
        il2cpp::os::FastAutoLock lock(&il2cpp::vm::g_MetadataLock);

        if (!assemblyData)
        {
            il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetArgumentNullException("rawAssembly is null"));
        }

        uint32_t imageId = InterpreterImage::AllocImageIndex((uint32_t)length);
        if (imageId == kInvalidImageIndex)
        {
            il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetExecutionEngineException("InterpreterImage::AllocImageIndex failed"));
        }
        InterpreterImage* image = new InterpreterImage(imageId);
        
        assemblyData = (const byte*)CopyBytes(assemblyData, length);
        LoadImageErrorCode err = image->Load(assemblyData, (size_t)length);

        if (err != LoadImageErrorCode::OK)
        {
            TEMP_FORMAT(errMsg, "LoadImageErrorCode:%d", (int)err);
            il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetBadImageFormatException(errMsg));
            // when load a bad image, mean a fatal error. we don't clean image on purpose.
        }

        if (rawSymbolStoreBytes)
        {
            rawSymbolStoreBytes = (const byte*)CopyBytes(rawSymbolStoreBytes, rawSymbolStoreLength);
            err = image->LoadPDB(rawSymbolStoreBytes, (size_t)rawSymbolStoreLength);
            if (err != LoadImageErrorCode::OK)
            {
                TEMP_FORMAT(errMsg, "LoadPDB Error:%d", (int)err);
                il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetBadImageFormatException(errMsg));
            }
        }

        TbAssembly data = image->GetRawImage().ReadAssembly(1);
        const char* nameNoExt = image->GetStringFromRawIndex(data.name);

        Il2CppAssembly* ass;
        Il2CppImage* image2;
        Il2CppAssembly* placeHolderAss = FindPlaceHolderAssembly(nameNoExt);
        // 需要被替换掉的旧程序集（重新加载同名程序集时非空）
        Il2CppAssembly* oldAss = nullptr;
        if (placeHolderAss != nullptr && !placeHolderAss->token)
        {
            // 占位程序集首次真正加载，复用预创建的 Il2CppAssembly/Il2CppImage
            ass = placeHolderAss;
            image2 = ass->image;
            HYBRIDCLR_FREE((void*)ass->image->name);
            HYBRIDCLR_FREE((void*)ass->image->nameNoExt);
        }
        else
        {
            // 普通加载，或重新加载同名程序集
            if (placeHolderAss != nullptr)
            {
                // 占位程序集已被加载过，本次为重新加载。
                // s_placeHolderAssembies 中的条目总是指向最近一次注册的同名程序集。
                oldAss = placeHolderAss;
            }
            else
            {
                // 非占位程序集也可能重复加载，逆序查找最近注册的同名解释器程序集
                oldAss = il2cpp::vm::MetadataCache::GetInterpreterAssemblyByName(nameNoExt);
            }
            // 注意：曾尝试"复用旧 Il2CppImage 指针"修复 MonoManager image 注册
            // （GetAssemblyIndexFromImage=-1 导致嵌套 managed 字段被剔除的 NRE），
            // 但泛型还原（RestoreCachedGenericClasses / RehashGenericClassSet）依赖
            // "新旧 image 是不同指针"来归一化陈旧元数据，复用后新旧同指针导致泛型
            // 实例 typeMetadataHandle 停留在旧元数据而越界崩溃，故回退为新建。
            ass = new (HYBRIDCLR_MALLOC_ZERO(sizeof(Il2CppAssembly))) Il2CppAssembly;
            image2 = new (HYBRIDCLR_MALLOC_ZERO(sizeof(Il2CppImage))) Il2CppImage;
        }

		image->InitBasic(image2);
		image->BuildIl2CppAssembly(ass);
		ass->image = image2;

		image->BuildIl2CppImage(image2);
		image2->name = ConcatNewString(ass->aname.name, ".dll");
		image2->nameNoExt = ass->aname.name;
		image2->assembly = ass;

		// ==={{ AssemblyReloadReuse
		// While collecting / restoring, the reload normalization & verify-repair
		// code in GenericClass.cpp must stay silent (class mutation/resolution
		// mid-pass can corrupt classes being initialized).
		il2cpp::vm::GenericClass::BeginReloadRestore();
		// If reloading an assembly with the same name, collect reusable
		// Il2CppClass / MethodInfo objects from the old image *before*
		// InitRuntimeMetadatas so the reuse maps are ready.
		if (oldAss != nullptr)
		{
			Il2CppImage* oldImage2 = oldAss->image;
			if (oldImage2 && hybridclr::metadata::IsInterpreterImage(oldImage2))
			{
				hybridclr::metadata::InterpreterImage* oldInterpImage =
					hybridclr::metadata::MetadataModule::GetImage(oldImage2);
				if (oldInterpImage)
				{
					// ==={{ AssemblyReloadReuse: pre-warm Unity-serialized
					// classes of the old image BEFORE collecting/reusing, so
					// their [SerializeField] fields deserialize correctly
					// after the reload (reload NRE investigation). ===
					oldInterpImage->PrewarmUnitySerializedClasses();
					// ===}} AssemblyReloadReuse
					image->CollectReusableObjects(oldInterpImage);
				}
			}
		}
		// ===}} AssemblyReloadReuse

		image->InitRuntimeMetadatas();

		// ==={{ AssemblyReloadReuse
		// After InitRuntimeMetadatas, restore reused Il2CppClass objects:
		// update their external pointers to the new image/assembly and reset
		// lazily-initialised fields.  This must happen before any class is
		// accessed (e.g. before RunModuleInitializer).
		if (image->HasReuseData())
		{
			image->RestoreReusedClasses();
			// ==={{ AssemblyReloadDiag: enable diagnostics that resolve
			// classes only AFTER all restore passes completed (class
			// creation is unsafe during the passes and during VM startup). ===
			ReloadDiagEnable();
			// From now on, normalize stale type_argv at generic member
			// inflation / cache lookup time (see GenericClass.cpp).
			il2cpp::vm::GenericClass::EnableReloadArgvNormalization();
			ReloadDiagLog("[ReloadDiag] DiagEnabled after restore: il2cppImage=%p(%s)\n",
				(void*)ass->image, ass->image ? ass->image->name : "?");
			// ===}} AssemblyReloadDiag
		}
		il2cpp::vm::GenericClass::EndReloadRestore();
		// ===}} AssemblyReloadReuse

        if (oldAss != nullptr)
        {
            // 重新加载：先把旧程序集从 s_cliAssemblies、s_Assemblies 等缓存中移除，
            // 再注册新程序集完成替换。
            il2cpp::vm::MetadataCache::UnregisterInterpreterAssembly(oldAss);
            // 更新占位程序集列表，保证后续重载能找到最新的同名程序集
            ReplacePlaceHolderAssembly(oldAss, ass);
            // 注意：旧的 Il2CppAssembly/Il2CppImage 及其元数据可能仍被已创建的
            // 类型/方法引用，这里有意不释放（泄漏换安全）。
        }

        il2cpp::vm::MetadataCache::RegisterInterpreterAssembly(ass);
        return ass;
    }

    LoadImageErrorCode Assembly::LoadMetadataForAOTAssembly(const void* dllBytes, uint32_t dllSize, HomologousImageMode mode)
    {
        il2cpp::os::FastAutoLock lock(&il2cpp::vm::g_MetadataLock);

        AOTHomologousImage* image = nullptr;
        switch (mode)
        {
        case HomologousImageMode::CONSISTENT: image = new ConsistentAOTHomologousImage(); break;
        case HomologousImageMode::SUPERSET: image = new SuperSetAOTHomologousImage(); break;
        default: return LoadImageErrorCode::INVALID_HOMOLOGOUS_MODE;
        }

        LoadImageErrorCode err = image->Load((byte*)CopyBytes(dllBytes, dllSize), dllSize);
        if (err != LoadImageErrorCode::OK)
        {
            delete image;
            return err;
        }

        RawImageBase* rawImage = &image->GetRawImage();
        TbAssembly data = rawImage->ReadAssembly(1);
        const char* assName = rawImage->GetStringFromRawIndex(data.name);
        const Il2CppAssembly* aotAss = il2cpp::vm::Assembly::GetLoadedAssembly(assName);
        // FIXME. not free memory.
        if (!aotAss)
        {
            delete image;
            return LoadImageErrorCode::AOT_ASSEMBLY_NOT_FIND;
        }
        if (hybridclr::metadata::IsInterpreterImage(aotAss->image))
        {
            delete image;
            return LoadImageErrorCode::HOMOLOGOUS_ONLY_SUPPORT_AOT_ASSEMBLY;
        }
        image->SetTargetAssembly(aotAss);
        if (AOTHomologousImage::FindImageByAssemblyLocked(image->GetTargetAssembly(), lock))
        {
            return LoadImageErrorCode::HOMOLOGOUS_ASSEMBLY_HAS_BEEN_LOADED;
        }
        image->InitRuntimeMetadatas();
        AOTHomologousImage::RegisterLocked(image, lock);
        return LoadImageErrorCode::OK;
    }


}
}

