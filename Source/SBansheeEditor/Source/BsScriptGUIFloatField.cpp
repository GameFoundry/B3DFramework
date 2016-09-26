//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIFloatField.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsGUIFloatField.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIContent.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptGUIFloatField::OnChangedThunkDef ScriptGUIFloatField::onChangedThunk;
	ScriptGUIFloatField::OnConfirmedThunkDef ScriptGUIFloatField::onConfirmedThunk;

	ScriptGUIFloatField::ScriptGUIFloatField(MonoObject* instance, GUIFloatField* floatField)
		:TScriptGUIElement(instance, floatField)
	{

	}

	void ScriptGUIFloatField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIFloatField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUIFloatField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUIFloatField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", &ScriptGUIFloatField::internal_hasInputFocus);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUIFloatField::internal_setTint);
		metaData.scriptClass->addInternalCall("Internal_SetRange", &ScriptGUIFloatField::internal_setRange);
		metaData.scriptClass->addInternalCall("Internal_SetStep", &ScriptGUIFloatField::internal_setStep);
		metaData.scriptClass->addInternalCall("Internal_GetStep", &ScriptGUIFloatField::internal_getStep);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnChanged", 1)->getThunk();
		onConfirmedThunk = (OnConfirmedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnConfirmed", 0)->getThunk();
	}

	void ScriptGUIFloatField::internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUIFloatField* guiFloatField = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiFloatField = GUIFloatField::create(nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiFloatField = GUIFloatField::create(options, styleName);
		}

		guiFloatField->onValueChanged.connect(std::bind(&ScriptGUIFloatField::onChanged, instance, _1));
		guiFloatField->onConfirm.connect(std::bind(&ScriptGUIFloatField::onConfirmed, instance));

		new (bs_alloc<ScriptGUIFloatField>()) ScriptGUIFloatField(instance, guiFloatField);
	}

	float ScriptGUIFloatField::internal_getValue(ScriptGUIFloatField* nativeInstance)
	{
		GUIFloatField* floatField = static_cast<GUIFloatField*>(nativeInstance->getGUIElement());
		return floatField->getValue();
	}

	float ScriptGUIFloatField::internal_setValue(ScriptGUIFloatField* nativeInstance, float value)
	{
		GUIFloatField* floatField = static_cast<GUIFloatField*>(nativeInstance->getGUIElement());
		return floatField->setValue(value);
	}

	bool ScriptGUIFloatField::internal_hasInputFocus(ScriptGUIFloatField* nativeInstance)
	{
		GUIFloatField* floatField = static_cast<GUIFloatField*>(nativeInstance->getGUIElement());
		return floatField->hasInputFocus();
	}

	void ScriptGUIFloatField::internal_setRange(ScriptGUIFloatField* nativeInstance, float min, float max)
	{
		GUIFloatField* intField = static_cast<GUIFloatField*>(nativeInstance->getGUIElement());
		intField->setRange(min, max);
	}

	void ScriptGUIFloatField::internal_setTint(ScriptGUIFloatField* nativeInstance, Color* color)
	{
		GUIFloatField* floatField = (GUIFloatField*)nativeInstance->getGUIElement();
		floatField->setTint(*color);
	}

	void ScriptGUIFloatField::internal_setStep(ScriptGUIFloatField* nativeInstance, float step)
	{
		GUIFloatField* floatField = (GUIFloatField*)nativeInstance->getGUIElement();
		floatField->setStep(step);
	}

	float ScriptGUIFloatField::internal_getStep(ScriptGUIFloatField* nativeInstance)
	{
		GUIFloatField* floatField = (GUIFloatField*)nativeInstance->getGUIElement();
		return floatField->getStep();
	}

	void ScriptGUIFloatField::onChanged(MonoObject* instance, float newValue)
	{
		MonoUtil::invokeThunk(onChangedThunk, instance, newValue);
	}

	void ScriptGUIFloatField::onConfirmed(MonoObject* instance)
	{
		MonoUtil::invokeThunk(onConfirmedThunk, instance);
	}
}