//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIClickable.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIClickable.h"
#include "BsScriptGUIContent.generated.h"

namespace bs
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
		static_cast<GUIClickable*>(GetNativeObject())->OnClick.Connect(std::bind(&ScriptGUIClickableWrapperBase::OnClick, this));
		static_cast<GUIClickable*>(GetNativeObject())->OnHover.Connect(std::bind(&ScriptGUIClickableWrapperBase::OnHover, this));
		static_cast<GUIClickable*>(GetNativeObject())->OnOut.Connect(std::bind(&ScriptGUIClickableWrapperBase::OnOut, this));
		static_cast<GUIClickable*>(GetNativeObject())->OnDoubleClick.Connect(std::bind(&ScriptGUIClickableWrapperBase::OnDoubleClick, this));
		ScriptGUIElementWrapper::RegisterEvents();
	}
	ScriptGUIClickable::ScriptGUIClickable(GUIClickable* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
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
		GUIContent tmpcontent;
		tmpcontent = ScriptGUIContent::FromInterop(*content);
		static_cast<GUIClickable*>(self->GetNativeObject())->SetContent(tmpcontent);
	}
}
