//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUILabel.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUILabel.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"
#include "Generated/BsScriptGUIElementStyle.generated.h"

using namespace bs;
ScriptGUILabel::ScriptGUILabel(MonoObject* instance, GUILabel* label)
	: TScriptGUIElement(instance, label)
{
}

void ScriptGUILabel::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUILabel::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_SetContent", (void*)&ScriptGUILabel::InternalSetContent);
	metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUILabel::InternalSetTint);
}

void ScriptGUILabel::InternalCreateInstance(MonoObject* instance, __GUIContentInterop* content, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIContent nativeContent = ScriptGUIContent::FromInterop(*content);
	GUILabel* guiLabel = GUILabel::Create(nativeContent, options, MonoUtil::MonoToString(style));

	new(B3DAllocate<ScriptGUILabel>()) ScriptGUILabel(instance, guiLabel);
}

void ScriptGUILabel::InternalSetContent(ScriptGUILabel* nativeInstance, __GUIContentInterop* content)
{
	GUIContent nativeContent = ScriptGUIContent::FromInterop(*content);

	GUILabel* label = (GUILabel*)nativeInstance->GetGuiElement();
	label->SetContent(nativeContent);
}

void ScriptGUILabel::InternalSetTint(ScriptGUILabel* nativeInstance, Color* color)
{
	GUILabel* label = (GUILabel*)nativeInstance->GetGuiElement();
	label->SetTint(*color);
}
