//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DManagedTypeInfo.h"
#include "RTTI/B3DManagedTypeInfoRTTI.h"
#include "Wrappers/GUI/B3DScriptRange.h"
#include "Wrappers/GUI/B3DScriptStep.h"
#include "B3DMonoUtil.h"
#include "B3DMonoClass.h"
#include "B3DMonoManager.h"
#include "B3DMonoField.h"
#include "B3DMonoProperty.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Wrappers/B3DScriptCategory.h"
#include "Wrappers/B3DScriptManagedResource.h"
#include "Wrappers/B3DScriptOrder.h"
#include "Wrappers/B3DScriptRRefBase.h"

using namespace b3d;
RTTIType* ManagedAssemblyInfo::GetRttiStatic()
{
	return ManagedAssemblyInfoRTTI::Instance();
}

RTTIType* ManagedAssemblyInfo::GetRtti() const
{
	return ManagedAssemblyInfo::GetRttiStatic();
}

MonoReflectionType* ManagedTypeInfo::GetReflectionType() const
{
	return MonoUtil::GetType(GetMonoClass());
}

MonoReflectionType* ManagedObjectInfo::GetReflectionType() const
{
	return MonoUtil::GetType(ScriptClass->GetInternalClass());
}

TShared<ManagedMemberInfo> ManagedObjectInfo::FindMatchingField(const TShared<ManagedMemberInfo>& fieldInfo, const TShared<ManagedTypeInfo>& fieldTypeInfo) const
{
	const ManagedObjectInfo* objInfo = this;
	while(objInfo != nullptr)
	{
		if(objInfo->TypeInfo->Matches(fieldTypeInfo))
		{
			if(auto found = objInfo->MemberNameToIndex.find(fieldInfo->Name); found != objInfo->MemberNameToIndex.end())
			{
				if(B3D_ENSURE(found->second < (u32)objInfo->Members.size()))
				{
					TShared<ManagedMemberInfo> foundMember = objInfo->Members[found->second];
					if(foundMember->IsSerializable())
					{
						if(fieldInfo->TypeInfo->Matches(foundMember->TypeInfo))
							return foundMember;
					}
				}
			}

			return nullptr;
		}

		if(objInfo->BaseClass != nullptr)
			objInfo = objInfo->BaseClass.get();
		else
			objInfo = nullptr;
	}

	return nullptr;
}

RTTIType* ManagedObjectInfo::GetRttiStatic()
{
	return ManagedObjectInfoRTTI::Instance();
}

RTTIType* ManagedObjectInfo::GetRtti() const
{
	return ManagedObjectInfo::GetRttiStatic();
}

void ManagedMemberInfo::SetValue(MonoObject* instance, MonoObject* value) const
{
	if(value != nullptr && MonoUtil::IsValueType((MonoUtil::GetClass(value))))
	{
		void* rawValue = MonoUtil::Unbox(value);
		SetUnboxedValue(instance, rawValue);
	}
	else
		SetUnboxedValue(instance, value);
}

RTTIType* ManagedMemberInfo::GetRttiStatic()
{
	return ManagedMemberInfoRTTI::Instance();
}

RTTIType* ManagedMemberInfo::GetRtti() const
{
	return ManagedMemberInfo::GetRttiStatic();
}

::MonoObject* ManagedFieldInfo::GetAttribute(MonoClass* monoClass) const
{
	return ScriptField->GetAttribute(monoClass);
}

MonoObject* ManagedFieldInfo::GetValue(MonoObject* instance) const
{
	return ScriptField->GetBoxed(instance);
}

void ManagedFieldInfo::SetUnboxedValue(MonoObject* instance, void* value) const
{
	ScriptField->Set(instance, value);
}

RTTIType* ManagedFieldInfo::GetRttiStatic()
{
	return ManagedFieldInfoRTTI::Instance();
}

RTTIType* ManagedFieldInfo::GetRtti() const
{
	return ManagedFieldInfo::GetRttiStatic();
}

::MonoObject* ManagedPropertyInfo::GetAttribute(MonoClass* monoClass) const
{
	return ScriptProperty->GetAttribute(monoClass);
}

MonoObject* ManagedPropertyInfo::GetValue(MonoObject* instance) const
{
	return ScriptProperty->Get(instance);
}

void ManagedPropertyInfo::SetUnboxedValue(MonoObject* instance, void* value) const
{
	ScriptProperty->Set(instance, value);
}

RTTIType* ManagedPropertyInfo::GetRttiStatic()
{
	return ManagedPropertyInfoRTTI::Instance();
}

RTTIType* ManagedPropertyInfo::GetRtti() const
{
	return ManagedPropertyInfo::GetRttiStatic();
}

RTTIType* ManagedTypeInfo::GetRttiStatic()
{
	return ManagedTypeInfoRTTI::Instance();
}

RTTIType* ManagedTypeInfo::GetRtti() const
{
	return ManagedTypeInfo::GetRttiStatic();
}

bool ManagedTypeInfoPrimitive::Matches(const TShared<ManagedTypeInfo>& typeInfo) const
{
	if(!B3DRTTIIsOfType<ManagedTypeInfoPrimitive>(typeInfo))
		return false;

	auto primTypeInfo = std::static_pointer_cast<ManagedTypeInfoPrimitive>(typeInfo);

	return primTypeInfo->PrimitiveType == PrimitiveType;
}

bool ManagedTypeInfoPrimitive::IsTypeLoaded() const
{
	return PrimitiveType < ManagedPrimitiveType::Count; // Ignoring some removed types
}

::MonoClass* ManagedTypeInfoPrimitive::GetMonoClass() const
{
	switch(PrimitiveType)
	{
	case ManagedPrimitiveType::Bool:
		return MonoUtil::GetBoolClass();
	case ManagedPrimitiveType::Char:
		return MonoUtil::GetCharClass();
	case ManagedPrimitiveType::I8:
		return MonoUtil::GetSByteClass();
	case ManagedPrimitiveType::U8:
		return MonoUtil::GetByteClass();
	case ManagedPrimitiveType::I16:
		return MonoUtil::GetInt16Class();
	case ManagedPrimitiveType::U16:
		return MonoUtil::GetUint16Class();
	case ManagedPrimitiveType::I32:
		return MonoUtil::GetInt32Class();
	case ManagedPrimitiveType::U32:
		return MonoUtil::GetUint32Class();
	case ManagedPrimitiveType::I64:
		return MonoUtil::GetInt64Class();
	case ManagedPrimitiveType::U64:
		return MonoUtil::GetUint64Class();
	case ManagedPrimitiveType::Float:
		return MonoUtil::GetFloatClass();
	case ManagedPrimitiveType::Double:
		return MonoUtil::GetDoubleClass();
	case ManagedPrimitiveType::String:
		return MonoUtil::GetStringClass();
	default:
		break;
	}

	return nullptr;
}

RTTIType* ManagedTypeInfoPrimitive::GetRttiStatic()
{
	return ManagedTypeInfoPrimitiveRTTI::Instance();
}

RTTIType* ManagedTypeInfoPrimitive::GetRtti() const
{
	return ManagedTypeInfoPrimitive::GetRttiStatic();
}

bool ManagedTypeInfoEnum::Matches(const TShared<ManagedTypeInfo>& typeInfo) const
{
	if(const auto enumTypeInfo = B3DRTTICast<ManagedTypeInfoEnum>(typeInfo.get()))
	{
		return enumTypeInfo->TypeNamespace == TypeNamespace &&
			enumTypeInfo->TypeName == TypeName &&
			enumTypeInfo->UnderlyingType == UnderlyingType;
	}

	return false;
}

bool ManagedTypeInfoEnum::IsTypeLoaded() const
{
	MonoClass* klass = MonoManager::Instance().FindClass(TypeNamespace, TypeName);
	return klass != nullptr;
}

::MonoClass* ManagedTypeInfoEnum::GetMonoClass() const
{
	MonoClass* klass = MonoManager::Instance().FindClass(TypeNamespace, TypeName);

	if(klass)
		return klass->GetInternalClass();

	return nullptr;
}

RTTIType* ManagedTypeInfoEnum::GetRttiStatic()
{
	return ManagedTypeInfoEnumRTTI::Instance();
}

RTTIType* ManagedTypeInfoEnum::GetRtti() const
{
	return ManagedTypeInfoEnum::GetRttiStatic();
}

bool ManagedTypeInfoReference::Matches(const TShared<ManagedTypeInfo>& typeInfo) const
{
	if(!B3DRTTIIsOfType<ManagedTypeInfoReference>(typeInfo))
		return false;

	auto objTypeInfo = std::static_pointer_cast<ManagedTypeInfoReference>(typeInfo);

	return objTypeInfo->TypeNamespace == TypeNamespace && objTypeInfo->TypeName == TypeName;
}

bool ManagedTypeInfoReference::IsTypeLoaded() const
{
	switch(ReferenceType)
	{
	case ManagedReferenceType::BuiltinResourceBase:
	case ManagedReferenceType::ManagedResourceBase:
	case ManagedReferenceType::BuiltinResource:
	case ManagedReferenceType::BuiltinComponentBase:
	case ManagedReferenceType::ManagedComponentBase:
	case ManagedReferenceType::BuiltinComponent:
	case ManagedReferenceType::SceneObject:
	case ManagedReferenceType::ReflectableObject:
		return true;
	default:
		break;
	}

	return ScriptAssemblyManager::Instance().HasSerializableObjectInfo(TypeNamespace, TypeName);
}

::MonoClass* ManagedTypeInfoReference::GetMonoClass() const
{
	switch(ReferenceType)
	{
	case ManagedReferenceType::BuiltinResourceBase:
		return ScriptResource::GetMetaData()->ScriptClass->GetInternalClass();
	case ManagedReferenceType::ManagedResourceBase:
		return ScriptManagedResource::GetMetaData()->ScriptClass->GetInternalClass();
	case ManagedReferenceType::SceneObject:
		return ScriptAssemblyManager::Instance().GetBuiltinClasses().SceneObjectClass->GetInternalClass();
	case ManagedReferenceType::BuiltinComponentBase:
		return ScriptAssemblyManager::Instance().GetBuiltinClasses().ComponentClass->GetInternalClass();
	case ManagedReferenceType::ManagedComponentBase:
		return ScriptAssemblyManager::Instance().GetBuiltinClasses().ManagedComponentClass->GetInternalClass();
	default:
		break;
	}

	// Specific component or resource (either builtin or custom)
	TShared<ManagedObjectInfo> objInfo;
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(TypeNamespace, TypeName, objInfo))
		return nullptr;

	return objInfo->ScriptClass->GetInternalClass();
}

RTTIType* ManagedTypeInfoReference::GetRttiStatic()
{
	return ManagedTypeInfoReferenceRTTI::Instance();
}

RTTIType* ManagedTypeInfoReference::GetRtti() const
{
	return ManagedTypeInfoReference::GetRttiStatic();
}

bool ManagedTypeInfoResourceReference::Matches(const TShared<ManagedTypeInfo>& typeInfo) const
{
	if(!B3DRTTIIsOfType<ManagedTypeInfoResourceReference>(typeInfo))
		return false;

	auto resourceTypeInfo = std::static_pointer_cast<ManagedTypeInfoResourceReference>(typeInfo);

	if(ResourceType == nullptr)
		return resourceTypeInfo->ResourceType == nullptr;

	return ResourceType->Matches(resourceTypeInfo->ResourceType);
}

bool ManagedTypeInfoResourceReference::IsTypeLoaded() const
{
	return ResourceType == nullptr || ResourceType->IsTypeLoaded();
}

::MonoClass* ManagedTypeInfoResourceReference::GetMonoClass() const
{
	// If non-null, this is a templated (i.e. C# generic) RRef type
	if(ResourceType)
	{
		::MonoClass* resourceTypeClass = ResourceType->GetMonoClass();
		if(resourceTypeClass == nullptr)
			return nullptr;

		return ScriptRRefBase::BindGenericParam(resourceTypeClass);
	}
	// RRefBase
	else
		return ScriptAssemblyManager::Instance().GetBuiltinClasses().RrefBaseClass->GetInternalClass();
}

RTTIType* ManagedTypeInfoResourceReference::GetRttiStatic()
{
	return ManagedTypeInfoResourceReferenceRTTI::Instance();
}

RTTIType* ManagedTypeInfoResourceReference::GetRtti() const
{
	return ManagedTypeInfoResourceReference::GetRttiStatic();
}

bool ManagedTypeInfoObject::Matches(const TShared<ManagedTypeInfo>& typeInfo) const
{
	if(!B3DRTTIIsOfType<ManagedTypeInfoObject>(typeInfo))
		return false;

	auto objTypeInfo = std::static_pointer_cast<ManagedTypeInfoObject>(typeInfo);

	return objTypeInfo->TypeNamespace == TypeNamespace && objTypeInfo->TypeName == TypeName &&
		objTypeInfo->IsValueType == IsValueType && objTypeInfo->TypeRTTIId == TypeRTTIId;
}

bool ManagedTypeInfoObject::IsTypeLoaded() const
{
	return ScriptAssemblyManager::Instance().HasSerializableObjectInfo(TypeNamespace, TypeName);
}

::MonoClass* ManagedTypeInfoObject::GetMonoClass() const
{
	TShared<ManagedObjectInfo> objInfo;
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(TypeNamespace, TypeName, objInfo))
		return nullptr;

	return objInfo->ScriptClass->GetInternalClass();
}

RTTIType* ManagedTypeInfoObject::GetRttiStatic()
{
	return ManagedTypeInfoObjectRTTI::Instance();
}

RTTIType* ManagedTypeInfoObject::GetRtti() const
{
	return ManagedTypeInfoObject::GetRttiStatic();
}

bool ManagedTypeInfoArray::Matches(const TShared<ManagedTypeInfo>& typeInfo) const
{
	if(!B3DRTTIIsOfType<ManagedTypeInfoArray>(typeInfo))
		return false;

	auto arrayTypeInfo = std::static_pointer_cast<ManagedTypeInfoArray>(typeInfo);

	return arrayTypeInfo->Rank == Rank && arrayTypeInfo->ElementType->Matches(ElementType);
}

bool ManagedTypeInfoArray::IsTypeLoaded() const
{
	return ElementType->IsTypeLoaded();
}

::MonoClass* ManagedTypeInfoArray::GetMonoClass() const
{
	::MonoClass* elementClass = ElementType->GetMonoClass();
	if(elementClass == nullptr)
		return nullptr;

	return ScriptArray::BuildArrayClass(ElementType->GetMonoClass(), Rank);
}

RTTIType* ManagedTypeInfoArray::GetRttiStatic()
{
	return ManagedTypeInfoArrayRTTI::Instance();
}

RTTIType* ManagedTypeInfoArray::GetRtti() const
{
	return ManagedTypeInfoArray::GetRttiStatic();
}

bool ManagedTypeInfoList::Matches(const TShared<ManagedTypeInfo>& typeInfo) const
{
	if(!B3DRTTIIsOfType<ManagedTypeInfoList>(typeInfo))
		return false;

	auto listTypeInfo = std::static_pointer_cast<ManagedTypeInfoList>(typeInfo);

	return listTypeInfo->ElementType->Matches(ElementType);
}

bool ManagedTypeInfoList::IsTypeLoaded() const
{
	return ElementType->IsTypeLoaded();
}

::MonoClass* ManagedTypeInfoList::GetMonoClass() const
{
	::MonoClass* elementClass = ElementType->GetMonoClass();
	if(elementClass == nullptr)
		return nullptr;

	MonoClass* genericListClass = ScriptAssemblyManager::Instance().GetBuiltinClasses().SystemGenericListClass;
	::MonoClass* genParams[1] = { elementClass };

	return MonoUtil::BindGenericParameters(genericListClass->GetInternalClass(), genParams, 1);
}

RTTIType* ManagedTypeInfoList::GetRttiStatic()
{
	return ManagedTypeInfoListRTTI::Instance();
}

RTTIType* ManagedTypeInfoList::GetRtti() const
{
	return ManagedTypeInfoList::GetRttiStatic();
}

bool ManagedTypeInfoDictionary::Matches(const TShared<ManagedTypeInfo>& typeInfo) const
{
	if(!B3DRTTIIsOfType<ManagedTypeInfoDictionary>(typeInfo))
		return false;

	auto dictTypeInfo = std::static_pointer_cast<ManagedTypeInfoDictionary>(typeInfo);

	return dictTypeInfo->KeyType->Matches(KeyType) && dictTypeInfo->ValueType->Matches(ValueType);
}

bool ManagedTypeInfoDictionary::IsTypeLoaded() const
{
	return KeyType->IsTypeLoaded() && ValueType->IsTypeLoaded();
}

::MonoClass* ManagedTypeInfoDictionary::GetMonoClass() const
{
	::MonoClass* keyClass = KeyType->GetMonoClass();
	::MonoClass* valueClass = ValueType->GetMonoClass();
	if(keyClass == nullptr || valueClass == nullptr)
		return nullptr;

	MonoClass* genericDictionaryClass =
		ScriptAssemblyManager::Instance().GetBuiltinClasses().SystemGenericDictionaryClass;

	::MonoClass* params[2] = { keyClass, valueClass };
	return MonoUtil::BindGenericParameters(genericDictionaryClass->GetInternalClass(), params, 2);
}

RTTIType* ManagedTypeInfoDictionary::GetRttiStatic()
{
	return ManagedTypeInfoDictionaryRTTI::Instance();
}

RTTIType* ManagedTypeInfoDictionary::GetRtti() const
{
	return ManagedTypeInfoDictionary::GetRttiStatic();
}

ManagedMemberStyle ManagedMemberInfo::ParseStyle() const
{
	ManagedMemberStyle memberStyle;

	if(MetaDataFlags.IsSet(ManagedFieldMetaDataFlag::Range))
	{
		MonoClass* const rangeAttributeType = ScriptAssemblyManager::Instance().GetBuiltinClasses().RangeAttribute;
		if(rangeAttributeType != nullptr)
		{
			MonoObject* const rangeAttribute = GetAttribute(rangeAttributeType);

			ScriptRange::GetMinRangeField()->Get(rangeAttribute, &memberStyle.RangeMin);
			ScriptRange::GetMaxRangeField()->Get(rangeAttribute, &memberStyle.RangeMax);
			ScriptRange::GetSliderField()->Get(rangeAttribute, &memberStyle.DisplayAsSlider);
		}
	}

	if(MetaDataFlags.IsSet(ManagedFieldMetaDataFlag::Step))
	{
		MonoClass* const stepAttributeType = ScriptAssemblyManager::Instance().GetBuiltinClasses().StepAttribute;
		if(stepAttributeType != nullptr)
		{
			MonoObject* const stepAttribute = GetAttribute(stepAttributeType);
			ScriptStep::GetStepField()->Get(stepAttribute, &memberStyle.StepIncrement);
		}
	}

	if(MetaDataFlags.IsSet(ManagedFieldMetaDataFlag::Category))
	{
		MonoClass* const categoryAttributeType = ScriptAssemblyManager::Instance().GetBuiltinClasses().CategoryAttribute;
		if(categoryAttributeType != nullptr)
		{
			MonoObject* const categoryAttribute = GetAttribute(categoryAttributeType);
			ScriptCategory::GetNameField()->Get(categoryAttribute, &memberStyle.CategoryName);
		}
	}

	if(MetaDataFlags.IsSet(ManagedFieldMetaDataFlag::Order))
	{
		MonoClass* const orderAttributeType = ScriptAssemblyManager::Instance().GetBuiltinClasses().OrderAttribute;
		if(orderAttributeType != nullptr)
		{
			MonoObject* const orderAttribute = GetAttribute(orderAttributeType);
			ScriptOrder::GetIndexField()->Get(orderAttribute, &memberStyle.Order);
		}
	}

	return memberStyle;
}

