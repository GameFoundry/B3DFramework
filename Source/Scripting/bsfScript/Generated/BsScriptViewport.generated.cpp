//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptViewport.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Reflection/BsRTTIType.h"
#include "Wrappers/BsScriptColor.h"
#include "BsScriptRenderTarget.generated.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTexture.h"
#include "BsScriptRenderTexture.generated.h"
#include "BsScriptViewport.generated.h"

namespace bs
{
	ScriptViewport::ScriptViewport(MonoObject* managedInstance, const SPtr<Viewport>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptViewport::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setTarget", (void*)&ScriptViewport::Internal_setTarget);
		metaData.scriptClass->addInternalCall("Internal_getTarget", (void*)&ScriptViewport::Internal_getTarget);
		metaData.scriptClass->addInternalCall("Internal_setArea", (void*)&ScriptViewport::Internal_setArea);
		metaData.scriptClass->addInternalCall("Internal_getArea", (void*)&ScriptViewport::Internal_getArea);
		metaData.scriptClass->addInternalCall("Internal_getPixelArea", (void*)&ScriptViewport::Internal_getPixelArea);
		metaData.scriptClass->addInternalCall("Internal_setClearFlags", (void*)&ScriptViewport::Internal_setClearFlags);
		metaData.scriptClass->addInternalCall("Internal_getClearFlags", (void*)&ScriptViewport::Internal_getClearFlags);
		metaData.scriptClass->addInternalCall("Internal_setClearColorValue", (void*)&ScriptViewport::Internal_setClearColorValue);
		metaData.scriptClass->addInternalCall("Internal_getClearColorValue", (void*)&ScriptViewport::Internal_getClearColorValue);
		metaData.scriptClass->addInternalCall("Internal_setClearDepthValue", (void*)&ScriptViewport::Internal_setClearDepthValue);
		metaData.scriptClass->addInternalCall("Internal_getClearDepthValue", (void*)&ScriptViewport::Internal_getClearDepthValue);
		metaData.scriptClass->addInternalCall("Internal_setClearStencilValue", (void*)&ScriptViewport::Internal_setClearStencilValue);
		metaData.scriptClass->addInternalCall("Internal_getClearStencilValue", (void*)&ScriptViewport::Internal_getClearStencilValue);
		metaData.scriptClass->addInternalCall("Internal_create", (void*)&ScriptViewport::Internal_create);

	}

	MonoObject* ScriptViewport::create(const SPtr<Viewport>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptViewport>()) ScriptViewport(managedInstance, value);
		return managedInstance;
	}
	void ScriptViewport::Internal_setTarget(ScriptViewport* thisPtr, MonoObject* target)
	{
		SPtr<RenderTarget> tmptarget;
		ScriptRenderTargetBase* scripttarget;
		scripttarget = (ScriptRenderTargetBase*)ScriptRenderTarget::toNative(target);
		if(scripttarget != nullptr)
			tmptarget = scripttarget->getInternal();
		thisPtr->getInternal()->setTarget(tmptarget);
	}

	MonoObject* ScriptViewport::Internal_getTarget(ScriptViewport* thisPtr)
	{
		SPtr<RenderTarget> tmp__output;
		tmp__output = thisPtr->getInternal()->getTarget();

		MonoObject* __output;
		if(tmp__output)
		{
			if(rtti_is_of_type<RenderTexture>(tmp__output))
				__output = ScriptRenderTexture::create(std::static_pointer_cast<RenderTexture>(tmp__output));
			else
				__output = ScriptRenderTarget::create(tmp__output);
		}
		else
			__output = ScriptRenderTarget::create(tmp__output);

		return __output;
	}

	void ScriptViewport::Internal_setArea(ScriptViewport* thisPtr, Rect2* area)
	{
		thisPtr->getInternal()->setArea(*area);
	}

	void ScriptViewport::Internal_getArea(ScriptViewport* thisPtr, Rect2* __output)
	{
		Rect2 tmp__output;
		tmp__output = thisPtr->getInternal()->getArea();

		*__output = tmp__output;
	}

	void ScriptViewport::Internal_getPixelArea(ScriptViewport* thisPtr, Rect2I* __output)
	{
		Rect2I tmp__output;
		tmp__output = thisPtr->getInternal()->getPixelArea();

		*__output = tmp__output;
	}

	void ScriptViewport::Internal_setClearFlags(ScriptViewport* thisPtr, ClearFlagBits flags)
	{
		thisPtr->getInternal()->setClearFlags(flags);
	}

	ClearFlagBits ScriptViewport::Internal_getClearFlags(ScriptViewport* thisPtr)
	{
		Flags<ClearFlagBits> tmp__output;
		tmp__output = thisPtr->getInternal()->getClearFlags();

		ClearFlagBits __output;
		__output = (ClearFlagBits)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptViewport::Internal_setClearColorValue(ScriptViewport* thisPtr, Color* color)
	{
		thisPtr->getInternal()->setClearColorValue(*color);
	}

	void ScriptViewport::Internal_getClearColorValue(ScriptViewport* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->getInternal()->getClearColorValue();

		*__output = tmp__output;
	}

	void ScriptViewport::Internal_setClearDepthValue(ScriptViewport* thisPtr, float depth)
	{
		thisPtr->getInternal()->setClearDepthValue(depth);
	}

	float ScriptViewport::Internal_getClearDepthValue(ScriptViewport* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->getClearDepthValue();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptViewport::Internal_setClearStencilValue(ScriptViewport* thisPtr, uint16_t value)
	{
		thisPtr->getInternal()->setClearStencilValue(value);
	}

	uint16_t ScriptViewport::Internal_getClearStencilValue(ScriptViewport* thisPtr)
	{
		uint16_t tmp__output;
		tmp__output = thisPtr->getInternal()->getClearStencilValue();

		uint16_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptViewport::Internal_create(MonoObject* managedInstance, MonoObject* target, float x, float y, float width, float height)
	{
		SPtr<RenderTarget> tmptarget;
		ScriptRenderTargetBase* scripttarget;
		scripttarget = (ScriptRenderTargetBase*)ScriptRenderTarget::toNative(target);
		if(scripttarget != nullptr)
			tmptarget = scripttarget->getInternal();
		SPtr<Viewport> instance = Viewport::create(tmptarget, x, y, width, height);
		new (bs_alloc<ScriptViewport>())ScriptViewport(managedInstance, instance);
	}
}
