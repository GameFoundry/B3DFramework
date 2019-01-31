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
		return ManagedSerializableAssemblyInfoRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableAssemblyInfo::getRTTI() const
	{
		return ManagedSerializableAssemblyInfo::getRTTIStatic();
	}

	ManagedSerializableObjectInfo::ManagedSerializableObjectInfo()
		:mMonoClass(nullptr)
	{

	}

	SPtr<ManagedSerializableMemberInfo> ManagedSerializableObjectInfo::findMatchingField(const SPtr<ManagedSerializableMemberInfo>& fieldInfo,
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

	RTTITypeBase* ManagedSerializableObjectInfo::getRTTIStatic()
	{
		return ManagedSerializableObjectInfoRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableObjectInfo::getRTTI() const
	{
		return ManagedSerializableObjectInfo::getRTTIStatic();
	}

	ManagedSerializableMemberInfo::ManagedSerializableMemberInfo()
		:mFieldId(0), mFlags(0)
	{

	}

	RTTITypeBase* ManagedSerializableMemberInfo::getRTTIStatic()
	{
		return ManagedSerializableMemberInfoRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableMemberInfo::getRTTI() const
	{
		return ManagedSerializableMemberInfo::getRTTIStatic();
	}

	ManagedSerializableFieldInfo::ManagedSerializableFieldInfo()
		:mMonoField(nullptr)
	{

	}

	::MonoObject* ManagedSerializableFieldInfo::getAttribute(MonoClass* monoClass)
	{
		return mMonoField->getAttribute(monoClass);
	}

	MonoObject* ManagedSerializableFieldInfo::getValue(MonoObject* instance) const
	{
		return mMonoField->getBoxed(instance);
	}

	void ManagedSerializableFieldInfo::setValue(MonoObject* instance, void* value) const
	{
		mMonoField->set(instance, value);
	}

	RTTITypeBase* ManagedSerializableFieldInfo::getRTTIStatic()
	{
		return ManagedSerializableFieldInfoRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableFieldInfo::getRTTI() const
	{
		return ManagedSerializableFieldInfo::getRTTIStatic();
	}

	ManagedSerializablePropertyInfo::ManagedSerializablePropertyInfo()
		:mMonoProperty(nullptr)
	{

	}

	::MonoObject* ManagedSerializablePropertyInfo::getAttribute(MonoClass* monoClass)
	{
		return mMonoProperty->getAttribute(monoClass);
	}

	MonoObject* ManagedSerializablePropertyInfo::getValue(MonoObject* instance) const
	{
		return mMonoProperty->get(instance);
	}

	void ManagedSerializablePropertyInfo::setValue(MonoObject* instance, void* value) const
	{
		mMonoProperty->set(instance, value);
	}

	RTTITypeBase* ManagedSerializablePropertyInfo::getRTTIStatic()
	{
		return ManagedSerializablePropertyInfoRTTI::instance();
	}

	RTTITypeBase* ManagedSerializablePropertyInfo::getRTTI() const
	{
		return ManagedSerializablePropertyInfo::getRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableTypeInfo::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfo::getRTTI() const
	{
		return ManagedSerializableTypeInfo::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoPrimitive::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoPrimitive>(typeInfo))
			return false;

		auto primTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);

		return primTypeInfo->mType == mType;
	}

	bool ManagedSerializableTypeInfoPrimitive::isTypeLoaded() const
	{
		return mType < ScriptPrimitiveType::Count; // Ignoring some removed types
	}

	::MonoClass* ManagedSerializableTypeInfoPrimitive::getMonoClass() const
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

	RTTITypeBase* ManagedSerializableTypeInfoPrimitive::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoPrimitiveRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoPrimitive::getRTTI() const
	{
		return ManagedSerializableTypeInfoPrimitive::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoEnum::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
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

	bool ManagedSerializableTypeInfoEnum::isTypeLoaded() const
	{
		MonoClass* klass = MonoManager::instance().findClass(mTypeNamespace, mTypeName);
		return klass != nullptr;
	}

	::MonoClass* ManagedSerializableTypeInfoEnum::getMonoClass() const
	{
		MonoClass* klass = MonoManager::instance().findClass(mTypeNamespace, mTypeName);

		if(klass)
			return klass->_getInternalClass();

		return nullptr;
	}

	RTTITypeBase* ManagedSerializableTypeInfoEnum::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoEnumRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoEnum::getRTTI() const
	{
		return ManagedSerializableTypeInfoEnum::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoRef::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if (!rtti_is_of_type<ManagedSerializableTypeInfoRef>(typeInfo))
			return false;

		auto objTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);

		return objTypeInfo->mTypeNamespace == mTypeNamespace && objTypeInfo->mTypeName == mTypeName;
	}

	bool ManagedSerializableTypeInfoRef::isTypeLoaded() const
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
			return true;
		default:
			break;
		}

		return ScriptAssemblyManager::instance().hasSerializableObjectInfo(mTypeNamespace, mTypeName);
	}

	::MonoClass* ManagedSerializableTypeInfoRef::getMonoClass() const
	{
		switch (mType)
		{
		case ScriptReferenceType::BuiltinResourceBase:
			return ScriptResource::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::ManagedResourceBase:
			return ScriptManagedResource::getMetaData()->scriptClass->_getInternalClass();
		case ScriptReferenceType::SceneObject:
			return ScriptAssemblyManager::instance().getBuiltinClasses().sceneObjectClass->_getInternalClass();
		case ScriptReferenceType::BuiltinComponentBase:
			return ScriptAssemblyManager::instance().getBuiltinClasses().componentClass->_getInternalClass();
		case ScriptReferenceType::ManagedComponentBase:
			return ScriptAssemblyManager::instance().getBuiltinClasses().managedComponentClass->_getInternalClass();
		default:
			break;
		}

		// Specific component or resource (either builtin or custom)
		SPtr<ManagedSerializableObjectInfo> objInfo;
		if (!ScriptAssemblyManager::instance().getSerializableObjectInfo(mTypeNamespace, mTypeName, objInfo))
			return nullptr;

		return objInfo->mMonoClass->_getInternalClass();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRef::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoRefRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRef::getRTTI() const
	{
		return ManagedSerializableTypeInfoRef::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoRRef::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoRRef>(typeInfo))
			return false;

		auto resourceTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRRef>(typeInfo);

		if(mResourceType == nullptr)
			return resourceTypeInfo->mResourceType == nullptr;

		return mResourceType->matches(resourceTypeInfo->mResourceType);
	}

	bool ManagedSerializableTypeInfoRRef::isTypeLoaded() const
	{
		return mResourceType == nullptr || mResourceType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoRRef::getMonoClass() const
	{
		// If non-null, this is a templated (i.e. C# generic) RRef type
		if(mResourceType)
		{
			::MonoClass* resourceTypeClass = mResourceType->getMonoClass();
			if (resourceTypeClass == nullptr)
				return nullptr;

			return ScriptRRefBase::bindGenericParam(resourceTypeClass);
		}
		// RRefBase
		else
			return ScriptAssemblyManager::instance().getBuiltinClasses().rrefBaseClass->_getInternalClass();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRRef::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoRRefRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoRRef::getRTTI() const
	{
		return ManagedSerializableTypeInfoRRef::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoObject::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoObject>(typeInfo))
			return false;

		auto objTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoObject>(typeInfo);

		return objTypeInfo->mTypeNamespace == mTypeNamespace && objTypeInfo->mTypeName == mTypeName && objTypeInfo->mValueType == mValueType;
	}

	bool ManagedSerializableTypeInfoObject::isTypeLoaded() const
	{
		return ScriptAssemblyManager::instance().hasSerializableObjectInfo(mTypeNamespace, mTypeName);
	}

	::MonoClass* ManagedSerializableTypeInfoObject::getMonoClass() const
	{
		SPtr<ManagedSerializableObjectInfo> objInfo;
		if(!ScriptAssemblyManager::instance().getSerializableObjectInfo(mTypeNamespace, mTypeName, objInfo))
			return nullptr;

		return objInfo->mMonoClass->_getInternalClass();
	}

	RTTITypeBase* ManagedSerializableTypeInfoObject::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoObjectRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoObject::getRTTI() const
	{
		return ManagedSerializableTypeInfoObject::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoArray::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoArray>(typeInfo))
			return false;

		auto arrayTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoArray>(typeInfo);

		return arrayTypeInfo->mRank == mRank && arrayTypeInfo->mElementType->matches(mElementType);
	}

	bool ManagedSerializableTypeInfoArray::isTypeLoaded() const
	{
		return mElementType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoArray::getMonoClass() const
	{
		::MonoClass* elementClass = mElementType->getMonoClass();
		if(elementClass == nullptr)
			return nullptr;

		return ScriptArray::buildArrayClass(mElementType->getMonoClass(), mRank);
	}

	RTTITypeBase* ManagedSerializableTypeInfoArray::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoArrayRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoArray::getRTTI() const
	{
		return ManagedSerializableTypeInfoArray::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoList::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoList>(typeInfo))
			return false;

		auto listTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoList>(typeInfo);

		return listTypeInfo->mElementType->matches(mElementType);
	}

	bool ManagedSerializableTypeInfoList::isTypeLoaded() const
	{
		return mElementType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoList::getMonoClass() const
	{
		::MonoClass* elementClass = mElementType->getMonoClass();
		if(elementClass == nullptr)
			return nullptr;

		MonoClass* genericListClass = ScriptAssemblyManager::instance().getBuiltinClasses().systemGenericListClass;
		::MonoClass* genParams[1] = { elementClass };

		return MonoUtil::bindGenericParameters(genericListClass->_getInternalClass(), genParams, 1);
	}

	RTTITypeBase* ManagedSerializableTypeInfoList::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoListRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoList::getRTTI() const
	{
		return ManagedSerializableTypeInfoList::getRTTIStatic();
	}

	bool ManagedSerializableTypeInfoDictionary::matches(const SPtr<ManagedSerializableTypeInfo>& typeInfo) const
	{
		if(!rtti_is_of_type<ManagedSerializableTypeInfoDictionary>(typeInfo))
			return false;

		auto dictTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(typeInfo);

		return dictTypeInfo->mKeyType->matches(mKeyType) && dictTypeInfo->mValueType->matches(mValueType);
	}

	bool ManagedSerializableTypeInfoDictionary::isTypeLoaded() const
	{
		return mKeyType->isTypeLoaded() && mValueType->isTypeLoaded();
	}

	::MonoClass* ManagedSerializableTypeInfoDictionary::getMonoClass() const
	{
		::MonoClass* keyClass = mKeyType->getMonoClass();
		::MonoClass* valueClass = mValueType->getMonoClass();
		if(keyClass == nullptr || valueClass == nullptr)
			return nullptr;

		MonoClass* genericDictionaryClass = 
			ScriptAssemblyManager::instance().getBuiltinClasses().systemGenericDictionaryClass;

		::MonoClass* params[2] = { keyClass, valueClass };
		return MonoUtil::bindGenericParameters(genericDictionaryClass->_getInternalClass(), params, 2);
	}

	RTTITypeBase* ManagedSerializableTypeInfoDictionary::getRTTIStatic()
	{
		return ManagedSerializableTypeInfoDictionaryRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableTypeInfoDictionary::getRTTI() const
	{
		return ManagedSerializableTypeInfoDictionary::getRTTIStatic();
	}
}
