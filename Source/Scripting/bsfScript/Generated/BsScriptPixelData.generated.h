//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPixelData : public TScriptReflectable<ScriptPixelData, PixelData>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "PixelData")

		ScriptPixelData(MonoObject* managedInstance, const SPtr<PixelData>& value);

		static MonoObject* Create(const SPtr<PixelData>& value);

	private:
		static uint32_t InternalGetRowPitch(ScriptPixelData* self);
		static uint32_t InternalGetSlicePitch(ScriptPixelData* self);
		static PixelFormat InternalGetFormat(ScriptPixelData* self);
		static void InternalGetExtents(ScriptPixelData* self, PixelVolume* __output);
		static bool InternalIsConsecutive(ScriptPixelData* self);
		static uint32_t InternalGetSize(ScriptPixelData* self);
		static void InternalCreate(MonoObject* managedInstance, PixelVolume* volume, PixelFormat format);
		static void InternalCreate0(MonoObject* managedInstance, uint32_t width, uint32_t height, uint32_t depth, PixelFormat pixelFormat);
		static void InternalGetPixel(ScriptPixelData* self, int32_t x, int32_t y, int32_t z, Color* __output);
		static void InternalSetPixel(ScriptPixelData* self, Color* value, int32_t x, int32_t y, int32_t z);
		static MonoArray* InternalGetPixels(ScriptPixelData* self);
		static void InternalSetPixels(ScriptPixelData* self, MonoArray* value);
		static MonoArray* InternalGetRawPixels(ScriptPixelData* self);
		static void InternalSetRawPixels(ScriptPixelData* self, MonoArray* value);
	};
}
