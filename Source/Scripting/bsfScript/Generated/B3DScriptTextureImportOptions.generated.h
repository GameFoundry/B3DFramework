//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptImportOptions.generated.h"
#include "../../../Engine/Core/Importer/B3DTextureImportOptions.h"
#include "../../../Engine/Core/Image/B3DPixelData.h"
#include "../../../Engine/Core/Utility/B3DCommonTypes.h"

namespace b3d { class TextureImportOptions; }
namespace b3d
{
#if !B3D_IS_ENGINE
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTextureImportOptions : public TScriptReflectableWrapper<TextureImportOptions, ScriptTextureImportOptions, ScriptImportOptionsWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "TextureImportOptions")

		ScriptTextureImportOptions(const TShared<TextureImportOptions>& nativeObject);
		~ScriptTextureImportOptions();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

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
		static void InternalCreate(MonoObject* scriptObject);
	};
#endif
}
