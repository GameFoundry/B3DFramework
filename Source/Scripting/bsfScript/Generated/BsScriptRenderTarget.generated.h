//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTarget.h"

namespace bs { class RenderTarget; }
namespace bs { class RenderTargetEx; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderTargetBase : public ScriptReflectableBase
	{
	public:
		ScriptRenderTargetBase(MonoObject* instance);
		virtual ~ScriptRenderTargetBase() {}

		SPtr<RenderTarget> GetInternal() const;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderTarget : public TScriptReflectable<ScriptRenderTarget, RenderTarget, ScriptRenderTargetBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "RenderTarget")

		ScriptRenderTarget(MonoObject* managedInstance, const SPtr<RenderTarget>& value);

		static MonoObject* Create(const SPtr<RenderTarget>& value);

	private:
		static uint32_t InternalGetWidth(ScriptRenderTargetBase* self);
		static uint32_t InternalGetHeight(ScriptRenderTargetBase* self);
		static bool InternalGetGammaCorrection(ScriptRenderTargetBase* self);
		static int32_t InternalGetPriority(ScriptRenderTargetBase* self);
		static void InternalSetPriority(ScriptRenderTargetBase* self, int32_t priority);
		static uint32_t InternalGetSampleCount(ScriptRenderTargetBase* self);
	};
}
