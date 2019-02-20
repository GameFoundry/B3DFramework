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

	SPtr<RenderTarget> ScriptRenderTargetBase::getInternal() const
	{
		return std::static_pointer_cast<RenderTarget>(mInternal);
	}
	ScriptRenderTarget::ScriptRenderTarget(MonoObject* managedInstance, const SPtr<RenderTarget>& value)
		:TScriptReflectable(managedInstance, value)
	{
		mInternal = value;
	}

	void ScriptRenderTarget::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getWidth", (void*)&ScriptRenderTarget::Internal_getWidth);
		metaData.scriptClass->addInternalCall("Internal_getHeight", (void*)&ScriptRenderTarget::Internal_getHeight);
		metaData.scriptClass->addInternalCall("Internal_getGammaCorrection", (void*)&ScriptRenderTarget::Internal_getGammaCorrection);
		metaData.scriptClass->addInternalCall("Internal_getPriority", (void*)&ScriptRenderTarget::Internal_getPriority);
		metaData.scriptClass->addInternalCall("Internal_setPriority", (void*)&ScriptRenderTarget::Internal_setPriority);
		metaData.scriptClass->addInternalCall("Internal_getSampleCount", (void*)&ScriptRenderTarget::Internal_getSampleCount);

	}

	MonoObject* ScriptRenderTarget::create(const SPtr<RenderTarget>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptRenderTarget>()) ScriptRenderTarget(managedInstance, value);
		return managedInstance;
	}
	uint32_t ScriptRenderTarget::Internal_getWidth(ScriptRenderTargetBase* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = RenderTargetEx::getWidth(thisPtr->getInternal());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	uint32_t ScriptRenderTarget::Internal_getHeight(ScriptRenderTargetBase* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = RenderTargetEx::getHeight(thisPtr->getInternal());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptRenderTarget::Internal_getGammaCorrection(ScriptRenderTargetBase* thisPtr)
	{
		bool tmp__output;
		tmp__output = RenderTargetEx::getGammaCorrection(thisPtr->getInternal());

		bool __output;
		__output = tmp__output;

		return __output;
	}

	int32_t ScriptRenderTarget::Internal_getPriority(ScriptRenderTargetBase* thisPtr)
	{
		int32_t tmp__output;
		tmp__output = RenderTargetEx::getPriority(thisPtr->getInternal());

		int32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptRenderTarget::Internal_setPriority(ScriptRenderTargetBase* thisPtr, int32_t priority)
	{
		RenderTargetEx::setPriority(thisPtr->getInternal(), priority);
	}

	uint32_t ScriptRenderTarget::Internal_getSampleCount(ScriptRenderTargetBase* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = RenderTargetEx::getSampleCount(thisPtr->getInternal());

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}
}
