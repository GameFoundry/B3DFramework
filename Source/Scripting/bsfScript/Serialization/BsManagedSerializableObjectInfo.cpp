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
	RTTITypeBase* ManagedSerializableAssemblyInfo::getRTTIStatic()
	{
		return ManagedSerializableAssemblyInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableAssemblyInfo::getRTTI() const
	{
		return ManagedSerializableAssemblyInfo::GetRTTIStatic();
	}

	SPtr<ManagedSerializableMemberInfo> ManagedSerializableObjectInfo::findMatchingField(const SPtr<ManagedSerializableMemberInfo>& fieldInfo,
		const SPtr<ManagedSerializableTypeInfo>& fieldTypeInfo) const
	{
		const ManagedSerializableObjectInfo* objInfo = this;
		while (objInfo != nullptr)
		{
			if (objInfo->mTypeInfo->Matches(fieldTypeInfo))
			{
				auto iterFind = objInfo->mFieldNameToId.Find(fieldInfo->mName);
				if (iterFind != objInfo->mFieldNameToId.End())
				{
					auto iterFind2 = objInfo->mFields.Find(iterFind->second);
					if (iterFind2 != objInfo->mFields.End())
					{
						SPtr<ManagedSerializableMemberInfo> foundField = iterFind2->second;
						if (foundField->IsSerializable())
						{
							if (fieldInfo->mTypeInfo->Matches(foundField->mTypeInfo))
								return foundField;
						}
					}
				}

				return nullptr;
			}

			if (objInfo->mBaseClass != nullptr)
				objInfo = objInfo->mBaseClass.Get();
			else
				objInfo = nullptr;
		}

		return nullptr;
	}

	RTTITypeBase* ManagedSerializableObjectInfo::getRTTIStatic()
	{
		return ManagedSerializableObjectInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableObjectInfo::getRTTI() const
	{
		return ManagedSerializableObjectInfo::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableMemberInfo::getRTTIStatic()
	{
		return ManagedSerializableMemberInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableMemberInfo::getRTTI() const
	{
		return ManagedSerializableMemberInfo::GetRTTIStatic();
	}

	::MonoObject* ManagedSerializableFieldInfo::getAttribute(MonoClass* monoClass)
	{
		return mMonoField->GetAttribute(monoClass);
	}

	MonoObject* ManagedSerializableFieldInfo::getValue(MonoObject* instance) const
	{
		return mMonoField->GetBoxed(instance);
	}

	void ManagedSerializableFieldInfo::SetValue(MonoObject* instance, void* value) const
	{
		mMonoField->Set(instance, value);
	}

	RTTITypeBase* ManagedSerializableFieldInfo::getRTTIStatic()
	{
		return ManagedSerializableFieldInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldInfo::getRTTI() const
	{
		return ManagedSerializableFieldInfo::GetRTTIStatic();
	}

	::MonoObject* ManagedSerializablePropertyInfo::getAttribute(MonoClass* monoClass)
	{
		return mMonoProperty->GetAttribute(monoClass);
	}

	MonoObject* ManagedSerializablePropertyInfo::getValue(MonoObject* instance) const
	{
		return mMonoProperty->Get(instance);
	}

	void ManagedSerializablePropertyInfo::SetValue(MonoObject* instance, void* value) const
	{
		mMonoProperty->Set(instance, value);
	}

	RTTITypeBase* ManagedSerializablePropertyInfo::getRTTIStatic()
	{
		return ManagedSerializablePropertyInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializablePropertyInfo::getRTTI() const
	{
		return ManagedSerializablePropertyInfo::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableTypeInfo::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfo::getRTTI() const
	{
		return ManagedSerializableTypeInfo::GetRTTIStatic();
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

	::MonoClass* ManagedSerializableTypeInfoPrimitive::getMonoClass() const
	{
		switch(mType)
		{
		case ScriptPrimitiveType::Bool:
			return MonoUtil::GetBoolClass();
		case ScriptPrimitiveType::Char:
			return MonoUtil::GetCharClass();
		case ScriptPrimitiveType::I8:
			return MonoUtil::GetSByteClass();
		case ScriptPrimitiveType::U8:
			return MonoUtil::GetByteClass();
		case ScriptPrimitiveType::I16:
			return MonoUtil::GetINT16Class();
		case ScriptPrimitiveType::U16:
			return MonoUtil::GetUINT16Class();
		case ScriptPrimitiveType::I32:
			return MonoUtil::GetINT32Class();
		case ScriptPrimitiveType::U32:
			return MonoUtil::GetUINT32Class();
		case ScriptPrimitiveType::I64:
			return MonoUtil::GetINT64Class();
		case ScriptPrimitiveType::U64:
			return MonoUtil::GetUINT64Class();
		case ScriptPrimitiveType::Float:
			return MonoUtil::GetFloatClass();
		case ScriptPrimitiveType::Double:
			return MonoUtil::GetDoubleClass();
		case ScriptPrimitiveType::String:
			return MonoUtil::GetStringClass();
		default:
			break;
		}

		return nullptr;
	}

	RTTITypeBase* ManagedSerializableTypeInfoPrimitive::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoPrimitiveRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoPrimitive::getRTTI() const
	{
		return ManagedSerializableTypeInfoPrimitive::GetRTTIStatic();
	}

	bool ManagedSerializableTypeInfoEnum::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(const auto enumTypeInfo = rtti_cast<ManagedSerializableTypeInfoEnum>(typeInfo.Get()))
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
		MonoClass* klass = MonoManager::instance().FindClass(mTypeNamespace, mTypeName);
		return klass != nullptr;
	}

	::MonoClass* ManagedSerializableTypeInfoEnum::getMonoClass() const
	{
		MonoClass* klass = MonoManager::instance().FindClass(mTypeNamespace, mTypeName);

		if(klass)
			return klass->_getInternalClass();

		return nullptr;
	}

	RTTITypeBase* ManagedSerializableTypeInfoEnum::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoEnumRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoEnum::getRTTI() const
	{
		return ManagedSerializableTypeInfoEnum::GetRTTIStatic();
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

		return ScriptAssemblyManager::Instance().HasSerializableObjectInfo(mTypeNamespace, mTypeName);
	}

	::MonoClass* ManagedSerializableTypeInfoRef::getMonoClass() const
	{
		switch (mType)
		{
		case ScriptReferenceType::BuiltinResourceBase:
			return ScriptResource::GetMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::ManagedResourceBase:
			return ScriptManagedResource::GetMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::SceneObject:
			return ScriptAssemblyManager::Instance().GetBuiltinClasses().sceneObjectClass->_getInternalClass();
		case ScriptReferenceType::BuiltinComponentBase:
			return ScriptAssemblyManager::Instance().GetBuiltinClasses().componentClass->_getInternalClass();
		case ScriptReferenceType::ManagedComponentBase:
			return ScriptAssemblyManager::Instance().GetBuiltinClasses().managedComponentClass->_getInternalClass();
		default:
			break;
		}

		// Specific component or resource (either builtin or custom)
		SPtr<ManagedSerializableObjectInfo> objInfo;
		if (!ScriptAssemblyManager::instance().GetSerializableObjectInfo(mTypeNamespace, mTypeName, objInfo))
			return nullptr;

		return objInfo->mMonoClass->_getInternalClass();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRef::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoRefRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRef::getRTTI() const
	{
		return ManagedSerializableTypeInfoRef::GetRTTIStatic();
	}

	bool ManagedSerializableTypeInfoRRef::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoRRef>(typeInfo))
			return false;

		auto resourceTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRRef>(typeInfo);

		if(mResourceType == nullptr)
			return resourceTypeInfo->mResourceType == nullptr;

		return mResourceType->Matches(resourceTypeInfo->mResourceType);
	}

	bool ManagedSerializableTypeInfoRRef::IsTypeLoaded() const
	{
		return mResourceType == nullptr || mResourceType->IsTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoRRef::getMonoClass() const
	{
		// If non-null, this is a templated (i.e. C# generic) RRef type
		if(mResourceType)
		{
			::MonoClass* resourceTypeClass = mResourceType->GetMonoClass();
			if (resourceTypeClass == nullptr)
				return nullptr;

			return ScriptRRefBase::BindGenericParam(resourceTypeClass);
		}
		// RRefBase
		else
			return ScriptAssemblyManager::Instance().GetBuiltinClasses().rrefBaseClass->_getInternalClass();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRRef::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoRRefRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRRef::getRTTI() const
	{
		return ManagedSerializableTypeInfoRRef::GetRTTIStatic();
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
		return ScriptAssemblyManager::Instance().HasSerializableObjectInfo(mTypeNamespace, mTypeName);
	}

	::MonoClass* ManagedSerializableTypeInfoObject::getMonoClass() const
	{
		SPtr<ManagedSerializableObjectInfo> objInfo;
		if(!ScriptAssemblyManager::instance().GetSerializableObjectInfo(mTypeNamespace, mTypeName, objInfo))
			return nullptr;

		return objInfo->mMonoClass->_getInternalClass();
	}

	RTTITypeBase* ManagedSerializableTypeInfoObject::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoObjectRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoObject::getRTTI() const
	{
		return ManagedSerializableTypeInfoObject::GetRTTIStatic();
	}

	bool ManagedSerializableTypeInfoArray::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoArray>(typeInfo))
			return false;

		auto arrayTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoArray>(typeInfo);

		return arrayTypeInfo->mRank == mRank && arrayTypeInfo->mElementType->Matches(mElementType);
	}

	bool ManagedSerializableTypeInfoArray::IsTypeLoaded() const
	{
		return mElementType->IsTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoArray::getMonoClass() const
	{
		::MonoClass* elementClass = mElementType->GetMonoClass();
		if(elementClass == nullptr)
			return nullptr;

		return ScriptArray::BuildArrayClass(mElementType->GetMonoClass(), mRank);
	}

	RTTITypeBase* ManagedSerializableTypeInfoArray::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoArrayRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoArray::getRTTI() const
	{
		return ManagedSerializableTypeInfoArray::GetRTTIStatic();
	}

	bool ManagedSerializableTypeInfoList::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoList>(typeInfo))
			return false;

		auto listTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoList>(typeInfo);

		return listTypeInfo->mElementType->Matches(mElementType);
	}

	bool ManagedSerializableTypeInfoList::IsTypeLoaded() const
	{
		return mElementType->IsTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoList::getMonoClass() const
	{
		::MonoClass* elementClass = mElementType->GetMonoClass();
		if(elementClass == nullptr)
			return nullptr;

		MonoClass* genericListClass = ScriptAssemblyManager::instance().GetBuiltinClasses().systemGenericListClass;
		::MonoClass* genParams[1] = { elementClass };

		return MonoUtil::BindGenericParameters(genericListClass->_getInternalClass(), genParams, 1);
	}

	RTTITypeBase* ManagedSerializableTypeInfoList::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoListRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoList::getRTTI() const
	{
		return ManagedSerializableTypeInfoList::GetRTTIStatic();
	}

	bool ManagedSerializableTypeInfoDictionary::Matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoDictionary>(typeInfo))
			return false;

		auto dictTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(typeInfo);

		return dictTypeInfo->mKeyType->Matches(mKeyType) && dictTypeInfo->mValueType->matches(mValueType);
	}

	bool ManagedSerializableTypeInfoDictionary::IsTypeLoaded() const
	{
		return mKeyType->IsTypeLoaded() && mValueType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoDictionary::getMonoClass() const
	{
		::MonoClass* keyClass = mKeyType->GetMonoClass();
		::MonoClass* valueClass = mValueType->GetMonoClass();
		if(keyClass == nullptr || valueClass == nullptr)
			return nullptr;

		MonoClass* genericDictionaryClass =
			ScriptAssemblyManager::instance().GetBuiltinClasses().systemGenericDictionaryClass;

		::MonoClass* params[2] = { keyClass, valueClass };
		return MonoUtil::BindGenericParameters(genericDictionaryClass->_getInternalClass(), params, 2);
	}

	RTTITypeBase* ManagedSerializableTypeInfoDictionary::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoDictionaryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoDictionary::getRTTI() const
	{
		return ManagedSerializableTypeInfoDictionary::GetRTTIStatic();
	}
}
