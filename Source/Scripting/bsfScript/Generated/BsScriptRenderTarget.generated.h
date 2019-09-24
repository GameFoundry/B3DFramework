//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTarget.h"

namespace bs { class RenderTarget; }
namespace bs { class RenderTargetEx; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptRenderTargetBase : public ScriptReflectableBase
	{
	public:
		ScriptRenderTargetBase(MonoObject* instance);
		virtual ~ScriptRenderTargetBase() {}

		SPtr<RenderTarget> getInternal() const;
	};

	class BS_SCR_BE_EXPORT ScriptRenderTarget : public TScriptReflectable<ScriptRenderTarget, RenderTarget, ScriptRenderTargetBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "RenderTarget")

		ScriptRenderTarget(MonoObject* managedInstance, const SPtr<RenderTarget>& value);

		static MonoObject* create(const SPtr<RenderTarget>& value);

	private:
		static uint32_t Internal_getWidth(ScriptRenderTargetBase* thisPtr);
		static uint32_t Internal_getHeight(ScriptRenderTargetBase* thisPtr);
		static bool Internal_getGammaCorrection(ScriptRenderTargetBase* thisPtr);
		static int32_t Internal_getPriority(ScriptRenderTargetBase* thisPtr);
		static void Internal_setPriority(ScriptRenderTargetBase* thisPtr, int32_t priority);
		static uint32_t Internal_getSampleCount(ScriptRenderTargetBase* thisPtr);
	};
}
