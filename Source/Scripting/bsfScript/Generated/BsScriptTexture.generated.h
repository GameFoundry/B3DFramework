//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "../../../Foundation/bsfUtility/Image/BsColor.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"

namespace bs { class TextureEx; }
namespace bs { class Texture; }
namespace bs
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
		static void InternalCreate(MonoObject* scriptObject, PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, TextureType texType, TextureUsage usage, uint32_t numSamples, bool hasMipmaps, bool gammaCorrection);
		static PixelFormat InternalGetPixelFormat(ScriptTexture* self);
		static TextureUsage InternalGetUsage(ScriptTexture* self);
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
