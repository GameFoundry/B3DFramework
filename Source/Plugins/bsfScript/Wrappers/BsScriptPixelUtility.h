//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Image/BsPixelData.h"
#include "Image/BsPixelUtil.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for PixelUtility. */
	class BS_SCR_BE_EXPORT ScriptPixelUtility : public ScriptObject <ScriptPixelUtility>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "PixelUtility")

	private:
		ScriptPixelUtility(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_getMemorySize(UINT32 width, UINT32 height, UINT32 depth, PixelFormat format, UINT32* value);
		static void internal_hasAlpha(PixelFormat format, bool* value);
		static void internal_isFloatingPoint(PixelFormat format, bool* value);
		static void internal_isCompressed(PixelFormat format, bool* value);
		static void internal_isDepth(PixelFormat format, bool* value);
		static void internal_getMaxMipmaps(UINT32 width, UINT32 height, UINT32 depth, PixelFormat format, UINT32* value);
		static MonoObject* internal_convertFormat(MonoObject* source, PixelFormat newFormat);
		static MonoObject* internal_compress(MonoObject* source, CompressionOptions* options);
		static MonoArray* internal_generateMipmaps(MonoObject* source, MipMapGenOptions* options);
		static MonoObject* internal_scale(MonoObject* source, PixelVolume* newSize, PixelUtil::Filter filter);
		static void internal_linearToSRGB(MonoObject* source);
		static void internal_SRGBToLinear(MonoObject* source);
	};

	/** @} */
}