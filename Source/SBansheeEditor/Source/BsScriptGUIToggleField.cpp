//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIToggleField.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsGUIToggleField.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIContent.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIToggleField::OnChangedThunkDef ScriptGUIToggleField::onChangedThunk;

	ScriptGUIToggleField::ScriptGUIToggleField(MonoObject* instance, GUIToggleField* toggleField)
		:TScriptGUIElement(instance, toggleField)
	{

	}

	void ScriptGUIToggleField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIToggleField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUIToggleField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUIToggleField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUIToggleField::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUIToggleField::internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUIToggleField* guiField = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiField = GUIToggleField::create(nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiField = GUIToggleField::create(options, styleName);
		}

		guiField->onValueChanged.connect(std::bind(&ScriptGUIToggleField::onChanged, instance, _1));

		new (bs_alloc<ScriptGUIToggleField>()) ScriptGUIToggleField(instance, guiField);
	}

	void ScriptGUIToggleField::internal_getValue(ScriptGUIToggleField* nativeInstance, bool* output)
	{
		GUIToggleField* toggleField = static_cast<GUIToggleField*>(nativeInstance->getGUIElement());
		*output = toggleField->getValue();
	}

	void ScriptGUIToggleField::internal_setValue(ScriptGUIToggleField* nativeInstance, bool value)
	{
		GUIToggleField* toggleField = static_cast<GUIToggleField*>(nativeInstance->getGUIElement());
		return toggleField->setValue(value);
	}

	void ScriptGUIToggleField::internal_setTint(ScriptGUIToggleField* nativeInstance, Color* color)
	{
		GUIToggleField* toggleField = (GUIToggleField*)nativeInstance->getGUIElement();
		toggleField->setTint(*color);
	}

	void ScriptGUIToggleField::onChanged(MonoObject* instance, bool newValue)
	{
		MonoUtil::invokeThunk(onChangedThunk, instance, newValue);
	}
}