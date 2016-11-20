//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIGameObjectField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsGUIGameObjectField.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIContent.h"
#include "BsScriptGameObject.h"
#include "BsScriptGameObjectManager.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIGameObjectField::OnChangedThunkDef ScriptGUIGameObjectField::onChangedThunk;

	ScriptGUIGameObjectField::ScriptGUIGameObjectField(MonoObject* instance, GUIGameObjectField* GOField)
		:TScriptGUIElement(instance, GOField)
	{

	}

	void ScriptGUIGameObjectField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIGameObjectField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUIGameObjectField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUIGameObjectField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUIGameObjectField::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUIGameObjectField::internal_createInstance(MonoObject* instance, MonoReflectionType* type, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		String styleName = toString(MonoUtil::monoToWString(style));

		::MonoClass* monoClass = MonoUtil::getClass(type);

		String typeNamespace;
		String typeName;
		MonoUtil::getClassName(monoClass, typeNamespace, typeName);

		GUIGameObjectField* guiGameObjectField = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiGameObjectField = GUIGameObjectField::create(typeNamespace, typeName, nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiGameObjectField = GUIGameObjectField::create(typeNamespace, typeName, options, styleName);
		}

		guiGameObjectField->onValueChanged.connect(std::bind(&ScriptGUIGameObjectField::onChanged, instance, _1));

		new (bs_alloc<ScriptGUIGameObjectField>()) ScriptGUIGameObjectField(instance, guiGameObjectField);
	}

	void ScriptGUIGameObjectField::internal_getValue(ScriptGUIGameObjectField* nativeInstance, MonoObject** output)
	{
		GUIGameObjectField* gameObjectField = static_cast<GUIGameObjectField*>(nativeInstance->getGUIElement());

		HGameObject gameObject = gameObjectField->getValue();
		*output = nativeToManagedGO(gameObject);
	}

	void ScriptGUIGameObjectField::internal_setValue(ScriptGUIGameObjectField* nativeInstance, MonoObject* value)
	{
		GUIGameObjectField* gameObjectField = static_cast<GUIGameObjectField*>(nativeInstance->getGUIElement());

		if (value == nullptr)
			gameObjectField->setValue(HGameObject());
		else
		{
			ScriptGameObject* scriptGO = ScriptGameObject::toNative(value);
			gameObjectField->setValue(scriptGO->getNativeHandle());
		}
	}

	void ScriptGUIGameObjectField::internal_setTint(ScriptGUIGameObjectField* nativeInstance, Color* color)
	{
		GUIGameObjectField* gameObjectField = (GUIGameObjectField*)nativeInstance->getGUIElement();
		gameObjectField->setTint(*color);
	}

	void ScriptGUIGameObjectField::onChanged(MonoObject* instance, const HGameObject& newValue)
	{
		MonoObject* managedObj = nativeToManagedGO(newValue);
		MonoUtil::invokeThunk(onChangedThunk, instance, managedObj);
	}

	MonoObject* ScriptGUIGameObjectField::nativeToManagedGO(const HGameObject& instance)
	{
		if (instance == nullptr)
		{
			return nullptr;
		}
		else
		{
			ScriptGameObjectBase* scriptGO = ScriptGameObjectManager::instance().getScriptGameObject(instance->getInstanceId());
			if (scriptGO == nullptr)
				return nullptr;
			else
				return scriptGO->getManagedInstance();
		}
	}
}