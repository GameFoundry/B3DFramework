//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUISlider.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUISlider.h"
#include "BsGUIOptions.h"
#include "BsScriptGUIElementStyle.h"
#include "BsScriptGUILayout.h"
#include "BsScriptHString.h"
#include "BsScriptGUIContent.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptGUISliderH::OnChangedThunkDef ScriptGUISliderH::onChangedThunk;

	ScriptGUISliderH::ScriptGUISliderH(MonoObject* instance, GUISliderHorz* slider)
		:TScriptGUIElement(instance, slider)
	{

	}

	void ScriptGUISliderH::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUISliderH::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetPercent", &ScriptGUISliderH::internal_setPercent);
		metaData.scriptClass->addInternalCall("Internal_GetPercent", &ScriptGUISliderH::internal_getPercent);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUISliderH::internal_setTint);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUISliderH::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUISliderH::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_SetRange", &ScriptGUISliderH::internal_setRange);
		metaData.scriptClass->addInternalCall("Internal_GetRangeMaximum", &ScriptGUISliderH::internal_getRangeMaximum);
		metaData.scriptClass->addInternalCall("Internal_GetRangeMinimum", &ScriptGUISliderH::internal_getRangeMinimum);
		metaData.scriptClass->addInternalCall("Internal_SetStep", &ScriptGUISliderH::internal_setStep);
		metaData.scriptClass->addInternalCall("Internal_GetStep", &ScriptGUISliderH::internal_getStep);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUISliderH::internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUISliderHorz* guiSlider = GUISliderHorz::create(options, toString(MonoUtil::monoToWString(style)));
		guiSlider->onChanged.connect(std::bind(&ScriptGUISliderH::onChanged, instance, _1));

		new (bs_alloc<ScriptGUISliderH>()) ScriptGUISliderH(instance, guiSlider);
	}

	void ScriptGUISliderH::internal_setPercent(ScriptGUISliderH* nativeInstance, float percent)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		slider->setPercent(percent);
	}

	float ScriptGUISliderH::internal_getPercent(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		return slider->getPercent();
	}

	float ScriptGUISliderH::internal_getValue(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		return slider->getValue();
	}

	void ScriptGUISliderH::internal_setValue(ScriptGUISliderH* nativeInstance, float percent)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		return slider->setValue(percent);
	}

	void ScriptGUISliderH::internal_setRange(ScriptGUISliderH* nativeInstance, float min, float max)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		return slider->setRange(min, max);
	}

	float ScriptGUISliderH::internal_getRangeMaximum(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		return slider->getRangeMaximum();
	}

	float ScriptGUISliderH::internal_getRangeMinimum(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		return slider->getRangeMinimum();
	}

	void ScriptGUISliderH::internal_setStep(ScriptGUISliderH* nativeInstance, float step)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		return slider->setStep(step);
	}

	float ScriptGUISliderH::internal_getStep(ScriptGUISliderH* nativeInstance)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		return slider->getStep();
	}

	void ScriptGUISliderH::internal_setTint(ScriptGUISliderH* nativeInstance, Color* color)
	{
		GUISliderHorz* slider = (GUISliderHorz*)nativeInstance->getGUIElement();
		slider->setTint(*color);
	}

	void ScriptGUISliderH::onChanged(MonoObject* instance, float percent)
	{
		MonoUtil::invokeThunk(onChangedThunk, instance, percent);
	}

	ScriptGUISliderV::OnChangedThunkDef ScriptGUISliderV::onChangedThunk;

	ScriptGUISliderV::ScriptGUISliderV(MonoObject* instance, GUISliderVert* slider)
		:TScriptGUIElement(instance, slider)
	{

	}

	void ScriptGUISliderV::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUISliderV::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetPercent", &ScriptGUISliderV::internal_setPercent);
		metaData.scriptClass->addInternalCall("Internal_GetPercent", &ScriptGUISliderV::internal_getPercent);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUISliderV::internal_setTint);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUISliderV::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUISliderV::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_SetRange", &ScriptGUISliderV::internal_setRange);
		metaData.scriptClass->addInternalCall("Internal_GetRangeMaximum", &ScriptGUISliderV::internal_getRangeMaximum);
		metaData.scriptClass->addInternalCall("Internal_GetRangeMinimum", &ScriptGUISliderV::internal_getRangeMinimum);
		metaData.scriptClass->addInternalCall("Internal_SetStep", &ScriptGUISliderV::internal_setStep);
		metaData.scriptClass->addInternalCall("Internal_GetStep", &ScriptGUISliderV::internal_getStep);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUISliderV::internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUISliderVert* guiSlider = GUISliderVert::create(options, toString(MonoUtil::monoToWString(style)));
		guiSlider->onChanged.connect(std::bind(&ScriptGUISliderV::onChanged, instance, _1));

		new (bs_alloc<ScriptGUISliderV>()) ScriptGUISliderV(instance, guiSlider);
	}

	void ScriptGUISliderV::internal_setPercent(ScriptGUISliderV* nativeInstance, float percent)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		slider->setPercent(percent);
	}

	float ScriptGUISliderV::internal_getPercent(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		return slider->getPercent();
	}

	float ScriptGUISliderV::internal_getValue(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		return slider->getValue();
	}

	void ScriptGUISliderV::internal_setValue(ScriptGUISliderV* nativeInstance, float percent)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		return slider->setValue(percent);
	}

	void ScriptGUISliderV::internal_setRange(ScriptGUISliderV* nativeInstance, float min, float max)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		return slider->setRange(min, max);
	}

	float ScriptGUISliderV::internal_getRangeMaximum(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		return slider->getRangeMaximum();
	}

	float ScriptGUISliderV::internal_getRangeMinimum(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		return slider->getRangeMinimum();
	}

	void ScriptGUISliderV::internal_setStep(ScriptGUISliderV* nativeInstance, float step)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		return slider->setStep(step);
	}

	float ScriptGUISliderV::internal_getStep(ScriptGUISliderV* nativeInstance)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		return slider->getStep();
	}

	void ScriptGUISliderV::internal_setTint(ScriptGUISliderV* nativeInstance, Color* color)
	{
		GUISliderVert* slider = (GUISliderVert*)nativeInstance->getGUIElement();
		slider->setTint(*color);
	}

	void ScriptGUISliderV::onChanged(MonoObject* instance, float percent)
	{
		MonoUtil::invokeThunk(onChangedThunk, instance, percent);
	}
}