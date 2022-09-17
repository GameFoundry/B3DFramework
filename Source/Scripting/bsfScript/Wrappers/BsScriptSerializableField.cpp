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
		metaData.scriptClass->AddInternalCall("Internal_CreateProperty", (void*)&ScriptSerializableField::InternalCreateProperty);
		metaData.scriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptSerializableField::InternalGetValue);
		metaData.scriptClass->AddInternalCall("Internal_SetValue", (void*)&ScriptSerializableField::InternalSetValue);
		metaData.scriptClass->AddInternalCall("Internal_GetStyle", (void*)&ScriptSerializableField::InternalGetStyle);
	}

	MonoObject* ScriptSerializableField::Create(MonoObject* parentObject, const SPtr<ManagedSerializableMemberInfo>& fieldInfo)
	{
		MonoString* monoStrName = MonoUtil::WstringToMono(toWString(fieldInfo->mName));
		MonoReflectionType* internalType = MonoUtil::getType(fieldInfo->mTypeInfo->GetMonoClass());
		UINT32 fieldFlags = (UINT32)fieldInfo->mFlags;

		void* params[4] = { parentObject, monoStrName, &fieldFlags, internalType };
		MonoObject* managedInstance = metaData.scriptClass->CreateInstance(params, 4);

		new (bs_alloc<ScriptSerializableField>()) ScriptSerializableField(managedInstance, fieldInfo);
		return managedInstance;
	}

	MonoObject* ScriptSerializableField::InternalCreateProperty(ScriptSerializableField* nativeInstance)
	{
		return ScriptSerializableProperty::Create(nativeInstance->mFieldInfo->mTypeInfo);
	}

	MonoObject* ScriptSerializableField::InternalGetValue(ScriptSerializableField* nativeInstance, MonoObject* instance)
	{
		return nativeInstance->mFieldInfo->GetValue(instance);
	}

	void ScriptSerializableField::InternalSetValue(ScriptSerializableField* nativeInstance, MonoObject* instance, MonoObject* value)
	{
		if (value != nullptr && MonoUtil::IsValueType((MonoUtil::GetClass(value))))
		{
			void* rawValue = MonoUtil::Unbox(value);
			nativeInstance->mFieldInfo->SetValue(instance, rawValue);
		}
		else
			nativeInstance->mFieldInfo->SetValue(instance, value);
	}

	void ScriptSerializableField::InternalGetStyle(ScriptSerializableField* nativeInstance, SerializableMemberStyle* style)
	{
		SPtr<ManagedSerializableMemberInfo> fieldInfo = nativeInstance->mFieldInfo;
		SerializableMemberStyle interopStyle;

		ScriptFieldFlags fieldFlags = fieldInfo->mFlags;
		if (fieldFlags.IsSet(ScriptFieldFlag::Range))
		{
			MonoClass* range = ScriptAssemblyManager::Instance().GetBuiltinClasses().rangeAttribute;
			if (range != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(range);

				ScriptRange::getMinRangeField()->get(attrib, &interopStyle.rangeMin);
				ScriptRange::getMaxRangeField()->get(attrib, &interopStyle.rangeMax);
				ScriptRange::getSliderField()->get(attrib, &interopStyle.displayAsSlider);
			}
		}

		if (fieldFlags.IsSet(ScriptFieldFlag::Step))
		{
			MonoClass* step = ScriptAssemblyManager::Instance().getBuiltinClasses().stepAttribute;
			if (step != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(step);
				ScriptStep::getStepField()->get(attrib, &interopStyle.stepIncrement);
			}
		}

		if (fieldFlags.IsSet(ScriptFieldFlag::Category))
		{
			MonoClass* category = ScriptAssemblyManager::Instance().getBuiltinClasses().categoryAttribute;
			if (category != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(category);
				ScriptCategory::GetNameField()->get(attrib, &interopStyle.categoryName);
			}
		}

		if (fieldFlags.IsSet(ScriptFieldFlag::Order))
		{
			MonoClass* order = ScriptAssemblyManager::Instance().getBuiltinClasses().orderAttribute;
			if (order != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(order);
				ScriptOrder::getIndexField()->get(attrib, &interopStyle.order);
			}
		}

		MonoUtil::valueCopy(style, &interopStyle, ScriptSerializableFieldStyle::getMetaData()->scriptClass->GetInternalClassInternal());
	}

	ScriptSerializableFieldStyle::ScriptSerializableFieldStyle(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSerializableFieldStyle::initRuntimeData()
	{ }
}
