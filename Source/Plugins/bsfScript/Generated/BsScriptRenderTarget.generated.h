//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class RenderTarget;
	class RenderTargetEx;

	class BS_SCR_BE_EXPORT ScriptRenderTargetBase : public ScriptObjectBase
	{
	public:
		ScriptRenderTargetBase(MonoObject* instance);
		virtual ~ScriptRenderTargetBase() {}

		SPtr<RenderTarget> getInternal() const { return mInternal; }
	protected:
		SPtr<RenderTarget> mInternal;
	};

	class BS_SCR_BE_EXPORT ScriptRenderTarget : public ScriptObject<ScriptRenderTarget, ScriptRenderTargetBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "RenderTarget")

		ScriptRenderTarget(MonoObject* managedInstance, const SPtr<RenderTarget>& value);

		SPtr<RenderTarget> getInternal() const;
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
