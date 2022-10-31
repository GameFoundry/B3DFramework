//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIButton.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIButton.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"
#include "Generated/BsScriptGUIElementStyle.generated.h"

using namespace bs;
ScriptGUIButton::OnClickThunkDef ScriptGUIButton::onClickThunk;
ScriptGUIButton::OnHoverThunkDef ScriptGUIButton::onHoverThunk;
ScriptGUIButton::OnOutThunkDef ScriptGUIButton::onOutThunk;
ScriptGUIButton::OnDoubleClickThunkDef ScriptGUIButton::onDoubleClickThunk;

ScriptGUIButton::ScriptGUIButton(MonoObject* instance, GUIButton* button)
	: TScriptGUIElement(instance, button)
{
}

void ScriptGUIButton::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIButton::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_SetContent", (void*)&ScriptGUIButton::InternalSetContent);
	metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIButton::InternalSetTint);

	onClickThunk = (OnClickThunkDef)metaData.ScriptClass->GetMethod("DoOnClick")->GetThunk();
	onDoubleClickThunk = (OnDoubleClickThunkDef)metaData.ScriptClass->GetMethod("DoOnDoubleClick")->GetThunk();
	onHoverThunk = (OnHoverThunkDef)metaData.ScriptClass->GetMethod("DoOnHover")->GetThunk();
	onOutThunk = (OnOutThunkDef)metaData.ScriptClass->GetMethod("DoOnOut")->GetThunk();
}

void ScriptGUIButton::InternalCreateInstance(MonoObject* instance, __GUIContentInterop* content, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIContent nativeContent = ScriptGUIContent::FromInterop(*content);
	GUIButton* guiButton = GUIButton::Create(nativeContent, options, MonoUtil::MonoToString(style));

	auto nativeInstance = new(B3DAllocate<ScriptGUIButton>()) ScriptGUIButton(instance, guiButton);

	guiButton->OnClick.Connect(std::bind(&::bs::ScriptGUIButton::OnClick, nativeInstance));
	guiButton->OnDoubleClick.Connect(std::bind(&::bs::ScriptGUIButton::OnDoubleClick, nativeInstance));
	guiButton->OnHover.Connect(std::bind(&::bs::ScriptGUIButton::OnHover, nativeInstance));
	guiButton->OnOut.Connect(std::bind(&ScriptGUIButton::OnOut, nativeInstance));
}

void ScriptGUIButton::InternalSetContent(ScriptGUIButton* nativeInstance, __GUIContentInterop* content)
{
	GUIContent nativeContent = ScriptGUIContent::FromInterop(*content);

	GUIButton* button = (GUIButton*)nativeInstance->GetGuiElement();
	button->SetContent(nativeContent);
}

void ScriptGUIButton::InternalSetTint(ScriptGUIButton* nativeInstance, Color* color)
{
	GUIButton* button = (GUIButton*)nativeInstance->GetGuiElement();
	button->SetTint(*color);
}

void ScriptGUIButton::OnClick()
{
	MonoUtil::InvokeThunk(onClickThunk, GetManagedInstance());
}

void ScriptGUIButton::OnDoubleClick()
{
	MonoUtil::InvokeThunk(onDoubleClickThunk, GetManagedInstance());
}

void ScriptGUIButton::OnHover()
{
	MonoUtil::InvokeThunk(onHoverThunk, GetManagedInstance());
}

void ScriptGUIButton::OnOut()
{
	MonoUtil::InvokeThunk(onOutThunk, GetManagedInstance());
}
