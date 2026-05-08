//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/GpuBackend/B3DRenderTarget.h"

namespace b3d { class RenderTarget; }
namespace b3d { class RenderTargetEx; }
namespace b3d
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

		ScriptRenderTarget(const TShared<RenderTarget>& nativeObject);
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
