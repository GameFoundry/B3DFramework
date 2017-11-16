//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/GUI/BsScriptGUITextField.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUITextField.h"
#include "GUI/BsGUIOptions.h"
#include "GUI/BsGUIContent.h"
#include "Wrappers/GUI/BsScriptGUIContent.h"

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
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUITextField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", (void*)&ScriptGUITextField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", (void*)&ScriptGUITextField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", (void*)&ScriptGUITextField::internal_hasInputFocus);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUITextField::internal_setTint);

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

		auto nativeInstance = new (bs_alloc<ScriptGUITextField>()) ScriptGUITextField(instance, guiField);

		guiField->onValueChanged.connect(std::bind(&ScriptGUITextField::onChanged, nativeInstance, _1));
		guiField->onConfirm.connect(std::bind(&ScriptGUITextField::onConfirmed, nativeInstance));
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

	void ScriptGUITextField::onChanged(const WString& newValue)
	{
		MonoString* monoNewValue = MonoUtil::wstringToMono(newValue);
		MonoUtil::invokeThunk(onChangedThunk, getManagedInstance(), monoNewValue);
	}

	void ScriptGUITextField::onConfirmed()
	{
		MonoUtil::invokeThunk(onConfirmedThunk, getManagedInstance());
	}
}