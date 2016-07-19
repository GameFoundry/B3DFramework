//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIIntField.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsGUIIntField.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIContent.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptGUIIntField::OnChangedThunkDef ScriptGUIIntField::onChangedThunk;
	ScriptGUIIntField::OnConfirmedThunkDef ScriptGUIIntField::onConfirmedThunk;

	ScriptGUIIntField::ScriptGUIIntField(MonoObject* instance, GUIIntField* intField)
		:TScriptGUIElement(instance, intField)
	{

	}

	void ScriptGUIIntField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIIntField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUIIntField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUIIntField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", &ScriptGUIIntField::internal_hasInputFocus);
		metaData.scriptClass->addInternalCall("Internal_SetRange", &ScriptGUIIntField::internal_setRange);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUIIntField::internal_setTint);
		metaData.scriptClass->addInternalCall("Internal_SetStep", &ScriptGUIIntField::internal_setStep);
		metaData.scriptClass->addInternalCall("Internal_GetStep", &ScriptGUIIntField::internal_getStep);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnChanged", 1)->getThunk();
		onConfirmedThunk = (OnConfirmedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnConfirmed", 0)->getThunk();
	}

	void ScriptGUIIntField::internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth, 
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUIIntField* guiIntField = nullptr;
		if(withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiIntField = GUIIntField::create(nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiIntField = GUIIntField::create(options, styleName);
		}

		guiIntField->onValueChanged.connect(std::bind(&ScriptGUIIntField::onChanged, instance, _1));
		guiIntField->onConfirm.connect(std::bind(&ScriptGUIIntField::onConfirmed, instance));

		new (bs_alloc<ScriptGUIIntField>()) ScriptGUIIntField(instance, guiIntField);
	}

	void ScriptGUIIntField::internal_getValue(ScriptGUIIntField* nativeInstance, INT32* output)
	{
		GUIIntField* intField = static_cast<GUIIntField*>(nativeInstance->getGUIElement());
		*output = intField->getValue();
	}

	INT32 ScriptGUIIntField::internal_setValue(ScriptGUIIntField* nativeInstance, INT32 value)
	{
		GUIIntField* intField = static_cast<GUIIntField*>(nativeInstance->getGUIElement());
		return intField->setValue(value);
	}

	void ScriptGUIIntField::internal_hasInputFocus(ScriptGUIIntField* nativeInstance, bool* output)
	{
		GUIIntField* intField = static_cast<GUIIntField*>(nativeInstance->getGUIElement());
		*output = intField->hasInputFocus();
	}

	void ScriptGUIIntField::internal_setRange(ScriptGUIIntField* nativeInstance, INT32 min, INT32 max)
	{
		GUIIntField* intField = static_cast<GUIIntField*>(nativeInstance->getGUIElement());
		intField->setRange(min, max);
	}

	void ScriptGUIIntField::internal_setTint(ScriptGUIIntField* nativeInstance, Color* color)
	{
		GUIIntField* intField = (GUIIntField*)nativeInstance->getGUIElement();
		intField->setTint(*color);
	}

	void ScriptGUIIntField::internal_setStep(ScriptGUIIntField* nativeInstance, INT32 step)
	{
		GUIIntField* intField = (GUIIntField*)nativeInstance->getGUIElement();
		intField->setStep(step);
	}

	INT32 ScriptGUIIntField::internal_getStep(ScriptGUIIntField* nativeInstance)
	{
		GUIIntField* intField = (GUIIntField*)nativeInstance->getGUIElement();
		return intField->getStep();
	}

	void ScriptGUIIntField::onChanged(MonoObject* instance, INT32 newValue)
	{
		MonoUtil::invokeThunk(onChangedThunk, instance, newValue);
	}

	void ScriptGUIIntField::onConfirmed(MonoObject* instance)
	{
		MonoUtil::invokeThunk(onConfirmedThunk, instance);
	}
}