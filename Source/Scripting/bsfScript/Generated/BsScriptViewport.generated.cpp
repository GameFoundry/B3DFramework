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

	void ScriptViewport::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_setTarget", (void*)&ScriptViewport::Internal_setTarget);
		metaData.scriptClass->AddInternalCall("Internal_getTarget", (void*)&ScriptViewport::Internal_getTarget);
		metaData.scriptClass->AddInternalCall("Internal_setArea", (void*)&ScriptViewport::Internal_setArea);
		metaData.scriptClass->AddInternalCall("Internal_getArea", (void*)&ScriptViewport::Internal_getArea);
		metaData.scriptClass->AddInternalCall("Internal_getPixelArea", (void*)&ScriptViewport::Internal_getPixelArea);
		metaData.scriptClass->AddInternalCall("Internal_setClearFlags", (void*)&ScriptViewport::Internal_setClearFlags);
		metaData.scriptClass->AddInternalCall("Internal_getClearFlags", (void*)&ScriptViewport::Internal_getClearFlags);
		metaData.scriptClass->AddInternalCall("Internal_setClearColorValue", (void*)&ScriptViewport::Internal_setClearColorValue);
		metaData.scriptClass->AddInternalCall("Internal_getClearColorValue", (void*)&ScriptViewport::Internal_getClearColorValue);
		metaData.scriptClass->AddInternalCall("Internal_setClearDepthValue", (void*)&ScriptViewport::Internal_setClearDepthValue);
		metaData.scriptClass->AddInternalCall("Internal_getClearDepthValue", (void*)&ScriptViewport::Internal_getClearDepthValue);
		metaData.scriptClass->AddInternalCall("Internal_setClearStencilValue", (void*)&ScriptViewport::Internal_setClearStencilValue);
		metaData.scriptClass->AddInternalCall("Internal_getClearStencilValue", (void*)&ScriptViewport::Internal_getClearStencilValue);
		metaData.scriptClass->AddInternalCall("Internal_create", (void*)&ScriptViewport::Internal_create);

	}

	MonoObject* ScriptViewport::create(const SPtr<Viewport>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptViewport>()) ScriptViewport(managedInstance, value);
		return managedInstance;
	}
	void ScriptViewport::Internal_setTarget(ScriptViewport* thisPtr, MonoObject* target)
	{
		SPtr<RenderTarget> tmptarget;
		ScriptRenderTargetBase* scripttarget;
		scripttarget = (ScriptRenderTargetBase*)ScriptRenderTarget::toNative(target);
		if(scripttarget != nullptr)
			tmptarget = scripttarget->GetInternal();
		thisPtr->GetInternal()->setTarget(tmptarget);
	}

	MonoObject* ScriptViewport::Internal_getTarget(ScriptViewport* thisPtr)
	{
		SPtr<RenderTarget> tmp__output;
		tmp__output = thisPtr->GetInternal()->getTarget();

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
		thisPtr->GetInternal()->setArea(*area);
	}

	void ScriptViewport::Internal_getArea(ScriptViewport* thisPtr, Rect2* __output)
	{
		Rect2 tmp__output;
		tmp__output = thisPtr->GetInternal()->getArea();

		*__output = tmp__output;
	}

	void ScriptViewport::Internal_getPixelArea(ScriptViewport* thisPtr, Rect2I* __output)
	{
		Rect2I tmp__output;
		tmp__output = thisPtr->GetInternal()->getPixelArea();

		*__output = tmp__output;
	}

	void ScriptViewport::Internal_setClearFlags(ScriptViewport* thisPtr, ClearFlagBits flags)
	{
		thisPtr->GetInternal()->setClearFlags(flags);
	}

	ClearFlagBits ScriptViewport::Internal_getClearFlags(ScriptViewport* thisPtr)
	{
		Flags<ClearFlagBits> tmp__output;
		tmp__output = thisPtr->GetInternal()->getClearFlags();

		ClearFlagBits __output;
		__output = (ClearFlagBits)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptViewport::Internal_setClearColorValue(ScriptViewport* thisPtr, Color* color)
	{
		thisPtr->GetInternal()->setClearColorValue(*color);
	}

	void ScriptViewport::Internal_getClearColorValue(ScriptViewport* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->getClearColorValue();

		*__output = tmp__output;
	}

	void ScriptViewport::Internal_setClearDepthValue(ScriptViewport* thisPtr, float depth)
	{
		thisPtr->GetInternal()->setClearDepthValue(depth);
	}

	float ScriptViewport::Internal_getClearDepthValue(ScriptViewport* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->getClearDepthValue();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptViewport::Internal_setClearStencilValue(ScriptViewport* thisPtr, uint16_t value)
	{
		thisPtr->GetInternal()->setClearStencilValue(value);
	}

	uint16_t ScriptViewport::Internal_getClearStencilValue(ScriptViewport* thisPtr)
	{
		uint16_t tmp__output;
		tmp__output = thisPtr->GetInternal()->getClearStencilValue();

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
			tmptarget = scripttarget->GetInternal();
		SPtr<Viewport> instance = Viewport::create(tmptarget, x, y, width, height);
		new (bs_alloc<ScriptViewport>())ScriptViewport(managedInstance, instance);
	}
}
