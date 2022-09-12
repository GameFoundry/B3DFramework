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

namespace bs
{
	ScriptGUISliderH::OnChangedThunkDef ScriptGUISliderH::onChangedThunk;

	ScriptGUISliderH::ScriptGUISliderH(MonoObject* instance, GUISliderHorz* slider)
		:TScriptGUIElement(instance, slider)
	{

	}

	void ScriptGUISliderH::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUISliderH::internal_createInstance);
		metaData.scriptClass->AddInternalCall("Internal_SetPercent", (void*)&ScriptGUISliderH::internal_setPercent);
		metaData.scriptClass->AddInternalCall("Internal_GetPercent", (void*)&ScriptGUISliderH::internal_getPercent);
		metaData.scriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUISliderH::internal_setTint);
		metaData.scriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptGUISliderH::internal_getValue);
		metaData.scriptClass->AddInternalCall("Internal_SetValue", (void*)&ScriptGUISliderH::internal_setValue);
		metaData.scriptClass->AddInternalCall("Internal_SetRange", (void*)&ScriptGUISliderH::internal_setRange);
		metaData.scriptClass->AddInternalCall("Internal_GetRangeMaximum", (void*)&ScriptGUISliderH::internal_getRangeMaximum);
		metaData.scriptClass->AddInternalCall("Internal_GetRangeMinimum", (void*)&ScriptGUISliderH::internal_getRangeMinimum);
		metaData.scriptClass->AddInternalCall("Internal_SetStep", (void*)&ScriptGUISliderH::internal_setStep);
		metaData.scriptClass->AddInternalCall("Internal_GetStep", (void*)&ScriptGUISliderH::internal_getStep);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->GetMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUISliderH::internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray ScriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.get<GUIOption>(i));

		GUISliderHorz* guiSlider = GUISliderHorz::create(options, MonoUtil::monoToString(style));

		auto nativeInstance = new (bs_alloc<ScriptGUISliderH>()) ScriptGUISliderH(instance, guiSlider);
		guiSlider->onChanged.Connect(std::bind(&ScriptGUISliderH::onChanged, nativeInstance, _1));
	}

	void ScriptGUISliderH::internal_setPercent(ScriptGUISliderH* nativeInstance, float percent)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->GetGUIElement();
		slider->SetPercent(percent);
	}

	float ScriptGUISliderH::internal_getPercent(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->GetGUIElement();
		return slider->GetPercent();
	}

	float ScriptGUISliderH::internal_getValue(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->GetGUIElement();
		return slider->GetValue();
	}

	void ScriptGUISliderH::internal_setValue(ScriptGUISliderH* nativeInstance, float percent)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->GetGUIElement();
		return slider->SetValue(percent);
	}

	void ScriptGUISliderH::internal_setRange(ScriptGUISliderH* nativeInstance, float min, float max)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->GetGUIElement();
		return slider->SetRange(min, max);
	}

	float ScriptGUISliderH::internal_getRangeMaximum(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->GetGUIElement();
		return slider->GetRangeMaximum();
	}

	float ScriptGUISliderH::internal_getRangeMinimum(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->GetGUIElement();
		return slider->GetRangeMinimum();
	}

	void ScriptGUISliderH::internal_setStep(ScriptGUISliderH* nativeInstance, float step)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->GetGUIElement();
		return slider->SetStep(step);
	}

	float ScriptGUISliderH::internal_getStep(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->GetGUIElement();
		return slider->GetStep();
	}

	void ScriptGUISliderH::internal_setTint(ScriptGUISliderH* nativeInstance, Color* color)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->GetGUIElement();
		slider->SetTint(*color);
	}

	void ScriptGUISliderH::OnChanged(float percent)
	{
		MonoUtil::invokeThunk(onChangedThunk, getManagedInstance(), percent);
	}

	ScriptGUISliderV::OnChangedThunkDef ScriptGUISliderV::onChangedThunk;

	ScriptGUISliderV::ScriptGUISliderV(MonoObject* instance, GUISliderVert* slider)
		:TScriptGUIElement(instance, slider)
	{

	}

	void ScriptGUISliderV::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUISliderV::internal_createInstance);
		metaData.scriptClass->AddInternalCall("Internal_SetPercent", (void*)&ScriptGUISliderV::internal_setPercent);
		metaData.scriptClass->AddInternalCall("Internal_GetPercent", (void*)&ScriptGUISliderV::internal_getPercent);
		metaData.scriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUISliderV::internal_setTint);
		metaData.scriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptGUISliderV::internal_getValue);
		metaData.scriptClass->AddInternalCall("Internal_SetValue", (void*)&ScriptGUISliderV::internal_setValue);
		metaData.scriptClass->AddInternalCall("Internal_SetRange", (void*)&ScriptGUISliderV::internal_setRange);
		metaData.scriptClass->AddInternalCall("Internal_GetRangeMaximum", (void*)&ScriptGUISliderV::internal_getRangeMaximum);
		metaData.scriptClass->AddInternalCall("Internal_GetRangeMinimum", (void*)ScriptGUISliderV::internal_getRangeMinimum);
		metaData.scriptClass->AddInternalCall("Internal_SetStep", (void*)&ScriptGUISliderV::internal_setStep);
		metaData.scriptClass->AddInternalCall("Internal_GetStep", (void*)&ScriptGUISliderV::internal_getStep);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->GetMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUISliderV::internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray ScriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.get<GUIOption>(i));

		GUISliderVert* guiSlider = GUISliderVert::create(options, MonoUtil::monoToString(style));

		auto nativeInstance = new (bs_alloc<ScriptGUISliderV>()) ScriptGUISliderV(instance, guiSlider);
		guiSlider->onChanged.Connect(std::bind(&ScriptGUISliderV::onChanged, nativeInstance, _1));
	}

	void ScriptGUISliderV::internal_setPercent(ScriptGUISliderV* nativeInstance, float percent)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->GetGUIElement();
		slider->SetPercent(percent);
	}

	float ScriptGUISliderV::internal_getPercent(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->GetGUIElement();
		return slider->GetPercent();
	}

	float ScriptGUISliderV::internal_getValue(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->GetGUIElement();
		return slider->GetValue();
	}

	void ScriptGUISliderV::internal_setValue(ScriptGUISliderV* nativeInstance, float percent)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->GetGUIElement();
		return slider->SetValue(percent);
	}

	void ScriptGUISliderV::internal_setRange(ScriptGUISliderV* nativeInstance, float min, float max)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->GetGUIElement();
		return slider->SetRange(min, max);
	}

	float ScriptGUISliderV::internal_getRangeMaximum(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->GetGUIElement();
		return slider->GetRangeMaximum();
	}

	float ScriptGUISliderV::internal_getRangeMinimum(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->GetGUIElement();
		return slider->GetRangeMinimum();
	}

	void ScriptGUISliderV::internal_setStep(ScriptGUISliderV* nativeInstance, float step)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->GetGUIElement();
		return slider->SetStep(step);
	}

	float ScriptGUISliderV::internal_getStep(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->GetGUIElement();
		return slider->GetStep();
	}

	void ScriptGUISliderV::internal_setTint(ScriptGUISliderV* nativeInstance, Color* color)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->GetGUIElement();
		slider->SetTint(*color);
	}

	void ScriptGUISliderV::OnChanged(float percent)
	{
		MonoUtil::invokeThunk(onChangedThunk, getManagedInstance(), percent);
	}
}
