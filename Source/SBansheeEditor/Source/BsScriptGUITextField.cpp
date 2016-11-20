//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUITextField.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsGUITextField.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIContent.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUITextField::OnChangedThunkDef ScriptGUITextField::onChangedThunk;
	ScriptGUITextField::OnConfirmedThunkDef ScriptGUITextField::onConfirmedThunk;

	ScriptGUITextField::ScriptGUITextField(MonoObject* instance, GUITextField* textField)
		:TScriptGUIElement(instance, textField)
	{

	}

	void ScriptGUITextField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUITextField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUITextField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUITextField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", &ScriptGUITextField::internal_hasInputFocus);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUITextField::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnChanged", 1)->getThunk();
		onConfirmedThunk = (OnConfirmedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnConfirmed", 0)->getThunk();
	}

	void ScriptGUITextField::internal_createInstance(MonoObject* instance, bool multiline, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUITextField* guiField = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiField = GUITextField::create(multiline, nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiField = GUITextField::create(multiline, options, styleName);
		}

		guiField->onValueChanged.connect(std::bind(&ScriptGUITextField::onChanged, instance, _1));
		guiField->onConfirm.connect(std::bind(&ScriptGUITextField::onConfirmed, instance));

		new (bs_alloc<ScriptGUITextField>()) ScriptGUITextField(instance, guiField);
	}

	void ScriptGUITextField::internal_getValue(ScriptGUITextField* nativeInstance, MonoString** output)
	{
		if (nativeInstance->isDestroyed())
			*output = MonoUtil::wstringToMono(StringUtil::WBLANK);

		GUITextField* field = static_cast<GUITextField*>(nativeInstance->getGUIElement());
		*output = MonoUtil::wstringToMono(field->getValue());
	}

	void ScriptGUITextField::internal_setValue(ScriptGUITextField* nativeInstance, MonoString* value)
	{
		if (nativeInstance->isDestroyed())
			return;

		GUITextField* field = static_cast<GUITextField*>(nativeInstance->getGUIElement());
		field->setValue(MonoUtil::monoToWString(value));
	}

	void ScriptGUITextField::internal_hasInputFocus(ScriptGUITextField* nativeInstance, bool* output)
	{
		if (nativeInstance->isDestroyed())
			*output = false;

		GUITextField* field = static_cast<GUITextField*>(nativeInstance->getGUIElement());
		*output = field->hasInputFocus();
	}

	void ScriptGUITextField::internal_setTint(ScriptGUITextField* nativeInstance, Color* color)
	{
		if (nativeInstance->isDestroyed())
			return;

		GUITextField* field = (GUITextField*)nativeInstance->getGUIElement();
		field->setTint(*color);
	}

	void ScriptGUITextField::onChanged(MonoObject* instance, const WString& newValue)
	{
		MonoString* monoNewValue = MonoUtil::wstringToMono(newValue);
		MonoUtil::invokeThunk(onChangedThunk, instance, monoNewValue);
	}

	void ScriptGUITextField::onConfirmed(MonoObject* instance)
	{
		MonoUtil::invokeThunk(onConfirmedThunk, instance);
	}
}