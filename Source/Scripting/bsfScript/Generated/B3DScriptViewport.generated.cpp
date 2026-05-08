//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptViewport.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptColor.generated.h"
#include "B3DScriptRenderTarget.generated.h"
#include "B3DScriptTArea2.generated.h"
#include "B3DScriptTArea2.generated.h"
#include "B3DScriptViewport.generated.h"

namespace b3d
{
	ScriptViewport::ScriptViewport(const TShared<Viewport>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptViewport::~ScriptViewport()
	{
		UnregisterEvents();
	}

	void ScriptViewport::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetTarget", (void*)&ScriptViewport::InternalSetTarget);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTarget", (void*)&ScriptViewport::InternalGetTarget);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetArea", (void*)&ScriptViewport::InternalSetArea);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetArea", (void*)&ScriptViewport::InternalGetArea);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPixelArea", (void*)&ScriptViewport::InternalGetPixelArea);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetClearFlags", (void*)&ScriptViewport::InternalSetClearFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetClearFlags", (void*)&ScriptViewport::InternalGetClearFlags);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetClearColorValue", (void*)&ScriptViewport::InternalSetClearColorValue);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetClearColorValue", (void*)&ScriptViewport::InternalGetClearColorValue);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetClearDepthValue", (void*)&ScriptViewport::InternalSetClearDepthValue);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetClearDepthValue", (void*)&ScriptViewport::InternalGetClearDepthValue);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetClearStencilValue", (void*)&ScriptViewport::InternalSetClearStencilValue);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetClearStencilValue", (void*)&ScriptViewport::InternalGetClearStencilValue);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptViewport::InternalCreate);

	}

	MonoObject* ScriptViewport::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptViewport::InternalSetTarget(ScriptViewport* self, MonoObject* target)
	{
		if(!self->IsNativeObjectValid())
			return;

		TShared<RenderTarget> tmptarget;
		ScriptRenderTargetWrapperBase* scriptObjectWrappertarget;
		scriptObjectWrappertarget = (ScriptRenderTargetWrapperBase*)ScriptRenderTarget::GetScriptObjectWrapper(target);
		if(scriptObjectWrappertarget != nullptr)
			tmptarget = std::static_pointer_cast<RenderTarget>(scriptObjectWrappertarget->GetBaseNativeObjectAsShared());
		static_cast<Viewport*>(self->GetNativeObject())->SetTarget(tmptarget);
	}

	MonoObject* ScriptViewport::InternalGetTarget(ScriptViewport* self)
	{
		TShared<RenderTarget> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Viewport*>(self->GetNativeObject())->GetTarget();

		MonoObject* __output;
		__output = ScriptRenderTarget::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptViewport::InternalSetArea(ScriptViewport* self, TArea2<float, float>* area)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Viewport*>(self->GetNativeObject())->SetArea(*area);
	}

	void ScriptViewport::InternalGetArea(ScriptViewport* self, TArea2<float, float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TArea2<float, float> tmp__output;
		tmp__output = static_cast<Viewport*>(self->GetNativeObject())->GetArea();

		*__output = tmp__output;
	}

	void ScriptViewport::InternalGetPixelArea(ScriptViewport* self, TArea2<int32_t, uint32_t>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TArea2<int32_t, uint32_t> tmp__output;
		tmp__output = static_cast<Viewport*>(self->GetNativeObject())->GetPixelArea();

		*__output = tmp__output;
	}

	void ScriptViewport::InternalSetClearFlags(ScriptViewport* self, ClearFlagBits flags)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Viewport*>(self->GetNativeObject())->SetClearFlags(flags);
	}

	ClearFlagBits ScriptViewport::InternalGetClearFlags(ScriptViewport* self)
	{
		Flags<ClearFlagBits> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Viewport*>(self->GetNativeObject())->GetClearFlags();

		ClearFlagBits __output;
		__output = (ClearFlagBits)(uint32_t)tmp__output;

		return __output;
	}

	void ScriptViewport::InternalSetClearColorValue(ScriptViewport* self, Color* color)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Viewport*>(self->GetNativeObject())->SetClearColorValue(*color);
	}

	void ScriptViewport::InternalGetClearColorValue(ScriptViewport* self, Color* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		Color tmp__output;
		tmp__output = static_cast<Viewport*>(self->GetNativeObject())->GetClearColorValue();

		*__output = tmp__output;
	}

	void ScriptViewport::InternalSetClearDepthValue(ScriptViewport* self, float depth)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Viewport*>(self->GetNativeObject())->SetClearDepthValue(depth);
	}

	float ScriptViewport::InternalGetClearDepthValue(ScriptViewport* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Viewport*>(self->GetNativeObject())->GetClearDepthValue();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptViewport::InternalSetClearStencilValue(ScriptViewport* self, uint16_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Viewport*>(self->GetNativeObject())->SetClearStencilValue(value);
	}

	uint16_t ScriptViewport::InternalGetClearStencilValue(ScriptViewport* self)
	{
		uint16_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Viewport*>(self->GetNativeObject())->GetClearStencilValue();

		uint16_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptViewport::InternalCreate(MonoObject* scriptObject, MonoObject* target, float x, float y, float width, float height)
	{
		TShared<RenderTarget> tmptarget;
		ScriptRenderTargetWrapperBase* scriptObjectWrappertarget;
		scriptObjectWrappertarget = (ScriptRenderTargetWrapperBase*)ScriptRenderTarget::GetScriptObjectWrapper(target);
		if(scriptObjectWrappertarget != nullptr)
			tmptarget = std::static_pointer_cast<RenderTarget>(scriptObjectWrappertarget->GetBaseNativeObjectAsShared());
		TShared<Viewport> nativeObject = Viewport::Create(tmptarget, x, y, width, height);
		ScriptObjectWrapper::Create<ScriptViewport>(nativeObject, scriptObject);
	}
}
