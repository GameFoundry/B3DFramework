//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptRenderTarget.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsRenderTarget.h"
#include "BsCoreThread.h"

namespace bs
{
	ScriptRenderTarget::ScriptRenderTarget(MonoObject* instance)
		:ScriptObject(instance)
	{

	}

	void ScriptRenderTarget::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetWidth", &ScriptRenderTarget::internal_getWidth);
		metaData.scriptClass->addInternalCall("Internal_GetHeight", &ScriptRenderTarget::internal_getHeight);
		metaData.scriptClass->addInternalCall("Internal_GetGammaCorrection", &ScriptRenderTarget::internal_getGammaCorrection);
		metaData.scriptClass->addInternalCall("Internal_GetSampleCount", &ScriptRenderTarget::internal_getSampleCount);
		metaData.scriptClass->addInternalCall("Internal_GetPriority", &ScriptRenderTarget::internal_getPriority);
		metaData.scriptClass->addInternalCall("Internal_SetPriority", &ScriptRenderTarget::internal_setPriority);
	}

	void ScriptRenderTarget::internal_getWidth(ScriptRenderTarget* thisPtr, int* value)
	{
		SPtr<RenderTarget> rt = thisPtr->getNativeValue();
		*value = rt->getProperties().getWidth();
	}

	void ScriptRenderTarget::internal_getHeight(ScriptRenderTarget* thisPtr, int* value)
	{
		SPtr<RenderTarget> rt = thisPtr->getNativeValue();
		*value = rt->getProperties().getHeight();
	}

	void ScriptRenderTarget::internal_getGammaCorrection(ScriptRenderTarget* thisPtr, bool* value)
	{
		SPtr<RenderTarget> rt = thisPtr->getNativeValue();
		*value = rt->getProperties().isHwGammaEnabled();
	}

	void ScriptRenderTarget::internal_getSampleCount(ScriptRenderTarget* thisPtr, int* value)
	{
		SPtr<RenderTarget> rt = thisPtr->getNativeValue();
		*value = rt->getProperties().getMultisampleCount();
	}

	void ScriptRenderTarget::internal_getPriority(ScriptRenderTarget* thisPtr, int* value)
	{
		SPtr<RenderTarget> rt = thisPtr->getNativeValue();
		*value = rt->getProperties().getPriority();
	}

	void ScriptRenderTarget::internal_setPriority(ScriptRenderTarget* thisPtr, int value)
	{
		SPtr<RenderTarget> rt = thisPtr->getNativeValue();
		rt->setPriority(gCoreAccessor(), value);
	}
}