//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIVector2Field.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUIVector2Field.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIContent.h"
#include "BsScriptVector.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIVector2Field::OnChangedThunkDef ScriptGUIVector2Field::onChangedThunk;
	ScriptGUIVector2Field::OnConfirmedThunkDef ScriptGUIVector2Field::onConfirmedThunk;

	ScriptGUIVector2Field::ScriptGUIVector2Field(MonoObject* instance, GUIVector2Field* vector2Field)
		:TScriptGUIElement(instance, vector2Field)
	{

	}

	void ScriptGUIVector2Field::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIVector2Field::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUIVector2Field::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUIVector2Field::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", &ScriptGUIVector2Field::internal_hasInputFocus);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUIVector2Field::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnChanged", 1)->getThunk();
		onConfirmedThunk = (OnConfirmedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnConfirmed", 0)->getThunk();
	}

	void ScriptGUIVector2Field::internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUIVector2Field* field = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			field = GUIVector2Field::create(nativeContent, titleWidth, options, styleName);
		}
		else
		{
			field = GUIVector2Field::create(options, styleName);
		}

		field->onValueChanged.connect(std::bind(&ScriptGUIVector2Field::onChanged, instance, _1));
		field->onConfirm.connect(std::bind(&ScriptGUIVector2Field::onConfirmed, instance));

		new (bs_alloc<ScriptGUIVector2Field>()) ScriptGUIVector2Field(instance, field);
	}

	void ScriptGUIVector2Field::internal_getValue(ScriptGUIVector2Field* nativeInstance, Vector2* output)
	{
		GUIVector2Field* field = static_cast<GUIVector2Field*>(nativeInstance->getGUIElement());
		*output = field->getValue();
	}

	void ScriptGUIVector2Field::internal_setValue(ScriptGUIVector2Field* nativeInstance, Vector2* value)
	{
		GUIVector2Field* field = static_cast<GUIVector2Field*>(nativeInstance->getGUIElement());
		return field->setValue(*value);
	}

	void ScriptGUIVector2Field::internal_hasInputFocus(ScriptGUIVector2Field* nativeInstance, bool* output)
	{
		GUIVector2Field* field = static_cast<GUIVector2Field*>(nativeInstance->getGUIElement());
		*output = field->hasInputFocus();
	}

	void ScriptGUIVector2Field::internal_setTint(ScriptGUIVector2Field* nativeInstance, Color* color)
	{
		GUIVector2Field* field = (GUIVector2Field*)nativeInstance->getGUIElement();
		field->setTint(*color);
	}

	void ScriptGUIVector2Field::onChanged(MonoObject* instance, const Vector2& newValue)
	{
		MonoUtil::invokeThunk(onChangedThunk, instance, ScriptVector2::box(newValue));
	}

	void ScriptGUIVector2Field::onConfirmed(MonoObject* instance)
	{
		MonoUtil::invokeThunk(onConfirmedThunk, instance);
	}
}