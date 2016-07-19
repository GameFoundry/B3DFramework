//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSerializableField.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsManagedSerializableObjectInfo.h"
#include "BsScriptSerializableProperty.h"

namespace BansheeEngine
{

	ScriptSerializableField::ScriptSerializableField(MonoObject* instance, const SPtr<ManagedSerializableFieldInfo>& fieldInfo)
		:ScriptObject(instance), mFieldInfo(fieldInfo)
	{

	}

	void ScriptSerializableField::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateProperty", &ScriptSerializableField::internal_createProperty);
		metaData.scriptClass->addInternalCall("Internal_GetValue", &ScriptSerializableField::internal_getValue);
		metaData.scriptClass->addInternalCall("Internal_SetValue", &ScriptSerializableField::internal_setValue);
		metaData.scriptClass->addInternalCall("Internal_GetRangeMaximum", &ScriptSerializableField::internal_getRangeMaximum);
		metaData.scriptClass->addInternalCall("Internal_GetRangeMinimum", &ScriptSerializableField::internal_getRangeMinimum);
		metaData.scriptClass->addInternalCall("Internal_RenderAsSlider", &ScriptSerializableField::internal_renderAsSlider);
		metaData.scriptClass->addInternalCall("Internal_GetStep", &ScriptSerializableField::internal_getStep);
	}

	ScriptSerializableField* ScriptSerializableField::create(MonoObject* parentObject, const SPtr<ManagedSerializableFieldInfo>& fieldInfo)
	{
		MonoString* monoStrName = MonoUtil::wstringToMono(toWString(fieldInfo->mName));
		MonoReflectionType* internalType = MonoUtil::getType(fieldInfo->mTypeInfo->getMonoClass());
		UINT32 fieldFlags = (UINT32)fieldInfo->mFlags;

		void* params[4] = { parentObject, monoStrName, &fieldFlags, internalType };
		MonoObject* managedInstance = metaData.scriptClass->createInstance(params, 4);

		ScriptSerializableField* nativeInstance = new (bs_alloc<ScriptSerializableField>()) ScriptSerializableField(managedInstance, fieldInfo);

		return nativeInstance;
	}

	MonoObject* ScriptSerializableField::internal_createProperty(ScriptSerializableField* nativeInstance)
	{
		ScriptSerializableProperty* newProperty = ScriptSerializableProperty::create(nativeInstance->mFieldInfo->mTypeInfo);

		return newProperty->getManagedInstance();
	}

	MonoObject* ScriptSerializableField::internal_getValue(ScriptSerializableField* nativeInstance, MonoObject* instance)
	{
		return nativeInstance->mFieldInfo->mMonoField->getValueBoxed(instance);
	}

	void ScriptSerializableField::internal_setValue(ScriptSerializableField* nativeInstance, MonoObject* instance, MonoObject* value)
	{
		if (value != nullptr && MonoUtil::isValueType((MonoUtil::getClass(value))))
		{
			void* rawValue = MonoUtil::unbox(value);
			nativeInstance->mFieldInfo->mMonoField->setValue(instance, rawValue);
		}
		else
			nativeInstance->mFieldInfo->mMonoField->setValue(instance, value);
	}
	float ScriptSerializableField::internal_getRangeMaximum(ScriptSerializableField* nativeInstance) { return nativeInstance->mFieldInfo->getRangeMaximum(); }
	float ScriptSerializableField::internal_getRangeMinimum(ScriptSerializableField* nativeInstance) { return nativeInstance->mFieldInfo->getRangeMinimum(); }
	bool ScriptSerializableField::internal_renderAsSlider(ScriptSerializableField* nativeInstance) { return  nativeInstance->mFieldInfo->renderAsSlider(); }
	float ScriptSerializableField::internal_getStep(ScriptSerializableField* nativeInstance) { return nativeInstance->mFieldInfo->getStep(); }
}