//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Image/B3DPixelData.h"
#include "../../../Engine/Utility/Image/B3DColor.h"
#include "../../../Engine/Core/Image/B3DPixelData.h"
#include "../../../Engine/Core/Image/B3DPixelVolume.h"

namespace b3d { class PixelData; }
namespace b3d { class PixelDataEx; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPixelData : public TScriptReflectableWrapper<PixelData, ScriptPixelData>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PixelData")

		ScriptPixelData(const TShared<PixelData>& nativeObject);
		~ScriptPixelData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static uint32_t InternalGetRowPitch(ScriptPixelData* self);
		static uint32_t InternalGetSlicePitch(ScriptPixelData* self);
		static PixelFormat InternalGetFormat(ScriptPixelData* self);
		static void InternalGetExtents(ScriptPixelData* self, PixelVolume* __output);
		static bool InternalIsConsecutive(ScriptPixelData* self);
		static uint32_t InternalGetSize(ScriptPixelData* self);
		static void InternalCreate(MonoObject* scriptObject, PixelVolume* volume, PixelFormat format);
		static void InternalCreate0(MonoObject* scriptObject, uint32_t width, uint32_t height, uint32_t depth, PixelFormat pixelFormat);
		static void InternalGetPixel(ScriptPixelData* self, int32_t x, int32_t y, int32_t z, Color* __output);
		static void InternalSetPixel(ScriptPixelData* self, Color* value, int32_t x, int32_t y, int32_t z);
		static MonoArray* InternalGetPixels(ScriptPixelData* self);
		static void InternalSetPixels(ScriptPixelData* self, MonoArray* value);
		static MonoArray* InternalGetRawPixels(ScriptPixelData* self);
		static void InternalSetRawPixels(ScriptPixelData* self, MonoArray* value);
	};
}
