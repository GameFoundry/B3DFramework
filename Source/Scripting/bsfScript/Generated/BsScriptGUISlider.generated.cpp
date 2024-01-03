//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUISlider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISlider.h"

namespace bs
{
	ScriptGUISlider::OnChangedThunkDef ScriptGUISlider::OnChangedThunk; 

	ScriptGUISlider::ScriptGUISlider(MonoObject* managedInstance, GUISlider* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		value->OnChanged.Connect(std::bind(&ScriptGUISlider::OnChanged, this, std::placeholders::_1));
	}

	void ScriptGUISlider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetHandlePositionInPercent", (void*)&ScriptGUISlider::InternalSetHandlePositionInPercent);
		metaData.ScriptClass->AddInternalCall("Internal_GetHandlePositionInPercent", (void*)&ScriptGUISlider::InternalGetHandlePositionInPercent);
		metaData.ScriptClass->AddInternalCall("Internal_SetHandlePositionInRange", (void*)&ScriptGUISlider::InternalSetHandlePositionInRange);
		metaData.ScriptClass->AddInternalCall("Internal_GetHandlePositionInRange", (void*)&ScriptGUISlider::InternalGetHandlePositionInRange);
		metaData.ScriptClass->AddInternalCall("Internal_SetRange", (void*)&ScriptGUISlider::InternalSetRange);
		metaData.ScriptClass->AddInternalCall("Internal_GetRangeMinimum", (void*)&ScriptGUISlider::InternalGetRangeMinimum);
		metaData.ScriptClass->AddInternalCall("Internal_GetRangeMaximum", (void*)&ScriptGUISlider::InternalGetRangeMaximum);
		metaData.ScriptClass->AddInternalCall("Internal_SetStep", (void*)&ScriptGUISlider::InternalSetStep);
		metaData.ScriptClass->AddInternalCall("Internal_GetStep", (void*)&ScriptGUISlider::InternalGetStep);

		OnChangedThunk = (OnChangedThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnChanged", "single")->GetThunk();
	}

	void ScriptGUISlider::OnChanged(float p0)
	{
		MonoUtil::InvokeThunk(OnChangedThunk, GetManagedInstance(), p0);
	}
	void ScriptGUISlider::InternalSetHandlePositionInPercent(ScriptGUIElementBase* thisPtr, float percent)
	{
		static_cast<GUISlider*>(thisPtr->GetGuiElement())->SetHandlePositionInPercent(percent);
	}

	float ScriptGUISlider::InternalGetHandlePositionInPercent(ScriptGUIElementBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_cast<GUISlider*>(thisPtr->GetGuiElement())->GetHandlePositionInPercent();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUISlider::InternalSetHandlePositionInRange(ScriptGUIElementBase* thisPtr, float value)
	{
		static_cast<GUISlider*>(thisPtr->GetGuiElement())->SetHandlePositionInRange(value);
	}

	float ScriptGUISlider::InternalGetHandlePositionInRange(ScriptGUIElementBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_cast<GUISlider*>(thisPtr->GetGuiElement())->GetHandlePositionInRange();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUISlider::InternalSetRange(ScriptGUIElementBase* thisPtr, float min, float max)
	{
		static_cast<GUISlider*>(thisPtr->GetGuiElement())->SetRange(min, max);
	}

	float ScriptGUISlider::InternalGetRangeMinimum(ScriptGUIElementBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_cast<GUISlider*>(thisPtr->GetGuiElement())->GetRangeMinimum();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptGUISlider::InternalGetRangeMaximum(ScriptGUIElementBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_cast<GUISlider*>(thisPtr->GetGuiElement())->GetRangeMaximum();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUISlider::InternalSetStep(ScriptGUIElementBase* thisPtr, float step)
	{
		static_cast<GUISlider*>(thisPtr->GetGuiElement())->SetStep(step);
	}

	float ScriptGUISlider::InternalGetStep(ScriptGUIElementBase* thisPtr)
	{
		float tmp__output;
		tmp__output = static_cast<GUISlider*>(thisPtr->GetGuiElement())->GetStep();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
