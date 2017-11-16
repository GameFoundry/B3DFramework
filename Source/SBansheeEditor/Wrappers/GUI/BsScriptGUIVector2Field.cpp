//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/GUI/BsScriptGUIVector2Field.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "2D/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUIVector2Field.h"
#include "GUI/BsGUIOptions.h"
#include "GUI/BsGUIContent.h"
#include "Wrappers/GUI/BsScriptGUIContent.h"
#include "Wrappers/BsScriptVector.h"

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
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIVector2Field::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", (void*)&ScriptGUIVector2Field::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", (void*)&ScriptGUIVector2Field::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", (void*)&ScriptGUIVector2Field::internal_hasInputFocus);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUIVector2Field::internal_setTint);

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


		auto nativeInstance = new (bs_alloc<ScriptGUIVector2Field>()) ScriptGUIVector2Field(instance, field);

		field->onValueChanged.connect(std::bind(&ScriptGUIVector2Field::onChanged, nativeInstance, _1));
		field->onConfirm.connect(std::bind(&ScriptGUIVector2Field::onConfirmed, nativeInstance));
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

	void ScriptGUIVector2Field::onChanged(const Vector2& newValue)
	{
		MonoUtil::invokeThunk(onChangedThunk, getManagedInstance(), ScriptVector2::box(newValue));
	}

	void ScriptGUIVector2Field::onConfirmed()
	{
		MonoUtil::invokeThunk(onConfirmedThunk, getManagedInstance());
	}
}