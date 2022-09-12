//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsManagedSerializableField.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "RTTI/BsManagedSerializableFieldRTTI.h"
#include "BsMonoUtil.h"
#include "BsMonoManager.h"
#include "BsScriptResourceManager.h"
#include "BsScriptGameObjectManager.h"
#include "Wrappers/BsScriptManagedResource.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "Wrappers/BsScriptComponent.h"
#include "Wrappers/BsScriptManagedComponent.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Serialization/BsManagedSerializableArray.h"
#include "Serialization/BsManagedSerializableList.h"
#include "Serialization/BsManagedSerializableDictionary.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Wrappers/BsScriptReflectable.h"

namespace bs
{
	template<class T>
	bool CompareFieldData(const T* a, const SPtr<ManagedSerializableFieldData>& b)
	{
		if (rtti_is_of_type<T>(b))
		{
			auto castObj = std::static_pointer_cast<T>(b);
			return a->value == castObj->value;
		}

		return false;
	}

	bool CompareFieldData(const SPtr<ManagedSerializableFieldData>& oldData, const SPtr<ManagedSerializableFieldData>& newData)
	{
		if (!oldData)
			return !newData;
		else
		{
			if (!newData)
				return false;
		}

		return oldData->Equals(newData);
	}

	bool IsPrimitiveOrEnumType(const SPtr<ManagedSerializableTypeInfo>& typeInfo, ScriptPrimitiveType underlyingType)
	{
		if(const auto primitiveTypeInfo = rtti_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo.Get()))
			return primitiveTypeInfo->mType == underlyingType;
		else if(const auto enumTypeInfo = rtti_cast<ManagedSerializableTypeInfoEnum>(typeInfo.Get()))
			return enumTypeInfo->mUnderlyingType == underlyingType;

		return false;
	}

	ManagedSerializableFieldKey::ManagedSerializableFieldKey(UINT16 typeId, UINT16 fieldId)
		:mTypeId(typeId), mFieldId(fieldId)
	{ }

	SPtr<ManagedSerializableFieldKey> ManagedSerializableFieldKey::Create(UINT16 typeId, UINT16 fieldId)
	{
		SPtr<ManagedSerializableFieldKey> fieldKey = bs_shared_ptr_new<ManagedSerializableFieldKey>(typeId, fieldId);
		return fieldKey;
	}

	SPtr<ManagedSerializableFieldDataEntry> ManagedSerializableFieldDataEntry::Create(const SPtr<ManagedSerializableFieldKey>& key, const SPtr<ManagedSerializableFieldData>& value)
	{
		SPtr<ManagedSerializableFieldDataEntry> fieldDataEntry = bs_shared_ptr_new<ManagedSerializableFieldDataEntry>();
		fieldDataEntry->mKey = key;
		fieldDataEntry->mValue = value;

		return fieldDataEntry;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableFieldData::Create(const SPtr<ManagedSerializableTypeInfo>& typeInfo, MonoObject* value)
	{
		return Create(typeInfo, value, true);
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableFieldData::CreateDefault(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return Create(typeInfo, nullptr, false);
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableFieldData::Create(const SPtr<ManagedSerializableTypeInfo>& typeInfo, MonoObject* value, bool allowNull)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive || typeInfo->getTypeId() == TID_SerializableTypeInfoEnum)
		{
			ScriptPrimitiveType primitiveType = ScriptPrimitiveType::I32;

			if(auto primitiveTypeInfo = rtti_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo.Get()))
				primitiveType = primitiveTypeInfo->mType;
			else if(auto enumTypeInfo = rtti_cast<ManagedSerializableTypeInfoEnum>(typeInfo.Get()))
				primitiveType = enumTypeInfo->mUnderlyingType;

			switch (primitiveType)
			{
			case ScriptPrimitiveType::Bool:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataBool>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::Char:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataChar>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::I8:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataI8>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::U8:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataU8>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::I16:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataI16>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::U16:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataU16>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::I32:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataI32>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::U32:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataU32>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::I64:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataI64>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::U64:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataU64>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::Float:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataFloat>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::Double:
				{
					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataDouble>();
					if(value != nullptr)
						memcpy(&fieldData->value, MonoUtil::unbox(value), sizeof(fieldData->value));

					return fieldData;
				}
			case ScriptPrimitiveType::String:
				{
					MonoString* strVal = (MonoString*)(value);

					auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataString>();
					if (strVal != nullptr)
						fieldData->value = MonoUtil::monoToWString(strVal);
					else
						fieldData->isNull = allowNull;

					return fieldData;
				}
			default:
				break;
			}
		}
		else if (typeInfo->GetTypeId() == TID_SerializableTypeInfoRef)
		{
			auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);
			switch (refTypeInfo->mType)
			{
			case ScriptReferenceType::SceneObject:
			{
				auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataGameObjectRef>();

				if (value != nullptr)
				{
					ScriptSceneObject* scriptSceneObject = ScriptSceneObject::toNative(value);
					fieldData->value = scriptSceneObject->GetNativeHandle();
				}

				return fieldData;
			}
			case ScriptReferenceType::ManagedComponentBase:
			case ScriptReferenceType::ManagedComponent:
			{
				auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataGameObjectRef>();

				if (value != nullptr)
				{
					ScriptManagedComponent* scriptComponent = ScriptManagedComponent::toNative(value);
					fieldData->value = scriptComponent->GetNativeHandle();
				}

				return fieldData;
			}
			case ScriptReferenceType::BuiltinComponentBase:
			case ScriptReferenceType::BuiltinComponent:
			{
				BuiltinComponentInfo* info = ScriptAssemblyManager::instance().GetBuiltinComponentInfo(refTypeInfo->mRTIITypeId);
				if (info == nullptr)
					return nullptr;

				auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataGameObjectRef>();

				if (value != nullptr)
				{
					ScriptComponentBase* scriptComponent = ScriptComponent::toNative(value);
					fieldData->value = static_object_cast<GameObject>(scriptComponent->GetComponent());
				}

				return fieldData;
			}
			case ScriptReferenceType::ManagedResourceBase:
			case ScriptReferenceType::ManagedResource:
			{
				auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataResourceRef>();

				if (value != nullptr)
				{
					ScriptResourceBase* scriptResource = ScriptManagedResource::toNative(value);
					fieldData->value = scriptResource->GetGenericHandle();
				}

				return fieldData;
			}
			case ScriptReferenceType::BuiltinResourceBase:
			case ScriptReferenceType::BuiltinResource:
			{
				BuiltinResourceInfo* info = ScriptAssemblyManager::instance().GetBuiltinResourceInfo(refTypeInfo->mRTIITypeId);
				if (info == nullptr)
					return nullptr;

				auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataResourceRef>();

				if (value != nullptr)
				{
					ScriptResourceBase* scriptResource = ScriptResource::toNative(value);
					fieldData->value = scriptResource->GetGenericHandle();
				}

				return fieldData;
			}
			case ScriptReferenceType::ReflectableObject:
			{
				ReflectableTypeInfo* info = ScriptAssemblyManager::instance().GetReflectableTypeInfo(refTypeInfo->mRTIITypeId);
				if (info == nullptr)
					return nullptr;

				auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataReflectableRef>();

				if (value != nullptr)
				{
					ScriptReflectableBase* scriptReflectable = (ScriptReflectableBase*)ScriptObjectImpl::toNative(value);
					fieldData->value = scriptReflectable->GetReflectable();
				}

				return fieldData;
			}
			default:
				break;
			}
		}
		else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRRef)
		{
			auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataResourceRef>();

			if(value != nullptr)
			{
				ScriptRRefBase* scriptRRefBase = ScriptRRefBase::toNative(value);
				fieldData->value = scriptRRefBase->GetHandle();
			}

			return fieldData;
		}
		else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoObject)
		{
			auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataObject>();
			if (value != nullptr)
				fieldData->value = ManagedSerializableObject::createFromExisting(value);
			else if (!allowNull)
				fieldData->value = ManagedSerializableObject::createNew(std::static_pointer_cast<ManagedSerializableTypeInfoObject>(typeInfo));

			return fieldData;
		}
		else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoArray)
		{
			SPtr<ManagedSerializableTypeInfoArray> arrayTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoArray>(typeInfo);

			auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataArray>();
			if(value != nullptr)
				fieldData->value = ManagedSerializableArray::createFromExisting(value, arrayTypeInfo);
			else if (!allowNull)
			{
				Vector<UINT32> Sizes(arrayTypeInfo->mRank, 0);
				fieldData->value = ManagedSerializableArray::createNew(arrayTypeInfo, sizes);
			}

			return fieldData;
		}
		else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoList)
		{
			SPtr<ManagedSerializableTypeInfoList> listTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoList>(typeInfo);

			auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataList>();
			if(value != nullptr)
				fieldData->value = ManagedSerializableList::createFromExisting(value, listTypeInfo);
			else if (!allowNull)
				fieldData->value = ManagedSerializableList::createNew(listTypeInfo, 0);

			return fieldData;
		}
		else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoDictionary)
		{
			SPtr<ManagedSerializableTypeInfoDictionary> dictTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(typeInfo);

			auto fieldData = bs_shared_ptr_new<ManagedSerializableFieldDataDictionary>();
			if(value != nullptr)
				fieldData->value = ManagedSerializableDictionary::createFromExisting(value, dictTypeInfo);
			else if (!allowNull)
				fieldData->value = ManagedSerializableDictionary::createNew(dictTypeInfo);

			return fieldData;
		}

		return nullptr;
	}

	void* ManagedSerializableFieldDataBool::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Bool))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataChar::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Char))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataI8::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I8))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataU8::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U8))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataI16::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I16))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataU16::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U16))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataI32::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I32))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataU32::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U32))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataI64::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I64))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataU64::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U64))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataFloat::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
		{
			auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
			if(primitiveTypeInfo->mType == ScriptPrimitiveType::Float)
				return &value;
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataDouble::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
		{
			auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
			if(primitiveTypeInfo->mType == ScriptPrimitiveType::Double)
				return &value;
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataString::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
		{
			auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
			if(primitiveTypeInfo->mType == ScriptPrimitiveType::String)
			{
				if (!isNull)
					return MonoUtil::WstringToMono(value);
				else
					return nullptr;
			}
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataResourceRef::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRef)
		{
			const auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);

			if (!value.IsLoaded())
				return nullptr;

			if (refTypeInfo->mType == ScriptReferenceType::ManagedResourceBase ||
				refTypeInfo->mType == ScriptReferenceType::ManagedResource)
			{
				ScriptResourceBase* scriptResource = ScriptResourceManager::instance().GetScriptResource(value, false);
				assert(scriptResource != nullptr);

				return scriptResource->GetManagedInstance();
			}
			else if (refTypeInfo->mType == ScriptReferenceType::BuiltinResourceBase ||
					 refTypeInfo->mType == ScriptReferenceType::BuiltinResource)
			{
				ScriptResourceBase* scriptResource = ScriptResourceManager::instance().GetScriptResource(value, true);

				return scriptResource->GetManagedInstance();
			}
		}
		else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRRef)
		{
			const auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRRef>(typeInfo);

			::MonoClass* resourceRRefClass = nullptr;
			if(refTypeInfo->mResourceType)
			{
				if (!typeInfo->IsTypeLoaded())
					return nullptr;

				resourceRRefClass = typeInfo->GetMonoClass();
				if (resourceRRefClass == nullptr)
					return nullptr;
			}

			// Note: Each reference ref ends up creating its own object instance. Perhaps share the same instance between
			// all references to the same resource?

			return ScriptRRefBase::Create(value, resourceRRefClass);
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataGameObjectRef::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRef)
		{
			auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);

			if(refTypeInfo->mType == ScriptReferenceType::SceneObject)
			{
				if(value)
				{
					ScriptSceneObject* scriptSceneObject =
						ScriptGameObjectManager::instance().GetOrCreateScriptSceneObject(static_object_cast<SceneObject>(value));
					return scriptSceneObject->GetManagedInstance();
				}
				else
					return nullptr;
			}
			else if(refTypeInfo->mType == ScriptReferenceType::ManagedComponentBase ||
					refTypeInfo->mType == ScriptReferenceType::ManagedComponent)
			{
				if (value)
				{
					ScriptManagedComponent* scriptComponent =
						ScriptGameObjectManager::instance().GetManagedScriptComponent(static_object_cast<ManagedComponent>(value));
					assert(scriptComponent != nullptr);

					return scriptComponent->GetManagedInstance();
				}
				else
					return nullptr;
			}
			else if (refTypeInfo->mType == ScriptReferenceType::BuiltinComponentBase ||
					 refTypeInfo->mType == ScriptReferenceType::BuiltinComponent)
			{
				if (value)
				{
					ScriptComponentBase* scriptComponent =
						ScriptGameObjectManager::instance().GetBuiltinScriptComponent(static_object_cast<Component>(value));
					assert(scriptComponent != nullptr);

					return scriptComponent->GetManagedInstance();
				}
				else
					return nullptr;
			}
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataReflectableRef::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRef)
		{
			const auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);

			if (!value)
				return nullptr;

			UINT32 rttiId = refTypeInfo->mRTIITypeId;
			ReflectableTypeInfo* info = ScriptAssemblyManager::instance().GetReflectableTypeInfo(rttiId);

			if (info == nullptr)
				return nullptr;

			return info->CreateCallback(value);
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataObject::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoObject)
		{
			auto objectTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoObject>(typeInfo);

			if(value != nullptr)
			{
				if(objectTypeInfo->mValueType)
				{
					MonoObject* managedInstance = value->GetManagedInstance();
					
					if(managedInstance != nullptr)
						return MonoUtil::Unbox(managedInstance); // Structs are passed as raw types because mono expects them as such
				}
				else
					return value->GetManagedInstance();
			}

			return nullptr;
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataArray::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoArray)
		{
			auto objectTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoArray>(typeInfo);

			if(value != nullptr)
				return value->GetManagedInstance();

			return nullptr;
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataList::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoList)
		{
			auto listTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoList>(typeInfo);

			if(value != nullptr)
				return value->GetManagedInstance();

			return nullptr;
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataDictionary::getValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoDictionary)
		{
			auto dictionaryTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(typeInfo);

			if(value != nullptr)
				return value->GetManagedInstance();

			return nullptr;
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataBool::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Bool))
			return MonoUtil::Box(MonoUtil::getBoolClass(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataChar::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Char))
			return MonoUtil::Box(MonoUtil::getCharClass(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataI8::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I8))
			return MonoUtil::Box(MonoUtil::getSByteClass(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataU8::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U8))
			return MonoUtil::Box(MonoUtil::getByteClass(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataI16::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I16))
			return MonoUtil::Box(MonoUtil::getINT16Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataU16::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U16))
			return MonoUtil::Box(MonoUtil::getUINT16Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataI32::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I32))
			return MonoUtil::Box(MonoUtil::getINT32Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataU32::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U32))
			return MonoUtil::Box(MonoUtil::getUINT32Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataI64::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I64))
			return MonoUtil::Box(MonoUtil::getINT64Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataU64::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U64))
			return MonoUtil::Box(MonoUtil::getUINT64Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataFloat::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if (typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
		{
			auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
			if (primitiveTypeInfo->mType == ScriptPrimitiveType::Float)
				return MonoUtil::Box(MonoUtil::getFloatClass(), &value);
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataDouble::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if (typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
		{
			auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
			if (primitiveTypeInfo->mType == ScriptPrimitiveType::Double)
				return MonoUtil::Box(MonoUtil::getDoubleClass(), &value);
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataString::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataResourceRef::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataGameObjectRef::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataReflectableRef::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataObject::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if (typeInfo->GetTypeId() == TID_SerializableTypeInfoObject)
		{
			auto objectTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoObject>(typeInfo);

			if (value != nullptr)
				return value->GetManagedInstance();

			return nullptr;
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataArray::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataList::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataDictionary::getValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	bool ManagedSerializableFieldDataBool::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataChar::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataI8::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataU8::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataI16::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataU16::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataI32::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataU32::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataI64::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataU64::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataFloat::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataDouble::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataString::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		if (rtti_is_of_type<ManagedSerializableFieldDataString>(other))
		{
			auto castObj = std::static_pointer_cast<ManagedSerializableFieldDataString>(other);
			return (isNull == true && isNull == castObj->isNull) || value == castObj->value;
		}

		return false;
	}

	bool ManagedSerializableFieldDataResourceRef::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataGameObjectRef::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataReflectableRef::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return CompareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataObject::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		if (auto otherObj = rtti_cast<ManagedSerializableFieldDataObject>(other))
		{
			if(!value && !otherObj->value)
				return true;


			if((value == nullptr && otherObj->value) || (value && !otherObj->value))
				return false;

			return value->Equals(*otherObj->value);
		}

		return false;
	}

	bool ManagedSerializableFieldDataArray::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		if (auto otherObj = rtti_cast<ManagedSerializableFieldDataArray>(other))
		{
			if(!value && !otherObj->value)
				return true;

			if((!value && otherObj->value) || (value && !otherObj->value))
				return false;

			UINT32 oldLength = value->GetTotalLength();
			UINT32 newLength = otherObj->value->GetTotalLength();

			if(oldLength != newLength)
				return false;

			for (UINT32 i = 0; i < newLength; i++)
			{
				SPtr<ManagedSerializableFieldData> oldData = value->GetFieldData(i);
				SPtr<ManagedSerializableFieldData> newData = otherObj->value->GetFieldData(i);

				if (compareFieldData(oldData, newData))
					return false;
			}

			return true;
		}

		return false;
	}

	bool ManagedSerializableFieldDataList::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		if (auto otherObj = rtti_cast<ManagedSerializableFieldDataList>(other))
		{
			if(!value && !otherObj->value)
				return true;

			if((!value && otherObj->value) || (value && !otherObj->value))
				return false;

			UINT32 oldLength = value->GetLength();
			UINT32 newLength = otherObj->value->GetLength();

			if(oldLength != newLength)
				return false;

			for (UINT32 i = 0; i < newLength; i++)
			{
				SPtr<ManagedSerializableFieldData> oldData = value->GetFieldData(i);
				SPtr<ManagedSerializableFieldData> newData = otherObj->value->GetFieldData(i);

				if (compareFieldData(oldData, newData))
					return false;
			}

			return true;
		}

		return false;
	}

	bool ManagedSerializableFieldDataDictionary::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		if (auto otherObj = rtti_cast<ManagedSerializableFieldDataDictionary>(other))
		{
			if(!value && !otherObj->value)
				return true;

			if((!value && otherObj->value) || (value && !otherObj->value))
				return false;

			auto newEnumerator = otherObj->value->GetEnumerator();
			while (newEnumerator.MoveNext())
			{
				SPtr<ManagedSerializableFieldData> key = newEnumerator.GetKey();
				if (value->Contains(key))
				{
					if(!compareFieldData(value->GetFieldData(key), newEnumerator.GetValue()))
						return false;
				}
				else
					return false;
			}

			auto oldEnumerator = value->GetEnumerator();
			while (oldEnumerator.MoveNext())
			{
				SPtr<ManagedSerializableFieldData> key = oldEnumerator.GetKey();
				if (!otherObj->value->Contains(oldEnumerator.GetKey()))
					return false;
			}

			return true;
		}

		return false;;
	}

	size_t ManagedSerializableFieldDataBool::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataChar::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataI8::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataU8::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataI16::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataU16::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataI32::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataU32::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataI64::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataU64::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataFloat::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataDouble::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataString::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataResourceRef::GetHash()
	{
		return bs_hash(value.GetUUID());
	}

	size_t ManagedSerializableFieldDataGameObjectRef::GetHash()
	{
		return bs_hash(value.GetInstanceId());
	}

	size_t ManagedSerializableFieldDataReflectableRef::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataObject::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataArray::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataList::GetHash()
	{
		return bs_hash(value);
	}

	size_t ManagedSerializableFieldDataDictionary::GetHash()
	{
		return bs_hash(value);
	}

	void ManagedSerializableFieldDataObject::Serialize()
	{
		if (value != nullptr)
			value->Serialize();
	}

	void ManagedSerializableFieldDataObject::Deserialize()
	{
		if (value != nullptr)
		{
			MonoObject* managedInstance = value->Deserialize();
			value = ManagedSerializableObject::createFromExisting(managedInstance);
		}
	}

	void ManagedSerializableFieldDataArray::Serialize()
	{
		if (value != nullptr)
			value->Serialize();
	}

	void ManagedSerializableFieldDataArray::Deserialize()
	{
		if (value != nullptr)
		{
			MonoObject* managedInstance = value->Deserialize();
			value = ManagedSerializableArray::createFromExisting(managedInstance, value->GetTypeInfo());
		}
	}

	void ManagedSerializableFieldDataList::Serialize()
	{
		if (value != nullptr)
			value->Serialize();
	}

	void ManagedSerializableFieldDataList::Deserialize()
	{
		if (value != nullptr)
		{
			MonoObject* managedInstance = value->Deserialize();
			value = ManagedSerializableList::createFromExisting(managedInstance, value->GetTypeInfo());
		}
	}

	void ManagedSerializableFieldDataDictionary::Serialize()
	{
		if (value != nullptr)
			value->Serialize();
	}

	void ManagedSerializableFieldDataDictionary::Deserialize()
	{
		if (value != nullptr)
		{
			MonoObject* managedInstance = value->Deserialize();
			value = ManagedSerializableDictionary::createFromExisting(managedInstance, value->GetTypeInfo());
		}
	}

	RTTITypeBase* ManagedSerializableFieldKey::getRTTIStatic()
	{
		return ManagedSerializableFieldKeyRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldKey::getRTTI() const
	{
		return ManagedSerializableFieldKey::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldData::getRTTIStatic()
	{
		return ManagedSerializableFieldDataRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldData::getRTTI() const
	{
		return ManagedSerializableFieldData::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataEntry::getRTTIStatic()
	{
		return ManagedSerializableFieldDataEntryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataEntry::getRTTI() const
	{
		return ManagedSerializableFieldDataEntry::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataBool::getRTTIStatic()
	{
		return ManagedSerializableFieldDataBoolRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataBool::getRTTI() const
	{
		return ManagedSerializableFieldDataBool::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataChar::getRTTIStatic()
	{
		return ManagedSerializableFieldDataCharRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataChar::getRTTI() const
	{
		return ManagedSerializableFieldDataChar::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataI8::getRTTIStatic()
	{
		return ManagedSerializableFieldDataI8RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataI8::getRTTI() const
	{
		return ManagedSerializableFieldDataI8::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataU8::getRTTIStatic()
	{
		return ManagedSerializableFieldDataU8RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataU8::getRTTI() const
	{
		return ManagedSerializableFieldDataU8::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataI16::getRTTIStatic()
	{
		return ManagedSerializableFieldDataI16RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataI16::getRTTI() const
	{
		return ManagedSerializableFieldDataI16::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataU16::getRTTIStatic()
	{
		return ManagedSerializableFieldDataU16RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataU16::getRTTI() const
	{
		return ManagedSerializableFieldDataU16::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataI32::getRTTIStatic()
	{
		return ManagedSerializableFieldDataI32RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataI32::getRTTI() const
	{
		return ManagedSerializableFieldDataI32::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataU32::getRTTIStatic()
	{
		return ManagedSerializableFieldDataU32RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataU32::getRTTI() const
	{
		return ManagedSerializableFieldDataU32::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataI64::getRTTIStatic()
	{
		return ManagedSerializableFieldDataI64RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataI64::getRTTI() const
	{
		return ManagedSerializableFieldDataI64::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataU64::getRTTIStatic()
	{
		return ManagedSerializableFieldDataU64RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataU64::getRTTI() const
	{
		return ManagedSerializableFieldDataU64::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataFloat::getRTTIStatic()
	{
		return ManagedSerializableFieldDataFloatRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataFloat::getRTTI() const
	{
		return ManagedSerializableFieldDataFloat::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataDouble::getRTTIStatic()
	{
		return ManagedSerializableFieldDataDoubleRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataDouble::getRTTI() const
	{
		return ManagedSerializableFieldDataDouble::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataString::getRTTIStatic()
	{
		return ManagedSerializableFieldDataStringRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataString::getRTTI() const
	{
		return ManagedSerializableFieldDataString::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataResourceRef::getRTTIStatic()
	{
		return ManagedSerializableFieldDataResourceRefRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataResourceRef::getRTTI() const
	{
		return ManagedSerializableFieldDataResourceRef::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataGameObjectRef::getRTTIStatic()
	{
		return ManagedSerializableFieldDataGameObjectRefRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataGameObjectRef::getRTTI() const
	{
		return ManagedSerializableFieldDataGameObjectRef::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataReflectableRef::getRTTIStatic()
	{
		return ManagedSerializableFieldDataReflectableRefRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataReflectableRef::getRTTI() const
	{
		return ManagedSerializableFieldDataReflectableRef::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataObject::getRTTIStatic()
	{
		return ManagedSerializableFieldDataObjectRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataObject::getRTTI() const
	{
		return ManagedSerializableFieldDataObject::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataArray::getRTTIStatic()
	{
		return ManagedSerializableFieldDataArrayRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataArray::getRTTI() const
	{
		return ManagedSerializableFieldDataArray::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataList::getRTTIStatic()
	{
		return ManagedSerializableFieldDataListRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataList::getRTTI() const
	{
		return ManagedSerializableFieldDataList::GetRTTIStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataDictionary::getRTTIStatic()
	{
		return ManagedSerializableFieldDataDictionaryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataDictionary::getRTTI() const
	{
		return ManagedSerializableFieldDataDictionary::GetRTTIStatic();
	}
}
