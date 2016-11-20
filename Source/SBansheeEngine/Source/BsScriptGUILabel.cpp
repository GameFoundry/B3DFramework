//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUILabel.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUILayout.h"
#include "BsGUILabel.h"
#include "BsGUIOptions.h"
#include "BsScriptGUIElementStyle.h"
#include "BsScriptGUILayout.h"
#include "BsScriptHString.h"
#include "BsScriptGUIContent.h"

namespace bs
{
	ScriptGUILabel::ScriptGUILabel(MonoObject* instance, GUILabel* label)
		:TScriptGUIElement(instance, label)
	{

	}

	void ScriptGUILabel::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUILabel::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_SetContent", &ScriptGUILabel::internal_setContent);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUILabel::internal_setTint);
	}

	void ScriptGUILabel::internal_createInstance(MonoObject* instance, MonoObject* content, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		GUIContent nativeContent(ScriptGUIContent::getText(content), ScriptGUIContent::getImage(content), ScriptGUIContent::getTooltip(content));
		GUILabel* guiLabel = GUILabel::create(nativeContent, options, toString(MonoUtil::monoToWString(style)));

		new (bs_alloc<ScriptGUILabel>()) ScriptGUILabel(instance, guiLabel);
	}

	void ScriptGUILabel::internal_setContent(ScriptGUILabel* nativeInstance, MonoObject* content)
	{
		GUIContent nativeContent(ScriptGUIContent::getText(content), ScriptGUIContent::getImage(content), ScriptGUIContent::getTooltip(content));

		GUILabel* label = (GUILabel*)nativeInstance->getGUIElement();
		label->setContent(nativeContent);
	}

	void ScriptGUILabel::internal_setTint(ScriptGUILabel* nativeInstance, Color* color)
	{
		GUILabel* label = (GUILabel*)nativeInstance->getGUIElement();
		label->setTint(*color);
	}
}