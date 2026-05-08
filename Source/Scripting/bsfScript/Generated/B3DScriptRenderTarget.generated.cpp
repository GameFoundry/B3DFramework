//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptRenderTarget.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../Extensions/B3DRenderTargetEx.h"

namespace b3d
{
	ScriptRenderTarget::ScriptRenderTarget(const TShared<RenderTarget>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptRenderTarget::~ScriptRenderTarget()
	{
		UnregisterEvents();
	}

	void ScriptRenderTarget::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetWidth", (void*)&ScriptRenderTarget::InternalGetWidth);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHeight", (void*)&ScriptRenderTarget::InternalGetHeight);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetGammaCorrection", (void*)&ScriptRenderTarget::InternalGetGammaCorrection);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPriority", (void*)&ScriptRenderTarget::InternalGetPriority);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPriority", (void*)&ScriptRenderTarget::InternalSetPriority);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSampleCount", (void*)&ScriptRenderTarget::InternalGetSampleCount);

	}

	MonoObject* ScriptRenderTarget::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	uint32_t ScriptRenderTarget::InternalGetWidth(ScriptRenderTargetWrapperBase* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = RenderTargetEx::GetWidth(std::static_pointer_cast<RenderTarget>(self->GetBaseNativeObjectAsShared()));

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptRenderTarget::InternalGetHeight(ScriptRenderTargetWrapperBase* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = RenderTargetEx::GetHeight(std::static_pointer_cast<RenderTarget>(self->GetBaseNativeObjectAsShared()));

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptRenderTarget::InternalGetGammaCorrection(ScriptRenderTargetWrapperBase* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = RenderTargetEx::GetGammaCorrection(std::static_pointer_cast<RenderTarget>(self->GetBaseNativeObjectAsShared()));

		bool __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRenderTarget::InternalGetPriority(ScriptRenderTargetWrapperBase* self)
	{
		int32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = RenderTargetEx::GetPriority(std::static_pointer_cast<RenderTarget>(self->GetBaseNativeObjectAsShared()));

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderTarget::InternalSetPriority(ScriptRenderTargetWrapperBase* self, int32_t priority)
	{
		if(!self->IsNativeObjectValid())
			return;

		RenderTargetEx::SetPriority(std::static_pointer_cast<RenderTarget>(self->GetBaseNativeObjectAsShared()), priority);
	}

	uint32_t ScriptRenderTarget::InternalGetSampleCount(ScriptRenderTargetWrapperBase* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = RenderTargetEx::GetSampleCount(std::static_pointer_cast<RenderTarget>(self->GetBaseNativeObjectAsShared()));

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}
}
