//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptSerializableField.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Wrappers/BsScriptSerializableProperty.h"

namespace bs
{

	ScriptSerializableField::ScriptSerializableField(MonoObject* instance, const SPtr<ManagedSerializableMemberInfo>& fieldInfo)
		:ScriptObject(instance), mFieldInfo(fieldInfo)
	{

	}

	void ScriptSerializableField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateProperty", (void*)&ScriptSerializableField::internal_createProperty);
		metaData.scriptClass->addInternalCall("Internal_GetValue", (void*)&ScriptSerializableField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", (void*)&ScriptSerializableField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_GetStyle", (void*)&ScriptSerializableField::internal_getStyle);
	}

	MonoObject* ScriptSerializableField::create(MonoObject* parentObject, const SPtr<ManagedSerializableMemberInfo>& fieldInfo)
	{
		MonoString* monoStrName = MonoUtil::wstringToMono(toWString(fieldInfo->mName));
		MonoReflectionType* internalType = MonoUtil::getType(fieldInfo->mTypeInfo->getMonoClass());
		UINT32 fieldFlags = (UINT32)fieldInfo->mFlags;

		void* params[4] = { parentObject, monoStrName, &fieldFlags, internalType };
		MonoObject* managedInstance = metaData.scriptClass->createInstance(params, 4);

		new (bs_alloc<ScriptSerializableField>()) ScriptSerializableField(managedInstance, fieldInfo);
		return managedInstance;
	}

	MonoObject* ScriptSerializableField::internal_createProperty(ScriptSerializableField* nativeInstance)
	{
		return ScriptSerializableProperty::create(nativeInstance->mFieldInfo->mTypeInfo);
	}

	MonoObject* ScriptSerializableField::internal_getValue(ScriptSerializableField* nativeInstance, MonoObject* instance)
	{
		return nativeInstance->mFieldInfo->getValue(instance);
	}

	void ScriptSerializableField::internal_setValue(ScriptSerializableField* nativeInstance, MonoObject* instance, MonoObject* value)
	{
		if (value != nullptr && MonoUtil::isValueType((MonoUtil::getClass(value))))
		{
			void* rawValue = MonoUtil::unbox(value);
			nativeInstance->mFieldInfo->setValue(instance, rawValue);
		}
		else
			nativeInstance->mFieldInfo->setValue(instance, value);
	}
	void ScriptSerializableField::internal_getStyle(ScriptSerializableField* nativeInstance, SerializableMemberStyle* style)
	{
		SPtr<ManagedSerializableMemberInfo> fieldInfo = nativeInstance->mFieldInfo;
		*style = SerializableMemberStyle();

		ScriptFieldFlags fieldFlags = fieldInfo->mFlags;
		style->displayAsSlider = fieldInfo->renderAsSlider();
		style->rangeMin = fieldInfo->getRangeMinimum();
		style->rangeMax = fieldInfo->getRangeMaximum();
		style->stepIncrement = fieldInfo->getStep();
		style->hasStep = fieldFlags.isSet(ScriptFieldFlag::Step);
		style->hasRange = fieldFlags.isSet(ScriptFieldFlag::Range);
	}
}