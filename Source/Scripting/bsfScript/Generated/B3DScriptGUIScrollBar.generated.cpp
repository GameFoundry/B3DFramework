//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIScrollBar.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/GUI/B3DGUIScrollBar.h"

namespace b3d
{
	ScriptGUIScrollBarWrapperBase::OnScrollOrResizeThunkDefinition ScriptGUIScrollBarWrapperBase::OnScrollOrResizeThunk; 

	void ScriptGUIScrollBarWrapperBase::OnScrollOrResize(float p0, float p1)
	{
		MonoUtil::InvokeThunk(OnScrollOrResizeThunk, GetScriptObject(), p0, p1);
	}

	void ScriptGUIScrollBarWrapperBase::RegisterEvents()
	{
		OnScrollOrResizeConnection = static_cast<GUIScrollBar*>(GetNativeObject())->OnScrollOrResize.Connect([this](float p0, float p1) { OnScrollOrResize(p0, p1); }));
		ScriptGUIInteractableWrapperBase::RegisterEvents();
	}
	void ScriptGUIScrollBarWrapperBase::UnregisterEvents()
	{
		OnScrollOrResizeConnection.Disconnect();
		ScriptGUIInteractableWrapperBase::UnregisterEvents();
	}
	ScriptGUIScrollBar::ScriptGUIScrollBar(GUIScrollBar* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIScrollBar::~ScriptGUIScrollBar()
	{
		UnregisterEvents();
	}

	void ScriptGUIScrollBar::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetScrollHandlePosition", (void*)&ScriptGUIScrollBar::InternalSetScrollHandlePosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScrollHandlePosition", (void*)&ScriptGUIScrollBar::InternalGetScrollHandlePosition);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetScrollHandleSize", (void*)&ScriptGUIScrollBar::InternalSetScrollHandleSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScrollHandleSize", (void*)&ScriptGUIScrollBar::InternalGetScrollHandleSize);

		OnScrollOrResizeThunk = (OnScrollOrResizeThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnScrollOrResize", "single,single")->GetThunk();
	}

	MonoObject* ScriptGUIScrollBar::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIScrollBar::InternalSetScrollHandlePosition(ScriptGUIScrollBarWrapperBase* self, float pct)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollBar*>(self->GetNativeObject())->SetScrollHandlePosition(pct);
	}

	float ScriptGUIScrollBar::InternalGetScrollHandlePosition(ScriptGUIScrollBarWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIScrollBar*>(self->GetNativeObject())->GetScrollHandlePosition();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIScrollBar::InternalSetScrollHandleSize(ScriptGUIScrollBarWrapperBase* self, float pct)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIScrollBar*>(self->GetNativeObject())->SetScrollHandleSize(pct);
	}

	float ScriptGUIScrollBar::InternalGetScrollHandleSize(ScriptGUIScrollBarWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIScrollBar*>(self->GetNativeObject())->GetScrollHandleSize();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
