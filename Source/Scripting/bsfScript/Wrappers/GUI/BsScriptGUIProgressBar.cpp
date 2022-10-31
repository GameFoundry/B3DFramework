//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIProgressBar.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIProgressBar.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"
#include "Generated/BsScriptGUIElementStyle.generated.h"

using namespace std::placeholders;

using namespace bs;
ScriptGUIProgressBar::ScriptGUIProgressBar(MonoObject* instance, GUIProgressBar* progressBar)
	: TScriptGUIElement(instance, progressBar)
{
}

void ScriptGUIProgressBar::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIProgressBar::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_SetPercent", (void*)&ScriptGUIProgressBar::InternalSetPercent);
	metaData.ScriptClass->AddInternalCall("Internal_GetPercent", (void*)&ScriptGUIProgressBar::InternalGetPercent);
	metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIProgressBar::InternalSetTint);
}

void ScriptGUIProgressBar::InternalCreateInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIProgressBar* progressBar = GUIProgressBar::Create(options, MonoUtil::MonoToString(style));
	new(B3DAllocate<ScriptGUIProgressBar>()) ScriptGUIProgressBar(instance, progressBar);
}

void ScriptGUIProgressBar::InternalSetPercent(ScriptGUIProgressBar* nativeInstance, float percent)
{
	GUIProgressBar* progressBar = (GUIProgressBar*)nativeInstance->GetGuiElement();
	progressBar->SetPercent(percent);
}

float ScriptGUIProgressBar::InternalGetPercent(ScriptGUIProgressBar* nativeInstance)
{
	GUIProgressBar* progressBar = (GUIProgressBar*)nativeInstance->GetGuiElement();
	return progressBar->GetPercent();
}

void ScriptGUIProgressBar::InternalSetTint(ScriptGUIProgressBar* nativeInstance, Color* color)
{
	GUIProgressBar* progressBar = (GUIProgressBar*)nativeInstance->GetGuiElement();
	progressBar->SetTint(*color);
}
