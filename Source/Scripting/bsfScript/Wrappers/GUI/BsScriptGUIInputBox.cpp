//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIInputBox.h"
#include "BsScriptMeta.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUIInputBox.h"
#include "GUI/BsGUIOptions.h"

using namespace std::placeholders;

using namespace bs;
ScriptGUIInputBox::OnChangedThunkDef ScriptGUIInputBox::onChangedThunk;
ScriptGUIInputBox::OnConfirmedThunkDef ScriptGUIInputBox::onConfirmedThunk;

ScriptGUIInputBox::ScriptGUIInputBox(MonoObject* instance, GUIInputBox* inputBox)
	: TScriptGUIElement(instance, inputBox)
{
}

void ScriptGUIInputBox::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIInputBox::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_GetText", (void*)&ScriptGUIInputBox::InternalGetText);
	metaData.ScriptClass->AddInternalCall("Internal_SetText", (void*)&ScriptGUIInputBox::InternalSetText);
	metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIInputBox::InternalSetTint);

	onChangedThunk = (OnChangedThunkDef)metaData.ScriptClass->GetMethod("Internal_DoOnChanged", 1)->GetThunk();
	onConfirmedThunk = (OnConfirmedThunkDef)metaData.ScriptClass->GetMethod("Internal_DoOnConfirmed", 0)->GetThunk();
}

void ScriptGUIInputBox::InternalCreateInstance(MonoObject* instance, bool multiline, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIInputBox* guiInputBox = GUIInputBox::Create(multiline, options, MonoUtil::MonoToString(style));

	auto nativeInstance = new(B3DAllocate<ScriptGUIInputBox>()) ScriptGUIInputBox(instance, guiInputBox);

	guiInputBox->OnValueChanged.Connect(std::bind(&::bs::ScriptGUIInputBox::OnChanged, nativeInstance, _1));
}

void ScriptGUIInputBox::InternalGetText(ScriptGUIInputBox* nativeInstance, MonoString** text)
{
	GUIInputBox* inputBox = (GUIInputBox*)nativeInstance->GetGuiElement();
	MonoUtil::ReferenceCopy(text, (MonoObject*)MonoUtil::StringToMono(inputBox->GetText()));
}

void ScriptGUIInputBox::InternalSetText(ScriptGUIInputBox* nativeInstance, MonoString* text)
{
	GUIInputBox* inputBox = (GUIInputBox*)nativeInstance->GetGuiElement();
	inputBox->SetText(MonoUtil::MonoToString(text));
}

void ScriptGUIInputBox::InternalSetTint(ScriptGUIInputBox* nativeInstance, Color* color)
{
	GUIInputBox* inputBox = (GUIInputBox*)nativeInstance->GetGuiElement();
	inputBox->SetTint(*color);
}

void ScriptGUIInputBox::OnChanged(const String& newValue)
{
	MonoString* monoValue = MonoUtil::StringToMono(newValue);
	MonoUtil::InvokeThunk(onChangedThunk, GetManagedInstance(), monoValue);
}

void ScriptGUIInputBox::OnConfirmed()
{
	MonoUtil::InvokeThunk(onConfirmedThunk, GetManagedInstance());
}
