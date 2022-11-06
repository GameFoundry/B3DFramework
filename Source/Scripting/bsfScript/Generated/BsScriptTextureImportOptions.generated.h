//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Importer/BsTextureImportOptions.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"

namespace bs { class TextureImportOptions; }
namespace bs
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTextureImportOptions : public TScriptReflectable<ScriptTextureImportOptions, TextureImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "TextureImportOptions")

		ScriptTextureImportOptions(MonoObject* managedInstance, const SPtr<TextureImportOptions>& value);

		static MonoObject* Create(const SPtr<TextureImportOptions>& value);

	private:
		static PixelFormat InternalGetFormat(ScriptTextureImportOptions* thisPtr);
		static void InternalSetFormat(ScriptTextureImportOptions* thisPtr, PixelFormat value);
		static bool InternalGetGenerateMips(ScriptTextureImportOptions* thisPtr);
		static void InternalSetGenerateMips(ScriptTextureImportOptions* thisPtr, bool value);
		static uint32_t InternalGetMaxMip(ScriptTextureImportOptions* thisPtr);
		static void InternalSetMaxMip(ScriptTextureImportOptions* thisPtr, uint32_t value);
		static bool InternalGetCpuCached(ScriptTextureImportOptions* thisPtr);
		static void InternalSetCpuCached(ScriptTextureImportOptions* thisPtr, bool value);
		static bool InternalGetSRgb(ScriptTextureImportOptions* thisPtr);
		static void InternalSetSRgb(ScriptTextureImportOptions* thisPtr, bool value);
		static bool InternalGetCubemap(ScriptTextureImportOptions* thisPtr);
		static void InternalSetCubemap(ScriptTextureImportOptions* thisPtr, bool value);
		static CubemapSourceType InternalGetCubemapSourceType(ScriptTextureImportOptions* thisPtr);
		static void InternalSetCubemapSourceType(ScriptTextureImportOptions* thisPtr, CubemapSourceType value);
		static void InternalCreate(MonoObject* managedInstance);
	};
#endif
}
