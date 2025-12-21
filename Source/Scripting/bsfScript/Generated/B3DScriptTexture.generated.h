//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptResourceWrapper.h"
#include "../../../Foundation/Core/Utility/B3DCommonTypes.h"
#include "../../../Foundation/Utility/Image/B3DColor.h"
#include "../../../Foundation/Core/Image/B3DPixelData.h"

namespace b3d { class Texture; }
namespace b3d { class TextureEx; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptTexture : public TScriptResourceWrapper<Texture, ScriptTexture>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Texture")

		ScriptTexture(const TResourceHandle<Texture>& nativeObject);
		~ScriptTexture();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptTexture* self);

		static MonoObject* InternalReadData(ScriptTexture* self, uint32_t face, uint32_t mipLevel);
		static void InternalCreate(MonoObject* scriptObject, PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, TextureType texType, int32_t usage, uint32_t numSamples, bool hasMipmaps, bool gammaCorrection);
		static PixelFormat InternalGetPixelFormat(ScriptTexture* self);
		static int32_t InternalGetUsage(ScriptTexture* self);
		static TextureType InternalGetType(ScriptTexture* self);
		static uint32_t InternalGetWidth(ScriptTexture* self);
		static uint32_t InternalGetHeight(ScriptTexture* self);
		static uint32_t InternalGetDepth(ScriptTexture* self);
		static bool InternalGetGammaCorrection(ScriptTexture* self);
		static uint32_t InternalGetSampleCount(ScriptTexture* self);
		static uint32_t InternalGetMipmapCount(ScriptTexture* self);
		static MonoObject* InternalGetPixels(ScriptTexture* self, uint32_t face, uint32_t mipLevel);
		static void InternalSetPixels(ScriptTexture* self, MonoObject* data, uint32_t face, uint32_t mipLevel);
		static void InternalSetPixelsArray(ScriptTexture* self, MonoArray* colors, uint32_t face, uint32_t mipLevel);
	};
}
