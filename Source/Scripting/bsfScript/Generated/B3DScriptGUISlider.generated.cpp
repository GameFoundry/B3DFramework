//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUISlider.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/GUI/B3DGUISlider.h"

namespace b3d
{
	ScriptGUISliderWrapperBase::OnChangedThunkDefinition ScriptGUISliderWrapperBase::OnChangedThunk; 

	void ScriptGUISliderWrapperBase::OnChanged(float p0)
	{
		MonoUtil::InvokeThunk(OnChangedThunk, GetScriptObject(), p0);
	}

	void ScriptGUISliderWrapperBase::RegisterEvents()
	{
		OnChangedConnection = static_cast<GUISlider*>(GetNativeObject())->OnChanged.Connect([this](float p0) { OnChanged(p0); });
		ScriptGUIInteractableWrapperBase::RegisterEvents();
	}
	void ScriptGUISliderWrapperBase::UnregisterEvents()
	{
		OnChangedConnection.Disconnect();
		ScriptGUIInteractableWrapperBase::UnregisterEvents();
	}
	ScriptGUISlider::ScriptGUISlider(GUISlider* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUISlider::~ScriptGUISlider()
	{
		UnregisterEvents();
	}

	void ScriptGUISlider::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHandlePositionInPercent", (void*)&ScriptGUISlider::InternalSetHandlePositionInPercent);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHandlePositionInPercent", (void*)&ScriptGUISlider::InternalGetHandlePositionInPercent);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetHandlePositionInRange", (void*)&ScriptGUISlider::InternalSetHandlePositionInRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetHandlePositionInRange", (void*)&ScriptGUISlider::InternalGetHandlePositionInRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRange", (void*)&ScriptGUISlider::InternalSetRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRangeMinimum", (void*)&ScriptGUISlider::InternalGetRangeMinimum);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRangeMaximum", (void*)&ScriptGUISlider::InternalGetRangeMaximum);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetStep", (void*)&ScriptGUISlider::InternalSetStep);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetStep", (void*)&ScriptGUISlider::InternalGetStep);

		OnChangedThunk = (OnChangedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnChanged", "single")->GetThunk();
	}

	MonoObject* ScriptGUISlider::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUISlider::InternalSetHandlePositionInPercent(ScriptGUISliderWrapperBase* self, float percent)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUISlider*>(self->GetNativeObject())->SetHandlePositionInPercent(percent);
	}

	float ScriptGUISlider::InternalGetHandlePositionInPercent(ScriptGUISliderWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUISlider*>(self->GetNativeObject())->GetHandlePositionInPercent();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUISlider::InternalSetHandlePositionInRange(ScriptGUISliderWrapperBase* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUISlider*>(self->GetNativeObject())->SetHandlePositionInRange(value);
	}

	float ScriptGUISlider::InternalGetHandlePositionInRange(ScriptGUISliderWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUISlider*>(self->GetNativeObject())->GetHandlePositionInRange();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUISlider::InternalSetRange(ScriptGUISliderWrapperBase* self, float minimum, float maximum)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUISlider*>(self->GetNativeObject())->SetRange(minimum, maximum);
	}

	float ScriptGUISlider::InternalGetRangeMinimum(ScriptGUISliderWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUISlider*>(self->GetNativeObject())->GetRangeMinimum();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptGUISlider::InternalGetRangeMaximum(ScriptGUISliderWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUISlider*>(self->GetNativeObject())->GetRangeMaximum();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUISlider::InternalSetStep(ScriptGUISliderWrapperBase* self, float step)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUISlider*>(self->GetNativeObject())->SetStep(step);
	}

	float ScriptGUISlider::InternalGetStep(ScriptGUISliderWrapperBase* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUISlider*>(self->GetNativeObject())->GetStep();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
