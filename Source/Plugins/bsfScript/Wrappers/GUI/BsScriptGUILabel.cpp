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
#include "Wrappers/GUI/BsScriptGUIElementStyle.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"

namespace bs
{
	ScriptGUILabel::ScriptGUILabel(MonoObject* instance, GUILabel* label)
		:TScriptGUIElement(instance, label)
	{

	}

	void ScriptGUILabel::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUILabel::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetContent", (void*)&ScriptGUILabel::internal_setContent);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUILabel::internal_setTint);
	}

	void ScriptGUILabel::internal_createInstance(MonoObject* instance, __GUIContentInterop* content, MonoString* style, 
		MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUIContent nativeContent = ScriptGUIContent::fromInterop(*content);
		GUILabel* guiLabel = GUILabel::create(nativeContent, options, MonoUtil::monoToString(style));

		new (bs_alloc<ScriptGUILabel>()) ScriptGUILabel(instance, guiLabel);
	}

	void ScriptGUILabel::internal_setContent(ScriptGUILabel* nativeInstance, __GUIContentInterop* content)
	{
		GUIContent nativeContent = ScriptGUIContent::fromInterop(*content);

		GUILabel* label = (GUILabel*)nativeInstance->getGUIElement();
		label->setContent(nativeContent);
	}

	void ScriptGUILabel::internal_setTint(ScriptGUILabel* nativeInstance, Color* color)
	{
		GUILabel* label = (GUILabel*)nativeInstance->getGUIElement();
		label->setTint(*color);
	}
}