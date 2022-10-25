//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTarget.h"

namespace bs
{
	class RenderTarget;
}

namespace bs
{
	class RenderTargetEx;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptRenderTargetBase : public ScriptReflectableBase
	{
	public:
		ScriptRenderTargetBase(MonoObject* instance);

		virtual ~ScriptRenderTargetBase() {}

		SPtr<RenderTarget> GetInternal() const;
	};

	class BS_SCR_BE_EXPORT ScriptRenderTarget : public TScriptReflectable<ScriptRenderTarget, RenderTarget, ScriptRenderTargetBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "RenderTarget")

		ScriptRenderTarget(MonoObject* managedInstance, const SPtr<RenderTarget>& value);

		static MonoObject* Create(const SPtr<RenderTarget>& value);

	private:
		static uint32_t InternalGetWidth(ScriptRenderTargetBase* thisPtr);
		static uint32_t InternalGetHeight(ScriptRenderTargetBase* thisPtr);
		static bool InternalGetGammaCorrection(ScriptRenderTargetBase* thisPtr);
		static int32_t InternalGetPriority(ScriptRenderTargetBase* thisPtr);
		static void InternalSetPriority(ScriptRenderTargetBase* thisPtr, int32_t priority);
		static uint32_t InternalGetSampleCount(ScriptRenderTargetBase* thisPtr);
	};
} // namespace bs
