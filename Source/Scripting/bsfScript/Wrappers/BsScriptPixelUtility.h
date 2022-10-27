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
	class BS_SCR_BE_EXPORT ScriptPixelUtility : public ScriptObject<ScriptPixelUtility>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "PixelUtility")

	private:
		ScriptPixelUtility(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void InternalGetMemorySize(u32 width, u32 height, u32 depth, PixelFormat format, u32* value);
		static void InternalHasAlpha(PixelFormat format, bool* value);
		static void InternalIsFloatingPoint(PixelFormat format, bool* value);
		static void InternalIsCompressed(PixelFormat format, bool* value);
		static void InternalIsDepth(PixelFormat format, bool* value);
		static void InternalGetMaxMipmaps(u32 width, u32 height, u32 depth, PixelFormat format, u32* value);
		static MonoObject* InternalConvertFormat(MonoObject* source, PixelFormat newFormat);
		static MonoObject* InternalCompress(MonoObject* source, CompressionOptions* options);
		static MonoArray* InternalGenerateMipmaps(MonoObject* source, MipMapGenOptions* options);
		static MonoObject* InternalScale(MonoObject* source, PixelVolume* newSize, PixelUtil::Filter filter);
		static void InternalLinearToSrgb(MonoObject* source);
		static void InternalSrgbToLinear(MonoObject* source);
	};

	/** @} */
} // namespace bs
