//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIClickable.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/GUI/B3DGUIClickable.h"
#include "B3DScriptGUIContent.generated.h"

namespace b3d
{
	ScriptGUIClickableWrapperBase::OnClickThunkDefinition ScriptGUIClickableWrapperBase::OnClickThunk; 
	ScriptGUIClickableWrapperBase::OnHoverThunkDefinition ScriptGUIClickableWrapperBase::OnHoverThunk; 
	ScriptGUIClickableWrapperBase::OnOutThunkDefinition ScriptGUIClickableWrapperBase::OnOutThunk; 
	ScriptGUIClickableWrapperBase::OnDoubleClickThunkDefinition ScriptGUIClickableWrapperBase::OnDoubleClickThunk; 

	void ScriptGUIClickableWrapperBase::OnClick()
	{
		MonoUtil::InvokeThunk(OnClickThunk, GetScriptObject());
	}

	void ScriptGUIClickableWrapperBase::OnHover()
	{
		MonoUtil::InvokeThunk(OnHoverThunk, GetScriptObject());
	}

	void ScriptGUIClickableWrapperBase::OnOut()
	{
		MonoUtil::InvokeThunk(OnOutThunk, GetScriptObject());
	}

	void ScriptGUIClickableWrapperBase::OnDoubleClick()
	{
		MonoUtil::InvokeThunk(OnDoubleClickThunk, GetScriptObject());
	}

	void ScriptGUIClickableWrapperBase::RegisterEvents()
	{
		OnClickConnection = static_cast<GUIClickable*>(GetNativeObject())->OnClick.Connect([this]() { OnClick(); }));
		OnHoverConnection = static_cast<GUIClickable*>(GetNativeObject())->OnHover.Connect([this]() { OnHover(); }));
		OnOutConnection = static_cast<GUIClickable*>(GetNativeObject())->OnOut.Connect([this]() { OnOut(); }));
		OnDoubleClickConnection = static_cast<GUIClickable*>(GetNativeObject())->OnDoubleClick.Connect([this]() { OnDoubleClick(); }));
		ScriptGUIInteractableWrapperBase::RegisterEvents();
	}
	void ScriptGUIClickableWrapperBase::UnregisterEvents()
	{
		OnClickConnection.Disconnect();
		OnHoverConnection.Disconnect();
		OnOutConnection.Disconnect();
		OnDoubleClickConnection.Disconnect();
		ScriptGUIInteractableWrapperBase::UnregisterEvents();
	}
	ScriptGUIClickable::ScriptGUIClickable(GUIClickable* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIClickable::~ScriptGUIClickable()
	{
		UnregisterEvents();
	}

	void ScriptGUIClickable::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetContent", (void*)&ScriptGUIClickable::InternalSetContent);

		OnClickThunk = (OnClickThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnClick", "")->GetThunk();
		OnHoverThunk = (OnHoverThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnHover", "")->GetThunk();
		OnOutThunk = (OnOutThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnOut", "")->GetThunk();
		OnDoubleClickThunk = (OnDoubleClickThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnDoubleClick", "")->GetThunk();
	}

	MonoObject* ScriptGUIClickable::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIClickable::InternalSetContent(ScriptGUIClickableWrapperBase* self, __GUIContentInterop* content)
	{
		if(!self->IsNativeObjectValid())
			return;

		GUIContent tmpcontent;
		tmpcontent = ScriptGUIContent::FromInterop(*content);
		static_cast<GUIClickable*>(self->GetNativeObject())->SetContent(tmpcontent);
	}
}
