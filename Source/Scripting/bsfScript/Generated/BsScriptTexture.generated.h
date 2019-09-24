//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"

namespace bs { class TextureEx; }
namespace bs { class Texture; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptTexture : public TScriptResource<ScriptTexture, Texture>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Texture")

		ScriptTexture(MonoObject* managedInstance, const ResourceHandle<Texture>& value);

		static MonoObject* createInstance();

	private:
		static MonoObject* Internal_getRef(ScriptTexture* thisPtr);

		static MonoObject* Internal_readData(ScriptTexture* thisPtr, uint32_t face, uint32_t mipLevel);
		static void Internal_create(MonoObject* managedInstance, PixelFormat format, uint32_t width, uint32_t height, uint32_t depth, TextureType texType, TextureUsage usage, uint32_t numSamples, bool hasMipmaps, bool gammaCorrection);
		static PixelFormat Internal_getPixelFormat(ScriptTexture* thisPtr);
		static TextureUsage Internal_getUsage(ScriptTexture* thisPtr);
		static TextureType Internal_getType(ScriptTexture* thisPtr);
		static uint32_t Internal_getWidth(ScriptTexture* thisPtr);
		static uint32_t Internal_getHeight(ScriptTexture* thisPtr);
		static uint32_t Internal_getDepth(ScriptTexture* thisPtr);
		static bool Internal_getGammaCorrection(ScriptTexture* thisPtr);
		static uint32_t Internal_getSampleCount(ScriptTexture* thisPtr);
		static uint32_t Internal_getMipmapCount(ScriptTexture* thisPtr);
		static MonoObject* Internal_getPixels(ScriptTexture* thisPtr, uint32_t face, uint32_t mipLevel);
		static void Internal_setPixels(ScriptTexture* thisPtr, MonoObject* data, uint32_t face, uint32_t mipLevel);
		static void Internal_setPixelsArray(ScriptTexture* thisPtr, MonoArray* colors, uint32_t face, uint32_t mipLevel);
	};
}
