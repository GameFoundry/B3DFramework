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
		static PixelFormat InternalGetFormat(ScriptTextureImportOptions* self);
		static void InternalSetFormat(ScriptTextureImportOptions* self, PixelFormat value);
		static bool InternalGetGenerateMips(ScriptTextureImportOptions* self);
		static void InternalSetGenerateMips(ScriptTextureImportOptions* self, bool value);
		static uint32_t InternalGetMaxMip(ScriptTextureImportOptions* self);
		static void InternalSetMaxMip(ScriptTextureImportOptions* self, uint32_t value);
		static bool InternalGetCpuCached(ScriptTextureImportOptions* self);
		static void InternalSetCpuCached(ScriptTextureImportOptions* self, bool value);
		static bool InternalGetSRgb(ScriptTextureImportOptions* self);
		static void InternalSetSRgb(ScriptTextureImportOptions* self, bool value);
		static bool InternalGetCubemap(ScriptTextureImportOptions* self);
		static void InternalSetCubemap(ScriptTextureImportOptions* self, bool value);
		static CubemapSourceType InternalGetCubemapSourceType(ScriptTextureImportOptions* self);
		static void InternalSetCubemapSourceType(ScriptTextureImportOptions* self, CubemapSourceType value);
		static void InternalCreate(MonoObject* managedInstance);
	};
#endif
}
