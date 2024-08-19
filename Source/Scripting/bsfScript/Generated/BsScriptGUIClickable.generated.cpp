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
	ScriptGUIClickableBase::OnClickThunkDefinition ScriptGUIClickableBase::OnClickThunk; 
	ScriptGUIClickableBase::OnHoverThunkDefinition ScriptGUIClickableBase::OnHoverThunk; 
	ScriptGUIClickableBase::OnOutThunkDefinition ScriptGUIClickableBase::OnOutThunk; 
	ScriptGUIClickableBase::OnDoubleClickThunkDefinition ScriptGUIClickableBase::OnDoubleClickThunk; 

	ScriptGUIClickableBase::ScriptGUIClickableBase(MonoObject* managedInstance)
		:ScriptGUIInteractableBase(managedInstance)
	 { }

	void ScriptGUIClickableBase::OnClick()
	{
		MonoUtil::InvokeThunk(OnClickThunk, GetManagedInstance());
	}

	void ScriptGUIClickableBase::OnHover()
	{
		MonoUtil::InvokeThunk(OnHoverThunk, GetManagedInstance());
	}

	void ScriptGUIClickableBase::OnOut()
	{
		MonoUtil::InvokeThunk(OnOutThunk, GetManagedInstance());
	}

	void ScriptGUIClickableBase::OnDoubleClick()
	{
		MonoUtil::InvokeThunk(OnDoubleClickThunk, GetManagedInstance());
	}

	void ScriptGUIClickableBase::RegisterEvents(GUIElement* value)
	{
		static_cast<GUIClickable*>(value)->OnClick.Connect(std::bind(&ScriptGUIClickableBase::OnClick, this));
		static_cast<GUIClickable*>(value)->OnHover.Connect(std::bind(&ScriptGUIClickableBase::OnHover, this));
		static_cast<GUIClickable*>(value)->OnOut.Connect(std::bind(&ScriptGUIClickableBase::OnOut, this));
		static_cast<GUIClickable*>(value)->OnDoubleClick.Connect(std::bind(&ScriptGUIClickableBase::OnDoubleClick, this));
		ScriptGUIInteractableBase::RegisterEvents(value);
	}
	ScriptGUIClickable::ScriptGUIClickable(MonoObject* managedInstance, GUIClickable* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
	}

	void ScriptGUIClickable::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetContent", (void*)&ScriptGUIClickable::InternalSetContent);

		OnClickThunk = (OnClickThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnClick", "")->GetThunk();
		OnHoverThunk = (OnHoverThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnHover", "")->GetThunk();
		OnOutThunk = (OnOutThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnOut", "")->GetThunk();
		OnDoubleClickThunk = (OnDoubleClickThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnDoubleClick", "")->GetThunk();
	}

	void ScriptGUIClickable::InternalSetContent(ScriptGUIElementBase* self, __GUIContentInterop* content)
	{
		GUIContent tmpcontent;
		tmpcontent = ScriptGUIContent::FromInterop(*content);
		static_cast<GUIClickable*>(self->GetGuiElement())->SetContent(tmpcontent);
	}
}
