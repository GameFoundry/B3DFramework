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
	ScriptGUIClickable::OnClickThunkDef ScriptGUIClickable::OnClickThunk; 
	ScriptGUIClickable::OnHoverThunkDef ScriptGUIClickable::OnHoverThunk; 
	ScriptGUIClickable::OnOutThunkDef ScriptGUIClickable::OnOutThunk; 
	ScriptGUIClickable::OnDoubleClickThunkDef ScriptGUIClickable::OnDoubleClickThunk; 

	ScriptGUIClickable::ScriptGUIClickable(MonoObject* managedInstance, GUIClickable* value)
		:TScriptGUIElement(managedInstance, value)
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

	void ScriptGUIClickable::OnClick()
	{
		MonoUtil::InvokeThunk(OnClickThunk, GetManagedInstance());
	}

	void ScriptGUIClickable::OnHover()
	{
		MonoUtil::InvokeThunk(OnHoverThunk, GetManagedInstance());
	}

	void ScriptGUIClickable::OnOut()
	{
		MonoUtil::InvokeThunk(OnOutThunk, GetManagedInstance());
	}

	void ScriptGUIClickable::OnDoubleClick()
	{
		MonoUtil::InvokeThunk(OnDoubleClickThunk, GetManagedInstance());
	}
	void ScriptGUIClickable::InternalSetContent(ScriptGUIElementBaseTBase* thisPtr, __GUIContentInterop* content)
	{
		GUIContent tmpcontent;
		tmpcontent = ScriptGUIContent::FromInterop(*content);
		static_cast<GUIClickable*>(thisPtr->GetGuiElement())->SetContent(tmpcontent);
	}
}
