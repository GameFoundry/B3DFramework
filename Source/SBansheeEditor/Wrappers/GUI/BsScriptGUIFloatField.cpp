//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/GUI/BsScriptGUIFloatField.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUIFloatField.h"
#include "GUI/BsGUIOptions.h"
#include "GUI/BsGUIContent.h"
#include "Wrappers/GUI/BsScriptGUIContent.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIFloatField::OnChangedThunkDef ScriptGUIFloatField::onChangedThunk;
	ScriptGUIFloatField::OnConfirmedThunkDef ScriptGUIFloatField::onConfirmedThunk;

	ScriptGUIFloatField::ScriptGUIFloatField(MonoObject* instance, GUIFloatField* floatField)
		:TScriptGUIElement(instance, floatField)
	{

	}

	void ScriptGUIFloatField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIFloatField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", (void*)&ScriptGUIFloatField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", (void*)&ScriptGUIFloatField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", (void*)&ScriptGUIFloatField::internal_hasInputFocus);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUIFloatField::internal_setTint);
		metaData.scriptClass->addInternalCall("Internal_SetRange", (void*)&ScriptGUIFloatField::internal_setRange);
		metaData.scriptClass->addInternalCall("Internal_SetStep", (void*)&ScriptGUIFloatField::internal_setStep);
		metaData.scriptClass->addInternalCall("Internal_GetStep", (void*)&ScriptGUIFloatField::internal_getStep);

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

		auto nativeInstance = new (bs_alloc<ScriptGUIFloatField>()) ScriptGUIFloatField(instance, guiFloatField);

		guiFloatField->onValueChanged.connect(std::bind(&ScriptGUIFloatField::onChanged, nativeInstance, _1));
		guiFloatField->onConfirm.connect(std::bind(&ScriptGUIFloatField::onConfirmed, nativeInstance));
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

	void ScriptGUIFloatField::onChanged(float newValue)
	{
		MonoUtil::invokeThunk(onChangedThunk, getManagedInstance(), newValue);
	}

	void ScriptGUIFloatField::onConfirmed()
	{
		MonoUtil::invokeThunk(onConfirmedThunk, getManagedInstance());
	}
}