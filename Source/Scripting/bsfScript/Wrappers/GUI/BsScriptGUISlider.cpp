//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUISlider.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUISlider.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptGUIElementStyle.generated.h"
#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"

using namespace std::placeholders;

using namespace bs;
ScriptGUISliderH::OnChangedThunkDef ScriptGUISliderH::onChangedThunk;

ScriptGUISliderH::ScriptGUISliderH(MonoObject* instance, GUIHorizontalSlider* slider)
	: TScriptGUIInteractable(instance, slider)
{
}

void ScriptGUISliderH::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUISliderH::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_SetPercent", (void*)&ScriptGUISliderH::InternalSetPercent);
	metaData.ScriptClass->AddInternalCall("Internal_GetPercent", (void*)&ScriptGUISliderH::InternalGetPercent);
	metaData.ScriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptGUISliderH::InternalGetValue);
	metaData.ScriptClass->AddInternalCall("Internal_SetValue", (void*)&ScriptGUISliderH::InternalSetValue);
	metaData.ScriptClass->AddInternalCall("Internal_SetRange", (void*)&ScriptGUISliderH::InternalSetRange);
	metaData.ScriptClass->AddInternalCall("Internal_GetRangeMaximum", (void*)&ScriptGUISliderH::InternalGetRangeMaximum);
	metaData.ScriptClass->AddInternalCall("Internal_GetRangeMinimum", (void*)&ScriptGUISliderH::InternalGetRangeMinimum);
	metaData.ScriptClass->AddInternalCall("Internal_SetStep", (void*)&ScriptGUISliderH::InternalSetStep);
	metaData.ScriptClass->AddInternalCall("Internal_GetStep", (void*)&ScriptGUISliderH::InternalGetStep);

	onChangedThunk = (OnChangedThunkDef)metaData.ScriptClass->GetMethod("DoOnChanged", 1)->GetThunk();
}

void ScriptGUISliderH::InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIHorizontalSlider* guiSlider = GUIHorizontalSlider::Create(MonoUtil::MonoToString(style), options);

	auto nativeInstance = new(B3DAllocate<ScriptGUISliderH>()) ScriptGUISliderH(instance, guiSlider);
	guiSlider->OnChanged.Connect(std::bind(&::bs::ScriptGUISliderH::OnChanged, nativeInstance, _1));
}

void ScriptGUISliderH::InternalSetPercent(ScriptGUISliderH* nativeInstance, float percent)
{
	GUIHorizontalSlider* slider = (GUIHorizontalSlider*)nativeInstance->GetGuiElement();
	slider->SetPercent(percent);
}

float ScriptGUISliderH::InternalGetPercent(ScriptGUISliderH* nativeInstance)
{
	GUIHorizontalSlider* slider = (GUIHorizontalSlider*)nativeInstance->GetGuiElement();
	return slider->GetPercent();
}

float ScriptGUISliderH::InternalGetValue(ScriptGUISliderH* nativeInstance)
{
	GUIHorizontalSlider* slider = (GUIHorizontalSlider*)nativeInstance->GetGuiElement();
	return slider->GetValue();
}

void ScriptGUISliderH::InternalSetValue(ScriptGUISliderH* nativeInstance, float percent)
{
	GUIHorizontalSlider* slider = (GUIHorizontalSlider*)nativeInstance->GetGuiElement();
	return slider->SetValue(percent);
}

void ScriptGUISliderH::InternalSetRange(ScriptGUISliderH* nativeInstance, float min, float max)
{
	GUIHorizontalSlider* slider = (GUIHorizontalSlider*)nativeInstance->GetGuiElement();
	return slider->SetRange(min, max);
}

float ScriptGUISliderH::InternalGetRangeMaximum(ScriptGUISliderH* nativeInstance)
{
	GUIHorizontalSlider* slider = (GUIHorizontalSlider*)nativeInstance->GetGuiElement();
	return slider->GetRangeMaximum();
}

float ScriptGUISliderH::InternalGetRangeMinimum(ScriptGUISliderH* nativeInstance)
{
	GUIHorizontalSlider* slider = (GUIHorizontalSlider*)nativeInstance->GetGuiElement();
	return slider->GetRangeMinimum();
}

void ScriptGUISliderH::InternalSetStep(ScriptGUISliderH* nativeInstance, float step)
{
	GUIHorizontalSlider* slider = (GUIHorizontalSlider*)nativeInstance->GetGuiElement();
	return slider->SetStep(step);
}

float ScriptGUISliderH::InternalGetStep(ScriptGUISliderH* nativeInstance)
{
	GUIHorizontalSlider* slider = (GUIHorizontalSlider*)nativeInstance->GetGuiElement();
	return slider->GetStep();
}

void ScriptGUISliderH::OnChanged(float percent)
{
	MonoUtil::InvokeThunk(onChangedThunk, GetManagedInstance(), percent);
}

ScriptGUISliderV::OnChangedThunkDef ScriptGUISliderV::onChangedThunk;

ScriptGUISliderV::ScriptGUISliderV(MonoObject* instance, GUIVerticalSlider* slider)
	: TScriptGUIInteractable(instance, slider)
{
}

void ScriptGUISliderV::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUISliderV::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_SetPercent", (void*)&ScriptGUISliderV::InternalSetPercent);
	metaData.ScriptClass->AddInternalCall("Internal_GetPercent", (void*)&ScriptGUISliderV::InternalGetPercent);
	metaData.ScriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptGUISliderV::InternalGetValue);
	metaData.ScriptClass->AddInternalCall("Internal_SetValue", (void*)&ScriptGUISliderV::InternalSetValue);
	metaData.ScriptClass->AddInternalCall("Internal_SetRange", (void*)&ScriptGUISliderV::InternalSetRange);
	metaData.ScriptClass->AddInternalCall("Internal_GetRangeMaximum", (void*)&ScriptGUISliderV::InternalGetRangeMaximum);
	metaData.ScriptClass->AddInternalCall("Internal_GetRangeMinimum", (void*)ScriptGUISliderV::InternalGetRangeMinimum);
	metaData.ScriptClass->AddInternalCall("Internal_SetStep", (void*)&ScriptGUISliderV::InternalSetStep);
	metaData.ScriptClass->AddInternalCall("Internal_GetStep", (void*)&ScriptGUISliderV::InternalGetStep);

	onChangedThunk = (OnChangedThunkDef)metaData.ScriptClass->GetMethod("DoOnChanged", 1)->GetThunk();
}

void ScriptGUISliderV::InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIVerticalSlider* guiSlider = GUIVerticalSlider::Create(MonoUtil::MonoToString(style), options);

	auto nativeInstance = new(B3DAllocate<ScriptGUISliderV>()) ScriptGUISliderV(instance, guiSlider);
	guiSlider->OnChanged.Connect(std::bind(&::bs::ScriptGUISliderV::OnChanged, nativeInstance, _1));
}

void ScriptGUISliderV::InternalSetPercent(ScriptGUISliderV* nativeInstance, float percent)
{
	GUIVerticalSlider* slider = (GUIVerticalSlider*)nativeInstance->GetGuiElement();
	slider->SetPercent(percent);
}

float ScriptGUISliderV::InternalGetPercent(ScriptGUISliderV* nativeInstance)
{
	GUIVerticalSlider* slider = (GUIVerticalSlider*)nativeInstance->GetGuiElement();
	return slider->GetPercent();
}

float ScriptGUISliderV::InternalGetValue(ScriptGUISliderV* nativeInstance)
{
	GUIVerticalSlider* slider = (GUIVerticalSlider*)nativeInstance->GetGuiElement();
	return slider->GetValue();
}

void ScriptGUISliderV::InternalSetValue(ScriptGUISliderV* nativeInstance, float percent)
{
	GUIVerticalSlider* slider = (GUIVerticalSlider*)nativeInstance->GetGuiElement();
	return slider->SetValue(percent);
}

void ScriptGUISliderV::InternalSetRange(ScriptGUISliderV* nativeInstance, float min, float max)
{
	GUIVerticalSlider* slider = (GUIVerticalSlider*)nativeInstance->GetGuiElement();
	return slider->SetRange(min, max);
}

float ScriptGUISliderV::InternalGetRangeMaximum(ScriptGUISliderV* nativeInstance)
{
	GUIVerticalSlider* slider = (GUIVerticalSlider*)nativeInstance->GetGuiElement();
	return slider->GetRangeMaximum();
}

float ScriptGUISliderV::InternalGetRangeMinimum(ScriptGUISliderV* nativeInstance)
{
	GUIVerticalSlider* slider = (GUIVerticalSlider*)nativeInstance->GetGuiElement();
	return slider->GetRangeMinimum();
}

void ScriptGUISliderV::InternalSetStep(ScriptGUISliderV* nativeInstance, float step)
{
	GUIVerticalSlider* slider = (GUIVerticalSlider*)nativeInstance->GetGuiElement();
	return slider->SetStep(step);
}

float ScriptGUISliderV::InternalGetStep(ScriptGUISliderV* nativeInstance)
{
	GUIVerticalSlider* slider = (GUIVerticalSlider*)nativeInstance->GetGuiElement();
	return slider->GetStep();
}

void ScriptGUISliderV::OnChanged(float percent)
{
	MonoUtil::InvokeThunk(onChangedThunk, GetManagedInstance(), percent);
}
