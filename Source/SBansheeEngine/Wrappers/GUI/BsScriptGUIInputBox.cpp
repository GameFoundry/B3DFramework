//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/GUI/BsScriptGUIInputBox.h"
#include "BsScriptMeta.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUIInputBox.h"
#include "GUI/BsGUIOptions.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIInputBox::OnChangedThunkDef ScriptGUIInputBox::onChangedThunk;
	ScriptGUIInputBox::OnConfirmedThunkDef ScriptGUIInputBox::onConfirmedThunk;

	ScriptGUIInputBox::ScriptGUIInputBox(MonoObject* instance, GUIInputBox* inputBox)
		:TScriptGUIElement(instance, inputBox)
	{

	}

	void ScriptGUIInputBox::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIInputBox::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetText", (void*)&ScriptGUIInputBox::internal_getText);
		metaData.scriptClass->addInternalCall("Internal_SetText", (void*)&ScriptGUIInputBox::internal_setText);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUIInputBox::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnChanged", 1)->getThunk();
		onConfirmedThunk = (OnConfirmedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnConfirmed", 0)->getThunk();
	}

	void ScriptGUIInputBox::internal_createInstance(MonoObject* instance, bool multiline, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for(UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUIInputBox* guiInputBox = GUIInputBox::create(multiline, options, toString(MonoUtil::monoToWString(style)));

		auto nativeInstance = new (bs_alloc<ScriptGUIInputBox>()) ScriptGUIInputBox(instance, guiInputBox);

		guiInputBox->onValueChanged.connect(std::bind(&ScriptGUIInputBox::onChanged, nativeInstance, _1));
	}

	void ScriptGUIInputBox::internal_getText(ScriptGUIInputBox* nativeInstance, MonoString** text)
	{
		GUIInputBox* inputBox = (GUIInputBox*)nativeInstance->getGUIElement();
		*text = MonoUtil::wstringToMono(inputBox->getText());
	}

	void ScriptGUIInputBox::internal_setText(ScriptGUIInputBox* nativeInstance, MonoString* text)
	{
		GUIInputBox* inputBox = (GUIInputBox*)nativeInstance->getGUIElement();
		inputBox->setText(MonoUtil::monoToWString(text));
	}

	void ScriptGUIInputBox::internal_setTint(ScriptGUIInputBox* nativeInstance, Color* color)
	{
		GUIInputBox* inputBox = (GUIInputBox*)nativeInstance->getGUIElement();
		inputBox->setTint(*color);
	}

	void ScriptGUIInputBox::onChanged(const WString& newValue)
	{
		MonoString* monoValue = MonoUtil::wstringToMono(newValue);
		MonoUtil::invokeThunk(onChangedThunk, getManagedInstance(), monoValue);
	}

	void ScriptGUIInputBox::onConfirmed()
	{
		MonoUtil::invokeThunk(onConfirmedThunk, getManagedInstance());
	}
}