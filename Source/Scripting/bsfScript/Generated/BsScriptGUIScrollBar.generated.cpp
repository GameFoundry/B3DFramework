//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIScrollBar.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIScrollBar.h"

namespace bs
{
	ScriptGUIScrollBarBase::OnScrollOrResizeThunkDef ScriptGUIScrollBarBase::OnScrollOrResizeThunk; 

	ScriptGUIScrollBarBase::ScriptGUIScrollBarBase(MonoObject* managedInstance)
		:ScriptGUIInteractableBase(managedInstance)
	 { }

	void ScriptGUIScrollBarBase::OnScrollOrResize(float p0, float p1)
	{
		MonoUtil::InvokeThunk(OnScrollOrResizeThunk, GetManagedInstance(), p0, p1);
	}

	void ScriptGUIScrollBarBase::RegisterEvents(GUIElement* value)
	{
		static_cast<GUIScrollBar*>(value)->OnScrollOrResize.Connect(std::bind(&ScriptGUIScrollBarBase::OnScrollOrResize, this, std::placeholders::_1, std::placeholders::_2));
		ScriptGUIInteractableBase::RegisterEvents(value);
	}
	ScriptGUIScrollBar::ScriptGUIScrollBar(MonoObject* managedInstance, GUIScrollBar* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
	}

	void ScriptGUIScrollBar::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetScrollHandlePosition", (void*)&ScriptGUIScrollBar::InternalSetScrollHandlePosition);
		metaData.ScriptClass->AddInternalCall("Internal_GetScrollHandlePosition", (void*)&ScriptGUIScrollBar::InternalGetScrollHandlePosition);
		metaData.ScriptClass->AddInternalCall("Internal_SetScrollHandleSize", (void*)&ScriptGUIScrollBar::InternalSetScrollHandleSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetScrollHandleSize", (void*)&ScriptGUIScrollBar::InternalGetScrollHandleSize);

		OnScrollOrResizeThunk = (OnScrollOrResizeThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnScrollOrResize", "single,single")->GetThunk();
	}

	void ScriptGUIScrollBar::InternalSetScrollHandlePosition(ScriptGUIElementBase* thisPtr, float pct)
	{
		static_cast<GUIScrollBar*>(thisPtr->GetGuiElement())->SetScrollHandlePosition(pct);
	}

	float ScriptGUIScrollBar::InternalGetScrollHandlePosition(ScriptGUIElementBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_cast<GUIScrollBar*>(thisPtr->GetGuiElement())->GetScrollHandlePosition();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIScrollBar::InternalSetScrollHandleSize(ScriptGUIElementBase* thisPtr, float pct)
	{
		static_cast<GUIScrollBar*>(thisPtr->GetGuiElement())->SetScrollHandleSize(pct);
	}

	float ScriptGUIScrollBar::InternalGetScrollHandleSize(ScriptGUIElementBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_cast<GUIScrollBar*>(thisPtr->GetGuiElement())->GetScrollHandleSize();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
