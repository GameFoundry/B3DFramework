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

	void ScriptSerializableField::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateProperty", (void*)&ScriptSerializableField::internal_createProperty);
		metaData.scriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptSerializableField::internal_getValue);
		metaData.scriptClass->AddInternalCall("Internal_SetValue", (void*)&ScriptSerializableField::internal_setValue);
		metaData.scriptClass->AddInternalCall("Internal_GetStyle", (void*)&ScriptSerializableField::internal_getStyle);
	}

	MonoObject* ScriptSerializableField::create(MonoObject* parentObject, const SPtr<ManagedSerializableMemberInfo>& fieldInfo)
	{
		MonoString* monoStrName = MonoUtil::wstringToMono(toWString(fieldInfo->mName));
		MonoReflectionType* internalType = MonoUtil::getType(fieldInfo->mTypeInfo->GetMonoClass());
		UINT32 fieldFlags = (UINT32)fieldInfo->mFlags;

		void* params[4] = { parentObject, monoStrName, &fieldFlags, internalType };
		MonoObject* managedInstance = metaData.scriptClass->CreateInstance(params, 4);

		new (bs_alloc<ScriptSerializableField>()) ScriptSerializableField(managedInstance, fieldInfo);
		return managedInstance;
	}

	MonoObject* ScriptSerializableField::internal_createProperty(ScriptSerializableField* nativeInstance)
	{
		return ScriptSerializableProperty::Create(nativeInstance->mFieldInfo->mTypeInfo);
	}

	MonoObject* ScriptSerializableField::internal_getValue(ScriptSerializableField* nativeInstance, MonoObject* instance)
	{
		return nativeInstance->mFieldInfo->GetValue(instance);
	}

	void ScriptSerializableField::internal_setValue(ScriptSerializableField* nativeInstance, MonoObject* instance, MonoObject* value)
	{
		if (value != nullptr && MonoUtil::isValueType((MonoUtil::getClass(value))))
		{
			void* rawValue = MonoUtil::unbox(value);
			nativeInstance->mFieldInfo->SetValue(instance, rawValue);
		}
		else
			nativeInstance->mFieldInfo->SetValue(instance, value);
	}

	void ScriptSerializableField::internal_getStyle(ScriptSerializableField* nativeInstance, SerializableMemberStyle* style)
	{
		SPtr<ManagedSerializableMemberInfo> fieldInfo = nativeInstance->mFieldInfo;
		SerializableMemberStyle interopStyle;

		ScriptFieldFlags fieldFlags = fieldInfo->mFlags;
		if (fieldFlags.IsSet(ScriptFieldFlag::Range))
		{
			MonoClass* range = ScriptAssemblyManager::instance().GetBuiltinClasses().rangeAttribute;
			if (range != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(range);

				ScriptRange::getMinRangeField()->Get(attrib, &interopStyle.rangeMin);
				ScriptRange::getMaxRangeField()->Get(attrib, &interopStyle.rangeMax);
				ScriptRange::getSliderField()->Get(attrib, &interopStyle.displayAsSlider);
			}
		}

		if (fieldFlags.IsSet(ScriptFieldFlag::Step))
		{
			MonoClass* step = ScriptAssemblyManager::instance().GetBuiltinClasses().stepAttribute;
			if (step != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(step);
				ScriptStep::getStepField()->Get(attrib, &interopStyle.stepIncrement);
			}
		}

		if (fieldFlags.IsSet(ScriptFieldFlag::Category))
		{
			MonoClass* category = ScriptAssemblyManager::instance().GetBuiltinClasses().categoryAttribute;
			if (category != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(category);
				ScriptCategory::getNameField()->Get(attrib, &interopStyle.categoryName);
			}
		}

		if (fieldFlags.IsSet(ScriptFieldFlag::Order))
		{
			MonoClass* order = ScriptAssemblyManager::instance().GetBuiltinClasses().orderAttribute;
			if (order != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(order);
				ScriptOrder::getIndexField()->Get(attrib, &interopStyle.order);
			}
		}

		MonoUtil::valueCopy(style, &interopStyle, ScriptSerializableFieldStyle::getMetaData()->scriptClass->_getInternalClass());
	}

	ScriptSerializableFieldStyle::ScriptSerializableFieldStyle(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSerializableFieldStyle::InitRuntimeData()
	{ }
}
