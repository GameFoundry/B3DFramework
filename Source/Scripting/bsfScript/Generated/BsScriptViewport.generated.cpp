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
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptViewport::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetTarget", (void*)&ScriptViewport::InternalSetTarget);
		metaData.ScriptClass->AddInternalCall("Internal_GetTarget", (void*)&ScriptViewport::InternalGetTarget);
		metaData.ScriptClass->AddInternalCall("Internal_SetArea", (void*)&ScriptViewport::InternalSetArea);
		metaData.ScriptClass->AddInternalCall("Internal_GetArea", (void*)&ScriptViewport::InternalGetArea);
		metaData.ScriptClass->AddInternalCall("Internal_GetPixelArea", (void*)&ScriptViewport::InternalGetPixelArea);
		metaData.ScriptClass->AddInternalCall("Internal_SetClearFlags", (void*)&ScriptViewport::InternalSetClearFlags);
		metaData.ScriptClass->AddInternalCall("Internal_GetClearFlags", (void*)&ScriptViewport::InternalGetClearFlags);
		metaData.ScriptClass->AddInternalCall("Internal_SetClearColorValue", (void*)&ScriptViewport::InternalSetClearColorValue);
		metaData.ScriptClass->AddInternalCall("Internal_GetClearColorValue", (void*)&ScriptViewport::InternalGetClearColorValue);
		metaData.ScriptClass->AddInternalCall("Internal_SetClearDepthValue", (void*)&ScriptViewport::InternalSetClearDepthValue);
		metaData.ScriptClass->AddInternalCall("Internal_GetClearDepthValue", (void*)&ScriptViewport::InternalGetClearDepthValue);
		metaData.ScriptClass->AddInternalCall("Internal_SetClearStencilValue", (void*)&ScriptViewport::InternalSetClearStencilValue);
		metaData.ScriptClass->AddInternalCall("Internal_GetClearStencilValue", (void*)&ScriptViewport::InternalGetClearStencilValue);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptViewport::InternalCreate);
	}

	MonoObject* ScriptViewport::Create(const SPtr<Viewport>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptViewport>()) ScriptViewport(managedInstance, value);
		return managedInstance;
	}

	void ScriptViewport::InternalSetTarget(ScriptViewport* thisPtr, MonoObject* target)
	{
		SPtr<RenderTarget> tmptarget;
		ScriptRenderTargetBase* scripttarget;
		scripttarget = (ScriptRenderTargetBase*)ScriptRenderTarget::ToNative(target);
		if(scripttarget != nullptr)
			tmptarget = scripttarget->GetInternal();
		thisPtr->GetInternal()->SetTarget(tmptarget);
	}

	MonoObject* ScriptViewport::InternalGetTarget(ScriptViewport* thisPtr)
	{
		SPtr<RenderTarget> tmp__output;
		tmp__output = thisPtr->GetInternal()->GetTarget();

		MonoObject* __output;
		if(tmp__output)
		{
			if(rtti_is_of_type<RenderTexture>(tmp__output))
				__output = ScriptRenderTexture::Create(std::static_pointer_cast<RenderTexture>(tmp__output));
			else
				__output = ScriptRenderTarget::Create(tmp__output);
		}
		else
			__output = ScriptRenderTarget::Create(tmp__output);

		return __output;
	}

	void ScriptViewport::InternalSetArea(ScriptViewport* thisPtr, Rect2* area)
	{
		thisPtr->GetInternal()->SetArea(*area);
	}

	void ScriptViewport::InternalGetArea(ScriptViewport* thisPtr, Rect2* __output)
	{
		Rect2 tmp__output;
		tmp__output = thisPtr->GetInternal()->GetArea();

		*__output = tmp__output;
	}

	void ScriptViewport::InternalGetPixelArea(ScriptViewport* thisPtr, Rect2I* __output)
	{
		Rect2I tmp__output;
		tmp__output = thisPtr->GetInternal()->GetPixelArea();

		*__output = tmp__output;
	}

	void ScriptViewport::InternalSetClearFlags(ScriptViewport* thisPtr, ClearFlagBits flags)
	{
		thisPtr->GetInternal()->SetClearFlags(flags);
	}

	ClearFlagBits ScriptViewport::InternalGetClearFlags(ScriptViewport* thisPtr)
	{
		Flags<ClearFlagBits> tmp__output;
		tmp__output = thisPtr->GetInternal()->GetClearFlags();

		ClearFlagBits __output;
		__output = (ClearFlagBits)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptViewport::InternalSetClearColorValue(ScriptViewport* thisPtr, Color* color)
	{
		thisPtr->GetInternal()->SetClearColorValue(*color);
	}

	void ScriptViewport::InternalGetClearColorValue(ScriptViewport* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetInternal()->GetClearColorValue();

		*__output = tmp__output;
	}

	void ScriptViewport::InternalSetClearDepthValue(ScriptViewport* thisPtr, float depth)
	{
		thisPtr->GetInternal()->SetClearDepthValue(depth);
	}

	float ScriptViewport::InternalGetClearDepthValue(ScriptViewport* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->GetClearDepthValue();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptViewport::InternalSetClearStencilValue(ScriptViewport* thisPtr, uint16_t value)
	{
		thisPtr->GetInternal()->SetClearStencilValue(value);
	}

	uint16_t ScriptViewport::InternalGetClearStencilValue(ScriptViewport* thisPtr)
	{
		uint16_t tmp__output;
		tmp__output = thisPtr->GetInternal()->GetClearStencilValue();

		uint16_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptViewport::InternalCreate(MonoObject* managedInstance, MonoObject* target, float x, float y, float width, float height)
	{
		SPtr<RenderTarget> tmptarget;
		ScriptRenderTargetBase* scripttarget;
		scripttarget = (ScriptRenderTargetBase*)ScriptRenderTarget::ToNative(target);
		if(scripttarget != nullptr)
			tmptarget = scripttarget->GetInternal();
		SPtr<Viewport> instance = Viewport::Create(tmptarget, x, y, width, height);
		new(bs_alloc<ScriptViewport>()) ScriptViewport(managedInstance, instance);
	}
} // namespace bs
