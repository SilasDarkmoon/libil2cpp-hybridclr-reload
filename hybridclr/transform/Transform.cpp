
#include "Transform.h"

#include <unordered_set>

#include "TransformContext.h"

#include "../metadata/MethodBodyCache.h"

namespace hybridclr
{
namespace transform
{

	InterpMethodInfo* HiTransform::Transform(const MethodInfo* methodInfo)
	{
		TemporaryMemoryArena pool;

		metadata::Image* image = metadata::MetadataModule::GetUnderlyingInterpreterImage(methodInfo);
		IL2CPP_ASSERT(image);

		// ==={{ AssemblyReloadReuse: diagnostic log (filtered)
		{
			const char* kname = methodInfo->klass->name ? methodInfo->klass->name : "";
			// Only log for AsyncUniTaskMethodBuilder or when image/token mismatch is likely
			if (strstr(kname, "AsyncUniTaskMethodBuilder"))
			{
				uint32_t decodedImgIdx = hybridclr::metadata::DecodeImageIndex(methodInfo->klass->image->token);
				uint32_t rowIdx = hybridclr::metadata::DecodeTokenRowIndex(methodInfo->token);
				const std::string tmpCache = il2cpp::os::Environment::GetEnvironmentVariable("UNITY_TEMPORARY_CACHE_PATH");
				std::string dirStr = !tmpCache.empty() ? tmpCache : "log";
				int createError = 0;
				il2cpp::os::Directory::Create(dirStr, &createError);
				FILE* fp = fopen((dirStr + "/assembly_reload_reuse.log").c_str(), "a");
				if (fp) {
					fprintf(fp, "[ReuseDiag] HiTransform::Transform: klass='%s.%s' method='%s' klass_image_token=0x%08X decodedImgIdx=%u method_token=0x%08X rowIndex=%u interpImage=%p isInterpType=%d\n",
						methodInfo->klass->namespaze ? methodInfo->klass->namespaze : "",
						kname,
						methodInfo->name ? methodInfo->name : "",
						(unsigned)methodInfo->klass->image->token,
						decodedImgIdx,
						(unsigned)methodInfo->token,
						rowIdx,
						(void*)image,
						(int)hybridclr::metadata::IsInterpreterType(methodInfo->klass));
					fclose(fp);
				}
			}
		}
		// ===}} AssemblyReloadReuse

		metadata::MethodBodyCache::EnableShrinkMethodBodyCache(false);
		metadata::MethodBody* methodBody = metadata::MethodBodyCache::GetMethodBody(image, methodInfo->token);
		if (methodBody == nullptr || methodBody->ilcodes == nullptr)
		{
			// ==={{ AssemblyReloadReuse: diagnostic log for null method body
			{
				uint32_t rowIdx = hybridclr::metadata::DecodeTokenRowIndex(methodInfo->token);
				const std::string tmpCache = il2cpp::os::Environment::GetEnvironmentVariable("UNITY_TEMPORARY_CACHE_PATH");
				std::string dirStr = !tmpCache.empty() ? tmpCache : "log";
				int createError = 0;
				il2cpp::os::Directory::Create(dirStr, &createError);
				FILE* fp = fopen((dirStr + "/assembly_reload_reuse.log").c_str(), "a");
				if (fp) {
					fprintf(fp, "[ReuseDiag] MethodBody null: klass='%s.%s' method='%s' token=0x%08X rowIndex=%u image=%p klassImageName='%s' isInterpType=%d isInterpImpl=%d is_inflated=%d is_generic=%d\n",
						methodInfo->klass->namespaze ? methodInfo->klass->namespaze : "",
						methodInfo->klass->name ? methodInfo->klass->name : "",
						methodInfo->name ? methodInfo->name : "",
						(unsigned)methodInfo->token, rowIdx, (void*)image,
						methodInfo->klass->image && methodInfo->klass->image->name ? methodInfo->klass->image->name : "?",
						(int)hybridclr::metadata::IsInterpreterType(methodInfo->klass),
						(int)methodInfo->isInterpterImpl,
						(int)methodInfo->is_inflated,
						(int)methodInfo->is_generic);
					fclose(fp);
				}
			}
			// ===}} AssemblyReloadReuse
			TEMP_FORMAT(errMsg, "Method body is null. %s.%s::%s", methodInfo->klass->namespaze, methodInfo->klass->name, methodInfo->name);
			il2cpp::vm::Exception::Raise(il2cpp::vm::Exception::GetExecutionEngineException(errMsg));
		}
		InterpMethodInfo* result = new (HYBRIDCLR_METADATA_MALLOC(sizeof(InterpMethodInfo))) InterpMethodInfo;
		il2cpp::utils::dynamic_array<uint64_t> resolveDatas;
		TransformContext ctx(image, methodInfo, *methodBody, pool, resolveDatas);

		ctx.TransformBody(0, 0, *result);
		metadata::MethodBodyCache::EnableShrinkMethodBodyCache(true);
		return result;
	}
}

}
