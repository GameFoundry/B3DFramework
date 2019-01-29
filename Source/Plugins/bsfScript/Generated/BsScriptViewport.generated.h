//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsViewport.h"
#include "Math/BsRect2.h"
#include "Math/BsRect2I.h"

namespace bs
{
	class Viewport;

	class BS_SCR_BE_EXPORT ScriptViewport : public ScriptObject<ScriptViewport>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Viewport")

		ScriptViewport(MonoObject* managedInstance, const SPtr<Viewport>& value);

		SPtr<Viewport> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<Viewport>& value);

	private:
		SPtr<Viewport> mInternal;

		static void Internal_setTarget(ScriptViewport* thisPtr, MonoObject* target);
		static MonoObject* Internal_getTarget(ScriptViewport* thisPtr);
		static void Internal_setArea(ScriptViewport* thisPtr, Rect2* area);
		static void Internal_getArea(ScriptViewport* thisPtr, Rect2* __output);
		static void Internal_getPixelArea(ScriptViewport* thisPtr, Rect2I* __output);
		static void Internal_setClearFlags(ScriptViewport* thisPtr, ClearFlagBits flags);
		static ClearFlagBits Internal_getClearFlags(ScriptViewport* thisPtr);
		static void Internal_setClearColorValue(ScriptViewport* thisPtr, Color* color);
		static void Internal_getClearColorValue(ScriptViewport* thisPtr, Color* __output);
		static void Internal_setClearDepthValue(ScriptViewport* thisPtr, float depth);
		static float Internal_getClearDepthValue(ScriptViewport* thisPtr);
		static void Internal_setClearStencilValue(ScriptViewport* thisPtr, uint16_t value);
		static uint16_t Internal_getClearStencilValue(ScriptViewport* thisPtr);
		static void Internal_create(MonoObject* managedInstance, MonoObject* target, float x, float y, float width, float height);
	};
}
