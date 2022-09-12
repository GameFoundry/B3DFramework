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
		:ScriptReflectableBase(managedInstance)
	 { }

	SPtr<RenderTarget> ScriptRenderTargetBase::GetInternal() const
	{
		return std::static_pointer_cast<RenderTarget>(mInternal);
	}
	ScriptRenderTarget::ScriptRenderTarget(MonoObject* managedInstance, const SPtr<RenderTarget>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptRenderTarget::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_getWidth", (void*)&ScriptRenderTarget::Internal_getWidth);
		metaData.scriptClass->AddInternalCall("Internal_getHeight", (void*)&ScriptRenderTarget::Internal_getHeight);
		metaData.scriptClass->AddInternalCall("Internal_getGammaCorrection", (void*)&ScriptRenderTarget::Internal_getGammaCorrection);
		metaData.scriptClass->AddInternalCall("Internal_getPriority", (void*)&ScriptRenderTarget::Internal_getPriority);
		metaData.scriptClass->AddInternalCall("Internal_setPriority", (void*)&ScriptRenderTarget::Internal_setPriority);
		metaData.scriptClass->AddInternalCall("Internal_getSampleCount", (void*)&ScriptRenderTarget::Internal_getSampleCount);

	}

	MonoObject* ScriptRenderTarget::create(const SPtr<RenderTarget>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptRenderTarget>()) ScriptRenderTarget(managedInstance, value);
		return managedInstance;
	}
	uint32_t ScriptRenderTarget::Internal_getWidth(ScriptRenderTargetBase* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = RenderTargetEx::getWidth(thisPtr->GetInternal());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptRenderTarget::Internal_getHeight(ScriptRenderTargetBase* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = RenderTargetEx::getHeight(thisPtr->GetInternal());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptRenderTarget::Internal_getGammaCorrection(ScriptRenderTargetBase* thisPtr)
	{
		bool tmp__output;
		tmp__output = RenderTargetEx::getGammaCorrection(thisPtr->GetInternal());

		bool __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRenderTarget::Internal_getPriority(ScriptRenderTargetBase* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = RenderTargetEx::getPriority(thisPtr->GetInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderTarget::Internal_setPriority(ScriptRenderTargetBase* thisPtr, int32_t priority)
	{
		RenderTargetEx::setPriority(thisPtr->GetInternal(), priority);
	}

	uint32_t ScriptRenderTarget::Internal_getSampleCount(ScriptRenderTargetBase* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = RenderTargetEx::getSampleCount(thisPtr->GetInternal());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}
}
