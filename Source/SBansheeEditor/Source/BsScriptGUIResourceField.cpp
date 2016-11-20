//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIResourceField.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUIResourceField.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIContent.h"
#include "BsScriptResource.h"
#include "BsScriptResourceManager.h"
#include "BsScriptResourceRef.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIResourceField::OnChangedThunkDef ScriptGUIResourceField::onChangedThunk;

	ScriptGUIResourceField::ScriptGUIResourceField(MonoObject* instance, GUIResourceField* resourceField)
		:TScriptGUIElement(instance, resourceField)
	{

	}

	void ScriptGUIResourceField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIResourceField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUIResourceField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUIResourceField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_GetValueRef", &ScriptGUIResourceField::internal_getValueRef);
		metaData.scriptClass->addInternalCall("Internal_SetValueRef", &ScriptGUIResourceField::internal_setValueRef);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUIResourceField::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("DoOnChanged", 1)->getThunk();
	}

	void ScriptGUIResourceField::internal_createInstance(MonoObject* instance, MonoReflectionType* type, MonoObject* title, UINT32 titleWidth,
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

		GUIResourceField* guiResourceField = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiResourceField = GUIResourceField::create(typeNamespace, typeName, nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiResourceField = GUIResourceField::create(typeNamespace, typeName, options, styleName);
		}

		guiResourceField->onValueChanged.connect(std::bind(&ScriptGUIResourceField::onChanged, instance, _1));

		new (bs_alloc<ScriptGUIResourceField>()) ScriptGUIResourceField(instance, guiResourceField);
	}

	void ScriptGUIResourceField::internal_getValue(ScriptGUIResourceField* nativeInstance, MonoObject** output)
	{
		GUIResourceField* resourceField = static_cast<GUIResourceField*>(nativeInstance->getGUIElement());

		HResource resource = resourceField->getValue();
		*output = nativeToManagedResource(resource);
	}

	void ScriptGUIResourceField::internal_setValue(ScriptGUIResourceField* nativeInstance, MonoObject* value)
	{
		GUIResourceField* resourceField = static_cast<GUIResourceField*>(nativeInstance->getGUIElement());

		if (value == nullptr)
			resourceField->setValue(HResource());
		else
		{
			ScriptResourceBase* scriptResource = ScriptResource::toNative(value);
			resourceField->setValue(scriptResource->getGenericHandle());
		}
	}

	void ScriptGUIResourceField::internal_getValueRef(ScriptGUIResourceField* nativeInstance, MonoObject** output)
	{
		GUIResourceField* resourceField = static_cast<GUIResourceField*>(nativeInstance->getGUIElement());

		WeakResourceHandle<Resource> resource = resourceField->getValueWeak();
		*output = ScriptResourceRef::create(resource);
	}

	void ScriptGUIResourceField::internal_setValueRef(ScriptGUIResourceField* nativeInstance, MonoObject* value)
	{
		GUIResourceField* resourceField = static_cast<GUIResourceField*>(nativeInstance->getGUIElement());

		if (value == nullptr)
			resourceField->setValue(HTexture());
		else
		{
			ScriptResourceRef* scriptTexture = ScriptResourceRef::toNative(value);
			resourceField->setValueWeak(static_resource_cast<Texture>(scriptTexture->getHandle()));
		}
	}

	void ScriptGUIResourceField::internal_setTint(ScriptGUIResourceField* nativeInstance, Color* color)
	{
		GUIResourceField* resourceField = (GUIResourceField*)nativeInstance->getGUIElement();
		resourceField->setTint(*color);
	}

	void ScriptGUIResourceField::onChanged(MonoObject* instance, const WeakResourceHandle<Resource>& newHandle)
	{
		MonoObject* managedObj = ScriptResourceRef::create(newHandle);
		MonoUtil::invokeThunk(onChangedThunk, instance, managedObj);
	}

	MonoObject* ScriptGUIResourceField::nativeToManagedResource(const HResource& instance)
	{
		if (instance == nullptr)
		{
			return nullptr;
		}
		else
		{
			ScriptResourceBase* scriptResource = ScriptResourceManager::instance().getScriptResource(instance.getUUID());
			if (scriptResource == nullptr)
			{
				ScriptResourceManager::instance().createScriptResource(instance, &scriptResource);

				return scriptResource->getManagedInstance();
			}
			else
				return scriptResource->getManagedInstance();
		}
	}
}