//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRenderTarget.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../Extensions/BsRenderTargetEx.h"

namespace bs
{
	ScriptRenderTargetBase::ScriptRenderTargetBase(MonoObject* managedInstance)
		: ScriptReflectableBase(managedInstance)
	{}

	SPtr<RenderTarget> ScriptRenderTargetBase::GetInternal() const
	{
		return std::static_pointer_cast<RenderTarget>(mInternal);
	}

	ScriptRenderTarget::ScriptRenderTarget(MonoObject* managedInstance, const SPtr<RenderTarget>& value)
		: TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptRenderTarget::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetWidth", (void*)&ScriptRenderTarget::InternalGetWidth);
		metaData.ScriptClass->AddInternalCall("Internal_GetHeight", (void*)&ScriptRenderTarget::InternalGetHeight);
		metaData.ScriptClass->AddInternalCall("Internal_GetGammaCorrection", (void*)&ScriptRenderTarget::InternalGetGammaCorrection);
		metaData.ScriptClass->AddInternalCall("Internal_GetPriority", (void*)&ScriptRenderTarget::InternalGetPriority);
		metaData.ScriptClass->AddInternalCall("Internal_SetPriority", (void*)&ScriptRenderTarget::InternalSetPriority);
		metaData.ScriptClass->AddInternalCall("Internal_GetSampleCount", (void*)&ScriptRenderTarget::InternalGetSampleCount);
	}

	MonoObject* ScriptRenderTarget::Create(const SPtr<RenderTarget>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptRenderTarget>()) ScriptRenderTarget(managedInstance, value);
		return managedInstance;
	}

	uint32_t ScriptRenderTarget::InternalGetWidth(ScriptRenderTargetBase* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = RenderTargetEx::GetWidth(thisPtr->GetInternal());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptRenderTarget::InternalGetHeight(ScriptRenderTargetBase* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = RenderTargetEx::GetHeight(thisPtr->GetInternal());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptRenderTarget::InternalGetGammaCorrection(ScriptRenderTargetBase* thisPtr)
	{
		bool tmp__output;
		tmp__output = RenderTargetEx::GetGammaCorrection(thisPtr->GetInternal());

		bool __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRenderTarget::InternalGetPriority(ScriptRenderTargetBase* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = RenderTargetEx::GetPriority(thisPtr->GetInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderTarget::InternalSetPriority(ScriptRenderTargetBase* thisPtr, int32_t priority)
	{
		RenderTargetEx::SetPriority(thisPtr->GetInternal(), priority);
	}

	uint32_t ScriptRenderTarget::InternalGetSampleCount(ScriptRenderTargetBase* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = RenderTargetEx::GetSampleCount(thisPtr->GetInternal());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}
} // namespace bs
