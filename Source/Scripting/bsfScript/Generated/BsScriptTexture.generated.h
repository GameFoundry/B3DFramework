//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"

namespace bs
{
	class TextureEx;
}

namespace bs
{
	class Texture;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptTexture : public TScriptResource<ScriptTexture, Texture>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Texture")

		ScriptTexture(MonoObject* managedInstance, const ResourceHandle<Texture>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptTexture* thisPtr);

		static MonoObject* InternalReadData(ScriptTexture* thisPtr, uint32_t face, uint32_t mipLevel);
		static void InternalCreate(MonoObject* managedInstance, PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, TextureType texType, TextureUsage usage, uint32_t numSamples, bool hasMipmaps, bool gammaCorrection);
		static PixelFormat InternalGetPixelFormat(ScriptTexture* thisPtr);
		static TextureUsage InternalGetUsage(ScriptTexture* thisPtr);
		static TextureType InternalGetType(ScriptTexture* thisPtr);
		static uint32_t InternalGetWidth(ScriptTexture* thisPtr);
		static uint32_t InternalGetHeight(ScriptTexture* thisPtr);
		static uint32_t InternalGetDepth(ScriptTexture* thisPtr);
		static bool InternalGetGammaCorrection(ScriptTexture* thisPtr);
		static uint32_t InternalGetSampleCount(ScriptTexture* thisPtr);
		static uint32_t InternalGetMipmapCount(ScriptTexture* thisPtr);
		static MonoObject* InternalGetPixels(ScriptTexture* thisPtr, uint32_t face, uint32_t mipLevel);
		static void InternalSetPixels(ScriptTexture* thisPtr, MonoObject* data, uint32_t face, uint32_t mipLevel);
		static void InternalSetPixelsArray(ScriptTexture* thisPtr, MonoArray* colors, uint32_t face, uint32_t mipLevel);
	};
} // namespace bs
