//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"
#include "../../../Foundation/bsfCore/Image/BsPixelVolume.h"

namespace bs { class PixelData; }
namespace bs { class PixelDataEx; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPixelData : public TScriptReflectable<ScriptPixelData, PixelData>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "PixelData")

		ScriptPixelData(MonoObject* managedInstance, const SPtr<PixelData>& value);

		static MonoObject* create(const SPtr<PixelData>& value);

	private:
		static uint32_t Internal_getRowPitch(ScriptPixelData* thisPtr);
		static uint32_t Internal_getSlicePitch(ScriptPixelData* thisPtr);
		static PixelFormat Internal_getFormat(ScriptPixelData* thisPtr);
		static void Internal_getExtents(ScriptPixelData* thisPtr, PixelVolume* __output);
		static bool Internal_isConsecutive(ScriptPixelData* thisPtr);
		static uint32_t Internal_getSize(ScriptPixelData* thisPtr);
		static void Internal_create(MonoObject* managedInstance, PixelVolume* volume, PixelFormat format);
		static void Internal_create0(MonoObject* managedInstance, uint32_t width, uint32_t height, uint32_t depth, PixelFormat pixelFormat);
		static void Internal_getPixel(ScriptPixelData* thisPtr, int32_t x, int32_t y, int32_t z, Color* __output);
		static void Internal_setPixel(ScriptPixelData* thisPtr, Color* value, int32_t x, int32_t y, int32_t z);
		static MonoArray* Internal_getPixels(ScriptPixelData* thisPtr);
		static void Internal_setPixels(ScriptPixelData* thisPtr, MonoArray* value);
		static MonoArray* Internal_getRawPixels(ScriptPixelData* thisPtr);
		static void Internal_setRawPixels(ScriptPixelData* thisPtr, MonoArray* value);
	};
}
