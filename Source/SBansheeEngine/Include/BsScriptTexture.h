//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsPixelData.h"
#include "BsTexture.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Texture. */
	class BS_SCR_BE_EXPORT ScriptTexture : public TScriptResource <ScriptTexture, Texture>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Texture")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* createInstance();

	private:
		friend class ScriptResourceManager;

		ScriptTexture(MonoObject* instance, const HTexture& texture);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_createInstance(MonoObject* instance, PixelFormat format, UINT32 width,
			UINT32 height, UINT32 depth, TextureType texType, TextureUsage usage, UINT32 numSamples, bool hasMipmaps, 
			bool gammaCorrection);
		static void internal_getPixelFormat(ScriptTexture* thisPtr, PixelFormat* value);
		static void internal_getUsage(ScriptTexture* thisPtr, TextureUsage* value);
		static void internal_getWidth(ScriptTexture* thisPtr, int* value);
		static void internal_getHeight(ScriptTexture* thisPtr, int* value);
		static void internal_getGammaCorrection(ScriptTexture* thisPtr, bool* value);
		static void internal_getSampleCount(ScriptTexture* thisPtr, int* value);
		static void internal_getMipmapCount(ScriptTexture* thisPtr, int* value);
		static MonoObject* internal_getPixels(ScriptTexture* thisPtr, UINT32 face, UINT32 mipLevel);
		static MonoObject* internal_getGPUPixels(ScriptTexture* thisPtr, UINT32 face, UINT32 mipLevel);
		static void internal_setPixels(ScriptTexture* thisPtr, MonoObject* data, UINT32 face, UINT32 mipLevel);
		static void internal_setPixelsArray(ScriptTexture* thisPtr, MonoArray* colors, UINT32 face, UINT32 mipLevel);
	};

	/** @} */
}