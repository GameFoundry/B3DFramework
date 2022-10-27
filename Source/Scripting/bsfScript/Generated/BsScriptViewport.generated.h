//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsViewport.h"
#include "Image/BsColor.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsViewport.h"
#include "Math/BsRect2.h"
#include "Math/BsRect2I.h"

namespace bs
{
	class Viewport;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptViewport : public TScriptReflectable<ScriptViewport, Viewport>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Viewport")

		ScriptViewport(MonoObject* managedInstance, const SPtr<Viewport>& value);

		static MonoObject* Create(const SPtr<Viewport>& value);

	private:
		static void InternalSetTarget(ScriptViewport* thisPtr, MonoObject* target);
		static MonoObject* InternalGetTarget(ScriptViewport* thisPtr);
		static void InternalSetArea(ScriptViewport* thisPtr, Rect2* area);
		static void InternalGetArea(ScriptViewport* thisPtr, Rect2* __output);
		static void InternalGetPixelArea(ScriptViewport* thisPtr, Rect2I* __output);
		static void InternalSetClearFlags(ScriptViewport* thisPtr, ClearFlagBits flags);
		static ClearFlagBits InternalGetClearFlags(ScriptViewport* thisPtr);
		static void InternalSetClearColorValue(ScriptViewport* thisPtr, Color* color);
		static void InternalGetClearColorValue(ScriptViewport* thisPtr, Color* __output);
		static void InternalSetClearDepthValue(ScriptViewport* thisPtr, float depth);
		static float InternalGetClearDepthValue(ScriptViewport* thisPtr);
		static void InternalSetClearStencilValue(ScriptViewport* thisPtr, uint16_t value);
		static uint16_t InternalGetClearStencilValue(ScriptViewport* thisPtr);
		static void InternalCreate(MonoObject* managedInstance, MonoObject* target, float x, float y, float width, float height);
	};
} // namespace bs
