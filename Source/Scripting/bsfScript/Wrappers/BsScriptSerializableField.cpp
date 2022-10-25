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
		: ScriptObject(instance), mFieldInfo(fieldInfo)
	{
	}

	void ScriptSerializableField::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateProperty", (void*)&ScriptSerializableField::InternalCreateProperty);
		metaData.ScriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptSerializableField::InternalGetValue);
		metaData.ScriptClass->AddInternalCall("Internal_SetValue", (void*)&ScriptSerializableField::InternalSetValue);
		metaData.ScriptClass->AddInternalCall("Internal_GetStyle", (void*)&ScriptSerializableField::InternalGetStyle);
	}

	MonoObject* ScriptSerializableField::Create(MonoObject* parentObject, const SPtr<ManagedSerializableMemberInfo>& fieldInfo)
	{
		MonoString* monoStrName = MonoUtil::WstringToMono(toWString(fieldInfo->MName));
		MonoReflectionType* internalType = MonoUtil::GetType(fieldInfo->MTypeInfo->GetMonoClass());
		u32 fieldFlags = (u32)fieldInfo->MFlags;

		void* params[4] = { parentObject, monoStrName, &fieldFlags, internalType };
		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance(params, 4);

		new(bs_alloc<ScriptSerializableField>()) ScriptSerializableField(managedInstance, fieldInfo);
		return managedInstance;
	}

	MonoObject* ScriptSerializableField::InternalCreateProperty(ScriptSerializableField* nativeInstance)
	{
		return ScriptSerializableProperty::Create(nativeInstance->mFieldInfo->MTypeInfo);
	}

	MonoObject* ScriptSerializableField::InternalGetValue(ScriptSerializableField* nativeInstance, MonoObject* instance)
	{
		return nativeInstance->mFieldInfo->GetValue(instance);
	}

	void ScriptSerializableField::InternalSetValue(ScriptSerializableField* nativeInstance, MonoObject* instance, MonoObject* value)
	{
		if(value != nullptr && MonoUtil::IsValueType((MonoUtil::GetClass(value))))
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

		ScriptFieldFlags fieldFlags = fieldInfo->MFlags;
		if(fieldFlags.IsSet(ScriptFieldFlag::Range))
		{
			MonoClass* range = ScriptAssemblyManager::Instance().GetBuiltinClasses().RangeAttribute;
			if(range != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(range);

				ScriptRange::GetMinRangeField()->Get(attrib, &interopStyle.RangeMin);
				ScriptRange::GetMaxRangeField()->Get(attrib, &interopStyle.RangeMax);
				ScriptRange::GetSliderField()->Get(attrib, &interopStyle.DisplayAsSlider);
			}
		}

		if(fieldFlags.IsSet(ScriptFieldFlag::Step))
		{
			MonoClass* step = ScriptAssemblyManager::Instance().GetBuiltinClasses().StepAttribute;
			if(step != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(step);
				ScriptStep::GetStepField()->Get(attrib, &interopStyle.StepIncrement);
			}
		}

		if(fieldFlags.IsSet(ScriptFieldFlag::Category))
		{
			MonoClass* category = ScriptAssemblyManager::Instance().GetBuiltinClasses().CategoryAttribute;
			if(category != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(category);
				ScriptCategory::GetNameField()->Get(attrib, &interopStyle.CategoryName);
			}
		}

		if(fieldFlags.IsSet(ScriptFieldFlag::Order))
		{
			MonoClass* order = ScriptAssemblyManager::Instance().GetBuiltinClasses().OrderAttribute;
			if(order != nullptr)
			{
				MonoObject* attrib = fieldInfo->GetAttribute(order);
				ScriptOrder::GetIndexField()->Get(attrib, &interopStyle.Order);
			}
		}

		MonoUtil::ValueCopy(style, &interopStyle, ScriptSerializableFieldStyle::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}

	ScriptSerializableFieldStyle::ScriptSerializableFieldStyle(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptSerializableFieldStyle::InitRuntimeData()
	{}
} // namespace bs
