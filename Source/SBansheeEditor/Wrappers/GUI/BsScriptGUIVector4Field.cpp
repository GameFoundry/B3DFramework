//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/GUI/BsScriptGUIVector4Field.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUIVector4Field.h"
#include "GUI/BsGUIOptions.h"
#include "GUI/BsGUIContent.h"
#include "Wrappers/GUI/BsScriptGUIContent.h"
#include "Wrappers/BsScriptVector.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIVector4Field::OnChangedThunkDef ScriptGUIVector4Field::onChangedThunk;
	ScriptGUIVector4Field::OnConfirmedThunkDef ScriptGUIVector4Field::onConfirmedThunk;

	ScriptGUIVector4Field::ScriptGUIVector4Field(MonoObject* instance, GUIVector4Field* vector4Field)
		:TScriptGUIElement(instance, vector4Field)
	{

	}

	void ScriptGUIVector4Field::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIVector4Field::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", (void*)&ScriptGUIVector4Field::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", (void*)&ScriptGUIVector4Field::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_HasInputFocus", (void*)&ScriptGUIVector4Field::internal_hasInputFocus);
		metaData.scriptClass->addInternalCall("Internal_SetTint", (void*)&ScriptGUIVector4Field::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnChanged", 1)->getThunk();
		onConfirmedThunk = (OnConfirmedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnConfirmed", 0)->getThunk();
	}

	void ScriptGUIVector4Field::internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUIVector4Field* field = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			field = GUIVector4Field::create(nativeContent, titleWidth, options, styleName);
		}
		else
		{
			field = GUIVector4Field::create(options, styleName);
		}


		auto nativeInstance = new (bs_alloc<ScriptGUIVector4Field>()) ScriptGUIVector4Field(instance, field);

		field->onValueChanged.connect(std::bind(&ScriptGUIVector4Field::onChanged, nativeInstance, _1));
		field->onConfirm.connect(std::bind(&ScriptGUIVector4Field::onConfirmed, nativeInstance));
	}

	void ScriptGUIVector4Field::internal_getValue(ScriptGUIVector4Field* nativeInstance, Vector4* output)
	{
		GUIVector4Field* field = static_cast<GUIVector4Field*>(nativeInstance->getGUIElement());
		*output = field->getValue();
	}

	void ScriptGUIVector4Field::internal_setValue(ScriptGUIVector4Field* nativeInstance, Vector4* value)
	{
		GUIVector4Field* field = static_cast<GUIVector4Field*>(nativeInstance->getGUIElement());
		return field->setValue(*value);
	}

	void ScriptGUIVector4Field::internal_hasInputFocus(ScriptGUIVector4Field* nativeInstance, bool* output)
	{
		GUIVector4Field* field = static_cast<GUIVector4Field*>(nativeInstance->getGUIElement());
		*output = field->hasInputFocus();
	}

	void ScriptGUIVector4Field::internal_setTint(ScriptGUIVector4Field* nativeInstance, Color* color)
	{
		GUIVector4Field* field = (GUIVector4Field*)nativeInstance->getGUIElement();
		field->setTint(*color);
	}

	void ScriptGUIVector4Field::onChanged(const Vector4& newValue)
	{
		MonoUtil::invokeThunk(onChangedThunk, getManagedInstance(), ScriptVector4::box(newValue));
	}

	void ScriptGUIVector4Field::onConfirmed()
	{
		MonoUtil::invokeThunk(onConfirmedThunk, getManagedInstance());
	}
}