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

	/**	Base class for all interop objects referencing textures. */
	class BS_SCR_BE_EXPORT ScriptTextureBase : public ScriptResourceBase
	{
	protected:
		friend class ScriptResourceManager;

		ScriptTextureBase(MonoObject* instance)
			:ScriptResourceBase(instance)
		{ }

		virtual ~ScriptTextureBase() {}
	};

	/**	Interop class between C++ & CLR for Texture. */
	class BS_SCR_BE_EXPORT ScriptTexture : public ScriptObject <ScriptTexture, ScriptTextureBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Texture")

	private:
		ScriptTexture(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_getPixelFormat(ScriptTexture* thisPtr, PixelFormat* value);
		static void internal_getUsage(ScriptTexture* thisPtr, TextureUsage* value);
		static void internal_getWidth(ScriptTexture* thisPtr, int* value);
		static void internal_getHeight(ScriptTexture* thisPtr, int* value);
		static void internal_getGammaCorrection(ScriptTexture* thisPtr, bool* value);
		static void internal_getSampleCount(ScriptTexture* thisPtr, int* value);
		static void internal_getMipmapCount(ScriptTexture* thisPtr, int* value);
	};

	/** @} */
}