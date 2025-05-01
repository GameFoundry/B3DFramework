//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTarget.h"

namespace bs { class RenderTarget; }
namespace bs { class RenderTargetEx; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderTargetWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderTarget : public TScriptReflectableWrapper<RenderTarget, ScriptRenderTarget, ScriptRenderTargetWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RenderTarget")

		ScriptRenderTarget(const SPtr<RenderTarget>& nativeObject);
		~ScriptRenderTarget();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static uint32_t InternalGetWidth(ScriptRenderTargetWrapperBase* self);
		static uint32_t InternalGetHeight(ScriptRenderTargetWrapperBase* self);
		static bool InternalGetGammaCorrection(ScriptRenderTargetWrapperBase* self);
		static int32_t InternalGetPriority(ScriptRenderTargetWrapperBase* self);
		static void InternalSetPriority(ScriptRenderTargetWrapperBase* self, int32_t priority);
		static uint32_t InternalGetSampleCount(ScriptRenderTargetWrapperBase* self);
	};
}
