//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptSerializableField.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Wrappers/BsScriptSerializableProperty.h"
#include "GUI/BsScriptRange.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "GUI/BsScriptStep.h"
#include "BsScriptCategory.h"
#include "BsScriptOrder.h"

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
		SerializableMemberStyle interopStyle;

		ScriptFieldFlags fieldFlags = fieldInfo->mFlags;
		if (fieldFlags.isSet(ScriptFieldFlag::Range))
		{
			MonoClass* range = ScriptAssemblyManager::instance().getBuiltinClasses().rangeAttribute;
			if (range != nullptr)
			{
				MonoObject* attrib = fieldInfo->getAttribute(range);

				ScriptRange::getMinRangeField()->get(attrib, &style->rangeMin);
				ScriptRange::getMaxRangeField()->get(attrib, &style->rangeMax);
				ScriptRange::getSliderField()->get(attrib, &style->displayAsSlider);
			}
		}

		if (fieldFlags.isSet(ScriptFieldFlag::Step))
		{
			MonoClass* step = ScriptAssemblyManager::instance().getBuiltinClasses().stepAttribute;
			if (step != nullptr)
			{
				MonoObject* attrib = fieldInfo->getAttribute(step);
				ScriptStep::getStepField()->get(attrib, &style->stepIncrement);
			}
		}

		if (fieldFlags.isSet(ScriptFieldFlag::Category))
		{
			MonoClass* category = ScriptAssemblyManager::instance().getBuiltinClasses().categoryAttribute;
			if (category != nullptr)
			{
				MonoObject* attrib = fieldInfo->getAttribute(category);
				ScriptCategory::getNameField()->get(attrib, &style->categoryName);
			}
		}

		if (fieldFlags.isSet(ScriptFieldFlag::Order))
		{
			MonoClass* order = ScriptAssemblyManager::instance().getBuiltinClasses().orderAttribute;
			if (order != nullptr)
			{
				MonoObject* attrib = fieldInfo->getAttribute(order);
				ScriptOrder::getIndexField()->get(attrib, &style->order);
			}
		}

		MonoUtil::valueCopy(&style, &interopStyle, ScriptSerializableFieldStyle::getMetaData()->scriptClass->_getInternalClass());
	}

	ScriptSerializableFieldStyle::ScriptSerializableFieldStyle(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSerializableFieldStyle::initRuntimeData()
	{ }
}
