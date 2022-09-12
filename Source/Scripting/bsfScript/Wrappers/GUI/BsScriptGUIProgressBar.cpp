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

namespace bs
{
	ScriptGUIProgressBar::ScriptGUIProgressBar(MonoObject* instance, GUIProgressBar* progressBar)
		:TScriptGUIElement(instance, progressBar)
	{

	}

	void ScriptGUIProgressBar::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIProgressBar::internal_createInstance);
		metaData.scriptClass->AddInternalCall("Internal_SetPercent", (void*)&ScriptGUIProgressBar::internal_setPercent);
		metaData.scriptClass->AddInternalCall("Internal_GetPercent", (void*)&ScriptGUIProgressBar::internal_getPercent);
		metaData.scriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIProgressBar::internal_setTint);
	}

	void ScriptGUIProgressBar::internal_createInstance(MonoObject* instance, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray ScriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.Size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.get<GUIOption>(i));

		GUIProgressBar* progressBar = GUIProgressBar::create(options, MonoUtil::monoToString(style));
		new (bs_alloc<ScriptGUIProgressBar>()) ScriptGUIProgressBar(instance, progressBar);
	}

	void ScriptGUIProgressBar::internal_setPercent(ScriptGUIProgressBar* nativeInstance, float percent)
	{
		GUIProgressBar* progressBar = (GUIProgressBar*)nativeInstance->GetGUIElement();
		progressBar->SetPercent(percent);
	}

	float ScriptGUIProgressBar::internal_getPercent(ScriptGUIProgressBar* nativeInstance)
	{
		GUIProgressBar* progressBar = (GUIProgressBar*)nativeInstance->GetGUIElement();
		return progressBar->GetPercent();
	}

	void ScriptGUIProgressBar::internal_setTint(ScriptGUIProgressBar* nativeInstance, Color* color)
	{
		GUIProgressBar* progressBar = (GUIProgressBar*)nativeInstance->GetGUIElement();
		progressBar->SetTint(*color);
	}
}
