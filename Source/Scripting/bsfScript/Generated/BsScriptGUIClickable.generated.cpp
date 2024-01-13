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
	ScriptGUIClickableBase::OnClickThunkDef ScriptGUIClickableBase::OnClickThunk; 
	ScriptGUIClickableBase::OnHoverThunkDef ScriptGUIClickableBase::OnHoverThunk; 
	ScriptGUIClickableBase::OnOutThunkDef ScriptGUIClickableBase::OnOutThunk; 
	ScriptGUIClickableBase::OnDoubleClickThunkDef ScriptGUIClickableBase::OnDoubleClickThunk; 

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

	ScriptGUIClickable::ScriptGUIClickable(MonoObject* managedInstance, GUIClickable* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		value->OnClick.Connect(std::bind(&ScriptGUIClickable::OnClick, this));
		value->OnHover.Connect(std::bind(&ScriptGUIClickable::OnHover, this));
		value->OnOut.Connect(std::bind(&ScriptGUIClickable::OnOut, this));
		value->OnDoubleClick.Connect(std::bind(&ScriptGUIClickable::OnDoubleClick, this));
	}

	void ScriptGUIClickable::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetContent", (void*)&ScriptGUIClickable::InternalSetContent);

		OnClickThunk = (OnClickThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnClick", "")->GetThunk();
		OnHoverThunk = (OnHoverThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnHover", "")->GetThunk();
		OnOutThunk = (OnOutThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnOut", "")->GetThunk();
		OnDoubleClickThunk = (OnDoubleClickThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnDoubleClick", "")->GetThunk();
	}

	void ScriptGUIClickable::InternalSetContent(ScriptGUIElementBase* thisPtr, __GUIContentInterop* content)
	{
		GUIContent tmpcontent;
		tmpcontent = ScriptGUIContent::FromInterop(*content);
		static_cast<GUIClickable*>(thisPtr->GetGuiElement())->SetContent(tmpcontent);
	}
}
