//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsViewport.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsViewport.h"
#include "Math/BsRect2.h"
#include "Math/BsRect2I.h"

namespace bs { class Viewport; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptViewport : public TScriptReflectableWrapper<Viewport, ScriptViewport>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Viewport")

		ScriptViewport(const SPtr<Viewport>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetTarget(ScriptViewport* self, MonoObject* target);
		static MonoObject* InternalGetTarget(ScriptViewport* self);
		static void InternalSetArea(ScriptViewport* self, Rect2* area);
		static void InternalGetArea(ScriptViewport* self, Rect2* __output);
		static void InternalGetPixelArea(ScriptViewport* self, Rect2I* __output);
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
