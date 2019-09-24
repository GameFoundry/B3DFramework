//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
#if !BS_IS_BANSHEE3D
	class BS_SCR_BE_EXPORT ScriptTextureImportOptions : public TScriptReflectable<ScriptTextureImportOptions, TextureImportOptions, ScriptImportOptionsBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "TextureImportOptions")

		ScriptTextureImportOptions(MonoObject* managedInstance, const SPtr<TextureImportOptions>& value);

		static MonoObject* create(const SPtr<TextureImportOptions>& value);

	private:
		static PixelFormat Internal_getformat(ScriptTextureImportOptions* thisPtr);
		static void Internal_setformat(ScriptTextureImportOptions* thisPtr, PixelFormat value);
		static bool Internal_getgenerateMips(ScriptTextureImportOptions* thisPtr);
		static void Internal_setgenerateMips(ScriptTextureImportOptions* thisPtr, bool value);
		static uint32_t Internal_getmaxMip(ScriptTextureImportOptions* thisPtr);
		static void Internal_setmaxMip(ScriptTextureImportOptions* thisPtr, uint32_t value);
		static bool Internal_getcpuCached(ScriptTextureImportOptions* thisPtr);
		static void Internal_setcpuCached(ScriptTextureImportOptions* thisPtr, bool value);
		static bool Internal_getsRGB(ScriptTextureImportOptions* thisPtr);
		static void Internal_setsRGB(ScriptTextureImportOptions* thisPtr, bool value);
		static bool Internal_getcubemap(ScriptTextureImportOptions* thisPtr);
		static void Internal_setcubemap(ScriptTextureImportOptions* thisPtr, bool value);
		static CubemapSourceType Internal_getcubemapSourceType(ScriptTextureImportOptions* thisPtr);
		static void Internal_setcubemapSourceType(ScriptTextureImportOptions* thisPtr, CubemapSourceType value);
		static void Internal_create(MonoObject* managedInstance);
	};
#endif
}
