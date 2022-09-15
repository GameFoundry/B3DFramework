//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "RTTI/BsManagedSerializableObjectInfoRTTI.h"
#include "Wrappers/GUI/BsScriptRange.h"
#include "Wrappers/GUI/BsScriptStep.h"
#include "BsMonoUtil.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoField.h"
#include "BsMonoProperty.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Wrappers/BsScriptManagedResource.h"
#include "Wrappers/BsScriptRRefBase.h"

namespace bs
{
	RTTITypeBase* ManagedSerializableAssemblyInfo::GetRttiStatic()
	{
		return ManagedSerializableAssemblyInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableAssemblyInfo::GetRtti() const
	{
		return ManagedSerializableAssemblyInfo::GetRttiStatic();
	}

	SPtr<ManagedSerializableMemberInfo> ManagedSerializableObjectInfo::FindMatchingField(const SPtr<ManagedSerializableMemberInfo>& fieldInfo,
		const SPtr<ManagedSerializableTypeInfo>& fieldTypeInfo) const
	{
		const ManagedSerializableObjectInfo* objInfo = this;
		while (objInfo != nullptr)
		{
			if (objInfo->mTypeInfo->matches(fieldTypeInfo))
			{
				auto iterFind = objInfo->mFieldNameToId.find(fieldInfo->mName);
				if (iterFind != objInfo->mFieldNameToId.end())
				{
					auto iterFind2 = objInfo->mFields.find(iterFind->second);
					if (iterFind2 != objInfo->mFields.end())
					{
						SPtr<ManagedSerializableMemberInfo> foundField = iterFind2->second;
						if (foundField->isSerializable())
						{
							if (fieldInfo->mTypeInfo->matches(foundField->mTypeInfo))
								return foundField;
						}
					}
				}

				return nullptr;
			}

			if (objInfo->mBaseClass != nullptr)
				objInfo = objInfo->mBaseClass.get();
			else
				objInfo = nullptr;
		}

		return nullptr;
	}

	RTTITypeBase* ManagedSerializableObjectInfo::GetRttiStatic()
	{
		return ManagedSerializableObjectInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableObjectInfo::GetRtti() const
	{
		return ManagedSerializableObjectInfo::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableMemberInfo::GetRttiStatic()
	{
		return ManagedSerializableMemberInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableMemberInfo::GetRtti() const
	{
		return ManagedSerializableMemberInfo::GetRttiStatic();
	}

	::MonoObject* ManagedSerializableFieldInfo::GetAttribute(MonoClass* monoClass)
	{
		return mMonoField->GetAttribute(monoClass);
	}

	MonoObject* ManagedSerializableFieldInfo::GetValue(MonoObject* instance) const
	{
		return mMonoField->GetBoxed(instance);
	}

	void ManagedSerializableFieldInfo::SetValue(MonoObject* instance, void* value) const
	{
		mMonoField->set(instance, value);
	}

	RTTITypeBase* ManagedSerializableFieldInfo::GetRttiStatic()
	{
		return ManagedSerializableFieldInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldInfo::GetRtti() const
	{
		return ManagedSerializableFieldInfo::GetRttiStatic();
	}

	::MonoObject* ManagedSerializablePropertyInfo::GetAttribute(MonoClass* monoClass)
	{
		return mMonoProperty->GetAttribute(monoClass);
	}

	MonoObject* ManagedSerializablePropertyInfo::GetValue(MonoObject* instance) const
	{
		return mMonoProperty->get(instance);
	}

	void ManagedSerializablePropertyInfo::SetValue(MonoObject* instance, void* value) const
	{
		mMonoProperty->set(instance, value);
	}

	RTTITypeBase* ManagedSerializablePropertyInfo::GetRttiStatic()
	{
		return ManagedSerializablePropertyInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializablePropertyInfo::GetRtti() const
	{
		return ManagedSerializablePropertyInfo::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableTypeInfo::GetRttiStatic()
	{
		return ManagedSerializableTypeInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfo::GetRtti() const
	{
		return ManagedSerializableTypeInfo::GetRttiStatic();
	}

	bool ManagedSerializableTypeInfoPrimitive::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoPrimitive>(typeInfo))
			return false;

		auto primTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);

		return primTypeInfo->mType == mType;
	}

	bool ManagedSerializableTypeInfoPrimitive::IsTypeLoaded() const
	{
		return mType < ScriptPrimitiveType::Count; // Ignoring some removed types
	}

	::MonoClass* ManagedSerializableTypeInfoPrimitive::GetMonoClass() const
	{
		switch(mType)
		{
		case ScriptPrimitiveType::Bool:
			return MonoUtil::getBoolClass();
		case ScriptPrimitiveType::Char:
			return MonoUtil::getCharClass();
		case ScriptPrimitiveType::I8:
			return MonoUtil::getSByteClass();
		case ScriptPrimitiveType::U8:
			return MonoUtil::getByteClass();
		case ScriptPrimitiveType::I16:
			return MonoUtil::getINT16Class();
		case ScriptPrimitiveType::U16:
			return MonoUtil::getUINT16Class();
		case ScriptPrimitiveType::I32:
			return MonoUtil::getINT32Class();
		case ScriptPrimitiveType::U32:
			return MonoUtil::getUINT32Class();
		case ScriptPrimitiveType::I64:
			return MonoUtil::getINT64Class();
		case ScriptPrimitiveType::U64:
			return MonoUtil::getUINT64Class();
		case ScriptPrimitiveType::Float:
			return MonoUtil::getFloatClass();
		case ScriptPrimitiveType::Double:
			return MonoUtil::getDoubleClass();
		case ScriptPrimitiveType::String:
			return MonoUtil::getStringClass();
		default:
			break;
		}

		return nullptr;
	}

	RTTITypeBase* ManagedSerializableTypeInfoPrimitive::GetRttiStatic()
	{
		return ManagedSerializableTypeInfoPrimitiveRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoPrimitive::GetRtti() const
	{
		return ManagedSerializableTypeInfoPrimitive::GetRttiStatic();
	}

	bool ManagedSerializableTypeInfoEnum::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(const auto enumTypeInfo = rtti_cast<ManagedSerializableTypeInfoEnum>(typeInfo.get()))
		{
			return
				enumTypeInfo->mTypeNamespace == mTypeNamespace &&
				enumTypeInfo->mTypeName == mTypeName &&
				enumTypeInfo->mUnderlyingType == mUnderlyingType;
		}

		return false;
	}

	bool ManagedSerializableTypeInfoEnum::IsTypeLoaded() const
	{
		MonoClass* klass = MonoManager::Instance().findClass(mTypeNamespace, mTypeName);
		return klass != nullptr;
	}

	::MonoClass* ManagedSerializableTypeInfoEnum::GetMonoClass() const
	{
		MonoClass* klass = MonoManager::Instance().findClass(mTypeNamespace, mTypeName);

		if(klass)
			return klass->GetInternalClassInternal();

		return nullptr;
	}

	RTTITypeBase* ManagedSerializableTypeInfoEnum::GetRttiStatic()
	{
		return ManagedSerializableTypeInfoEnumRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoEnum::GetRtti() const
	{
		return ManagedSerializableTypeInfoEnum::GetRttiStatic();
	}

	bool ManagedSerializableTypeInfoRef::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if (!rtti_is_of_type<ManagedSerializableTypeInfoRef>(typeInfo))
			return false;

		auto objTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);

		return objTypeInfo->mTypeNamespace == mTypeNamespace && objTypeInfo->mTypeName == mTypeName;
	}

	bool ManagedSerializableTypeInfoRef::IsTypeLoaded() const
	{
		switch (mType)
		{
		case ScriptReferenceType::BuiltinResourceBase:
		case ScriptReferenceType::ManagedResourceBase:
		case ScriptReferenceType::BuiltinResource:
		case ScriptReferenceType::BuiltinComponentBase:
		case ScriptReferenceType::ManagedComponentBase:
		case ScriptReferenceType::BuiltinComponent:
		case ScriptReferenceType::SceneObject:
		case ScriptReferenceType::ReflectableObject:
			return true;
		default:
			break;
		}

		return ScriptAssemblyManager::Instance().hasSerializableObjectInfo(mTypeNamespace, mTypeName);
	}

	::MonoClass* ManagedSerializableTypeInfoRef::GetMonoClass() const
	{
		switch (mType)
		{
		case ScriptReferenceType::BuiltinResourceBase:
			return ScriptResource::getMetaData()->scriptClass->GetInternalClassInternal();
		case ScriptReferenceType::ManagedResourceBase:
			return ScriptManagedResource::getMetaData()->scriptClass->GetInternalClassInternal();
		case ScriptReferenceType::SceneObject:
			return ScriptAssemblyManager::Instance().getBuiltinClasses().sceneObjectClass->GetInternalClassInternal();
		case ScriptReferenceType::BuiltinComponentBase:
			return ScriptAssemblyManager::Instance().getBuiltinClasses().componentClass->GetInternalClassInternal();
		case ScriptReferenceType::ManagedComponentBase:
			return ScriptAssemblyManager::Instance().getBuiltinClasses().managedComponentClass->GetInternalClassInternal();
		default:
			break;
		}

		// Specific component or resource (either builtin or custom)
		SPtr<ManagedSerializableObjectInfo> objInfo;
		if (!ScriptAssemblyManager::Instance().getSerializableObjectInfo(mTypeNamespace, mTypeName, objInfo))
			return nullptr;

		return objInfo->mMonoClass->GetInternalClassInternal();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRef::GetRttiStatic()
	{
		return ManagedSerializableTypeInfoRefRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRef::GetRtti() const
	{
		return ManagedSerializableTypeInfoRef::GetRttiStatic();
	}

	bool ManagedSerializableTypeInfoRRef::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoRRef>(typeInfo))
			return false;

		auto resourceTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRRef>(typeInfo);

		if(mResourceType == nullptr)
			return resourceTypeInfo->mResourceType == nullptr;

		return mResourceType->matches(resourceTypeInfo->mResourceType);
	}

	bool ManagedSerializableTypeInfoRRef::IsTypeLoaded() const
	{
		return mResourceType == nullptr || mResourceType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoRRef::GetMonoClass() const
	{
		// If non-null, this is a templated (i.e. C# generic) RRef type
		if(mResourceType)
		{
			::MonoClass* resourceTypeClass = mResourceType->GetMonoClass();
			if (resourceTypeClass == nullptr)
				return nullptr;

			return ScriptRRefBase::bindGenericParam(resourceTypeClass);
		}
		// RRefBase
		else
			return ScriptAssemblyManager::Instance().getBuiltinClasses().rrefBaseClass->GetInternalClassInternal();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRRef::GetRttiStatic()
	{
		return ManagedSerializableTypeInfoRRefRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRRef::GetRtti() const
	{
		return ManagedSerializableTypeInfoRRef::GetRttiStatic();
	}

	bool ManagedSerializableTypeInfoObject::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoObject>(typeInfo))
			return false;

		auto objTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoObject>(typeInfo);

		return objTypeInfo->mTypeNamespace == mTypeNamespace && objTypeInfo->mTypeName == mTypeName &&
			objTypeInfo->mValueType == mValueType && objTypeInfo->mRTIITypeId == mRTIITypeId;
	}

	bool ManagedSerializableTypeInfoObject::IsTypeLoaded() const
	{
		return ScriptAssemblyManager::Instance().hasSerializableObjectInfo(mTypeNamespace, mTypeName);
	}

	::MonoClass* ManagedSerializableTypeInfoObject::GetMonoClass() const
	{
		SPtr<ManagedSerializableObjectInfo> objInfo;
		if(!ScriptAssemblyManager::Instance().getSerializableObjectInfo(mTypeNamespace, mTypeName, objInfo))
			return nullptr;

		return objInfo->mMonoClass->GetInternalClassInternal();
	}

	RTTITypeBase* ManagedSerializableTypeInfoObject::GetRttiStatic()
	{
		return ManagedSerializableTypeInfoObjectRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoObject::GetRtti() const
	{
		return ManagedSerializableTypeInfoObject::GetRttiStatic();
	}

	bool ManagedSerializableTypeInfoArray::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoArray>(typeInfo))
			return false;

		auto arrayTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoArray>(typeInfo);

		return arrayTypeInfo->mRank == mRank && arrayTypeInfo->mElementType->matches(mElementType);
	}

	bool ManagedSerializableTypeInfoArray::IsTypeLoaded() const
	{
		return mElementType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoArray::GetMonoClass() const
	{
		::MonoClass* elementClass = mElementType->GetMonoClass();
		if(elementClass == nullptr)
			return nullptr;

		return ScriptArray::buildArrayClass(mElementType->GetMonoClass(), mRank);
	}

	RTTITypeBase* ManagedSerializableTypeInfoArray::GetRttiStatic()
	{
		return ManagedSerializableTypeInfoArrayRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoArray::GetRtti() const
	{
		return ManagedSerializableTypeInfoArray::GetRttiStatic();
	}

	bool ManagedSerializableTypeInfoList::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoList>(typeInfo))
			return false;

		auto listTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoList>(typeInfo);

		return listTypeInfo->mElementType->matches(mElementType);
	}

	bool ManagedSerializableTypeInfoList::IsTypeLoaded() const
	{
		return mElementType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoList::GetMonoClass() const
	{
		::MonoClass* elementClass = mElementType->GetMonoClass();
		if(elementClass == nullptr)
			return nullptr;

		MonoClass* genericListClass = ScriptAssemblyManager::Instance().getBuiltinClasses().systemGenericListClass;
		::MonoClass* genParams[1] = { elementClass };

		return MonoUtil::bindGenericParameters(genericListClass->GetInternalClassInternal(), genParams, 1);
	}

	RTTITypeBase* ManagedSerializableTypeInfoList::GetRttiStatic()
	{
		return ManagedSerializableTypeInfoListRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoList::GetRtti() const
	{
		return ManagedSerializableTypeInfoList::GetRttiStatic();
	}

	bool ManagedSerializableTypeInfoDictionary::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoDictionary>(typeInfo))
			return false;

		auto dictTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(typeInfo);

		return dictTypeInfo->mKeyType->matches(mKeyType) && dictTypeInfo->mValueType->matches(mValueType);
	}

	bool ManagedSerializableTypeInfoDictionary::IsTypeLoaded() const
	{
		return mKeyType->isTypeLoaded() && mValueType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoDictionary::GetMonoClass() const
	{
		::MonoClass* keyClass = mKeyType->GetMonoClass();
		::MonoClass* valueClass = mValueType->GetMonoClass();
		if(keyClass == nullptr || valueClass == nullptr)
			return nullptr;

		MonoClass* genericDictionaryClass =
			ScriptAssemblyManager::Instance().getBuiltinClasses().systemGenericDictionaryClass;

		::MonoClass* params[2] = { keyClass, valueClass };
		return MonoUtil::bindGenericParameters(genericDictionaryClass->GetInternalClassInternal(), params, 2);
	}

	RTTITypeBase* ManagedSerializableTypeInfoDictionary::GetRttiStatic()
	{
		return ManagedSerializableTypeInfoDictionaryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoDictionary::GetRtti() const
	{
		return ManagedSerializableTypeInfoDictionary::GetRttiStatic();
	}
}
