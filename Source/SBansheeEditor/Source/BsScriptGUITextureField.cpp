//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUITextureField.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsGUITextureField.h"
#include "BsGUIOptions.h"
#include "BsGUIContent.h"
#include "BsScriptGUIContent.h"
#include "BsScriptResourceManager.h"
#include "BsScriptResourceRef.h"

#include "BsScriptTexture.generated.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUITextureField::OnChangedThunkDef ScriptGUITextureField::onChangedThunk;

	ScriptGUITextureField::ScriptGUITextureField(MonoObject* instance, GUITextureField* resourceField)
		:TScriptGUIElement(instance, resourceField)
	{

	}

	void ScriptGUITextureField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUITextureField::internal_createInstance);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptGUITextureField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptGUITextureField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_GetValueRef", &ScriptGUITextureField::internal_getValueRef);
		metaData.scriptClass->addInternalCall("Internal_SetValueRef", &ScriptGUITextureField::internal_setValueRef);
		metaData.scriptClass->addInternalCall("Internal_SetTint", &ScriptGUITextureField::internal_setTint);

		onChangedThunk = (OnChangedThunkDef)metaData.scriptClass->getMethod("Internal_DoOnChanged", 1)->getThunk();
	}

	void ScriptGUITextureField::internal_createInstance(MonoObject* instance, MonoObject* title, UINT32 titleWidth,
		MonoString* style, MonoArray* guiOptions, bool withTitle)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		UINT32 arrayLen = scriptArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			options.addOption(scriptArray.get<GUIOption>(i));

		String styleName = toString(MonoUtil::monoToWString(style));

		GUITextureField* guiTextureField = nullptr;
		if (withTitle)
		{
			GUIContent nativeContent(ScriptGUIContent::getText(title), ScriptGUIContent::getImage(title), ScriptGUIContent::getTooltip(title));
			guiTextureField = GUITextureField::create(nativeContent, titleWidth, options, styleName);
		}
		else
		{
			guiTextureField = GUITextureField::create(options, styleName);
		}

		guiTextureField->onValueChanged.connect(std::bind(&ScriptGUITextureField::onChanged, instance, _1));

		new (bs_alloc<ScriptGUITextureField>()) ScriptGUITextureField(instance, guiTextureField);
	}

	void ScriptGUITextureField::internal_getValue(ScriptGUITextureField* nativeInstance, MonoObject** output)
	{
		GUITextureField* textureField = static_cast<GUITextureField*>(nativeInstance->getGUIElement());

		HTexture resource = textureField->getValue();
		*output = nativeToManagedResource(resource);
	}

	void ScriptGUITextureField::internal_setValue(ScriptGUITextureField* nativeInstance, MonoObject* value)
	{
		GUITextureField* textureField = static_cast<GUITextureField*>(nativeInstance->getGUIElement());

		if (value == nullptr)
			textureField->setValue(HTexture());
		else
		{
			ScriptTexture* scriptTexture = ScriptTexture::toNative(value);
			textureField->setValue(static_resource_cast<Texture>(scriptTexture->getGenericHandle()));
		}
	}

	void ScriptGUITextureField::internal_getValueRef(ScriptGUITextureField* nativeInstance, MonoObject** output)
	{
		GUITextureField* textureField = static_cast<GUITextureField*>(nativeInstance->getGUIElement());

		WeakResourceHandle<Texture> resource = textureField->getValueWeak();
		*output = ScriptResourceRef::create(resource);
	}

	void ScriptGUITextureField::internal_setValueRef(ScriptGUITextureField* nativeInstance, MonoObject* value)
	{
		GUITextureField* textureField = static_cast<GUITextureField*>(nativeInstance->getGUIElement());

		if (value == nullptr)
			textureField->setValue(HTexture());
		else
		{
			ScriptResourceRef* scriptTexture = ScriptResourceRef::toNative(value);
			textureField->setValueWeak(static_resource_cast<Texture>(scriptTexture->getHandle()));
		}
	}

	void ScriptGUITextureField::internal_setTint(ScriptGUITextureField* nativeInstance, Color* color)
	{
		GUITextureField* textureField = (GUITextureField*)nativeInstance->getGUIElement();
		textureField->setTint(*color);
	}

	void ScriptGUITextureField::onChanged(MonoObject* instance, const WeakResourceHandle<Texture>& newHandle)
	{
		MonoObject* managedObj = ScriptResourceRef::create(newHandle);
		MonoUtil::invokeThunk(onChangedThunk, instance, managedObj);
	}

	MonoObject* ScriptGUITextureField::nativeToManagedResource(const HTexture& instance)
	{
		if (instance == nullptr)
			return nullptr;

		ScriptResourceBase* scriptResource = ScriptResourceManager::instance().getScriptResource(instance, true);
		return scriptResource->getManagedInstance();
	}
}