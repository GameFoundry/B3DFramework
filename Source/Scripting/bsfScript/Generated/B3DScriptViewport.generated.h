//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/GpuBackend/B3DViewport.h"
#include "../../../Engine/Utility/Image/B3DColor.h"
#include "../../../Engine/Utility/Math/B3DArea2.h"
#include "../../../Engine/Utility/Math/B3DArea2.h"
#include "../../../Engine/Core/GpuBackend/B3DViewport.h"

namespace b3d { class Viewport; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptViewport : public TScriptReflectableWrapper<Viewport, ScriptViewport>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Viewport")

		ScriptViewport(const TShared<Viewport>& nativeObject);
		~ScriptViewport();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetTarget(ScriptViewport* self, MonoObject* target);
		static MonoObject* InternalGetTarget(ScriptViewport* self);
		static void InternalSetArea(ScriptViewport* self, TArea2<float, float>* area);
		static void InternalGetArea(ScriptViewport* self, TArea2<float, float>* __output);
		static void InternalGetPixelArea(ScriptViewport* self, TArea2<int32_t, uint32_t>* __output);
		static void InternalSetClearFlags(ScriptViewport* self, ClearFlagBits flags);
		static ClearFlagBits InternalGetClearFlags(ScriptViewport* self);
		static void InternalSetClearColorValue(ScriptViewport* self, Color* color);
		static void InternalGetClearColorValue(ScriptViewport* self, Color* __output);
		static void InternalSetClearDepthValue(ScriptViewport* self, float depth);
		static float InternalGetClearDepthValue(ScriptViewport* self);
		static void InternalSetClearStencilValue(ScriptViewport* self, uint16_t value);
		static uint16_t InternalGetClearStencilValue(ScriptViewport* self);
		static void InternalCreate(MonoObject* scriptObject, MonoObject* target, float x, float y, float width, float height);
	};
}
