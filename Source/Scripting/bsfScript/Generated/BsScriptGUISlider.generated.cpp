//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUISlider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISlider.h"

namespace bs
{
	ScriptGUISliderBase::OnChangedThunkDefinition ScriptGUISliderBase::OnChangedThunk; 

	ScriptGUISliderBase::ScriptGUISliderBase(MonoObject* managedInstance)
		:ScriptGUIInteractableBase(managedInstance)
	 { }

	void ScriptGUISliderBase::OnChanged(float p0)
	{
		MonoUtil::InvokeThunk(OnChangedThunk, GetManagedInstance(), p0);
	}

	void ScriptGUISliderBase::RegisterEvents(GUIElement* value)
	{
		static_cast<GUISlider*>(value)->OnChanged.Connect(std::bind(&ScriptGUISliderBase::OnChanged, this, std::placeholders::_1));
		ScriptGUIInteractableBase::RegisterEvents(value);
	}
	ScriptGUISlider::ScriptGUISlider(MonoObject* managedInstance, GUISlider* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
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

		OnChangedThunk = (OnChangedThunkDefinition)metaData.ScriptClass->GetMethodExact("Internal_OnChanged", "single")->GetThunk();
	}

	void ScriptGUISlider::InternalSetHandlePositionInPercent(ScriptGUIElementBase* self, float percent)
	{
		static_cast<GUISlider*>(self->GetGuiElement())->SetHandlePositionInPercent(percent);
	}

	float ScriptGUISlider::InternalGetHandlePositionInPercent(ScriptGUIElementBase* self)
	{
		float tmp__output;
		tmp__output = static_cast<GUISlider*>(self->GetGuiElement())->GetHandlePositionInPercent();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUISlider::InternalSetHandlePositionInRange(ScriptGUIElementBase* self, float value)
	{
		static_cast<GUISlider*>(self->GetGuiElement())->SetHandlePositionInRange(value);
	}

	float ScriptGUISlider::InternalGetHandlePositionInRange(ScriptGUIElementBase* self)
	{
		float tmp__output;
		tmp__output = static_cast<GUISlider*>(self->GetGuiElement())->GetHandlePositionInRange();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUISlider::InternalSetRange(ScriptGUIElementBase* self, float min, float max)
	{
		static_cast<GUISlider*>(self->GetGuiElement())->SetRange(min, max);
	}

	float ScriptGUISlider::InternalGetRangeMinimum(ScriptGUIElementBase* self)
	{
		float tmp__output;
		tmp__output = static_cast<GUISlider*>(self->GetGuiElement())->GetRangeMinimum();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptGUISlider::InternalGetRangeMaximum(ScriptGUIElementBase* self)
	{
		float tmp__output;
		tmp__output = static_cast<GUISlider*>(self->GetGuiElement())->GetRangeMaximum();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUISlider::InternalSetStep(ScriptGUIElementBase* self, float step)
	{
		static_cast<GUISlider*>(self->GetGuiElement())->SetStep(step);
	}

	float ScriptGUISlider::InternalGetStep(ScriptGUIElementBase* self)
	{
		float tmp__output;
		tmp__output = static_cast<GUISlider*>(self->GetGuiElement())->GetStep();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
