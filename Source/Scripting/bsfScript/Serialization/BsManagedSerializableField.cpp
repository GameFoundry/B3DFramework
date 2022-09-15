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
	bool compareFieldData(const T* a, const SPtr<ManagedSerializableFieldData>& b)
	{
		if (rtti_is_of_type<T>(b))
		{
			auto castObj = std::static_pointer_cast<T>(b);
			return a->value == castObj->value;
		}

		return false;
	}

	bool compareFieldData(const SPtr<ManagedSerializableFieldData>& oldData, const SPtr<ManagedSerializableFieldData>& newData)
	{
		if (!oldData)
			return !newData;
		else
		{
			if (!newData)
				return false;
		}

		return oldData->equals(newData);
	}

	bool isPrimitiveOrEnumType(const SPtr<ManagedSerializableTypeInfo>& typeInfo, ScriptPrimitiveType underlyingType)
	{
		if(const auto primitiveTypeInfo = rtti_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo.get()))
			return primitiveTypeInfo->mType == underlyingType;
		else if(const auto enumTypeInfo = rtti_cast<ManagedSerializableTypeInfoEnum>(typeInfo.get()))
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
		return create(typeInfo, value, true);
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableFieldData::CreateDefault(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return create(typeInfo, nullptr, false);
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableFieldData::Create(const SPtr<ManagedSerializableTypeInfo>& typeInfo, MonoObject* value, bool allowNull)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive || typeInfo->GetTypeId() == TID_SerializableTypeInfoEnum)
		{
			ScriptPrimitiveType primitiveType = ScriptPrimitiveType::I32;

			if(auto primitiveTypeInfo = rtti_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo.get()))
				primitiveType = primitiveTypeInfo->mType;
			else if(auto enumTypeInfo = rtti_cast<ManagedSerializableTypeInfoEnum>(typeInfo.get()))
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
				BuiltinComponentInfo* info = ScriptAssemblyManager::Instance().getBuiltinComponentInfo(refTypeInfo->mRTIITypeId);
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
				BuiltinResourceInfo* info = ScriptAssemblyManager::Instance().getBuiltinResourceInfo(refTypeInfo->mRTIITypeId);
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
				ReflectableTypeInfo* info = ScriptAssemblyManager::Instance().getReflectableTypeInfo(refTypeInfo->mRTIITypeId);
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
				Vector<UINT32> sizes(arrayTypeInfo->mRank, 0);
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

	void* ManagedSerializableFieldDataBool::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Bool))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataChar::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Char))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataI8::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I8))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataU8::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U8))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataI16::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I16))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataU16::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U16))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataI32::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I32))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataU32::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U32))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataI64::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I64))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataU64::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U64))
			return &value;

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataFloat::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
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

	void* ManagedSerializableFieldDataDouble::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
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

	void* ManagedSerializableFieldDataString::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
		{
			auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
			if(primitiveTypeInfo->mType == ScriptPrimitiveType::String)
			{
				if (!isNull)
					return MonoUtil::wstringToMono(value);
				else
					return nullptr;
			}
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataResourceRef::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRef)
		{
			const auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);

			if (!value.IsLoaded())
				return nullptr;

			if (refTypeInfo->mType == ScriptReferenceType::ManagedResourceBase ||
				refTypeInfo->mType == ScriptReferenceType::ManagedResource)
			{
				ScriptResourceBase* scriptResource = ScriptResourceManager::Instance().getScriptResource(value, false);
				assert(scriptResource != nullptr);

				return scriptResource->GetManagedInstance();
			}
			else if (refTypeInfo->mType == ScriptReferenceType::BuiltinResourceBase ||
					 refTypeInfo->mType == ScriptReferenceType::BuiltinResource)
			{
				ScriptResourceBase* scriptResource = ScriptResourceManager::Instance().getScriptResource(value, true);

				return scriptResource->GetManagedInstance();
			}
		}
		else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRRef)
		{
			const auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRRef>(typeInfo);

			::MonoClass* resourceRRefClass = nullptr;
			if(refTypeInfo->mResourceType)
			{
				if (!typeInfo->isTypeLoaded())
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

	void* ManagedSerializableFieldDataGameObjectRef::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRef)
		{
			auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);

			if(refTypeInfo->mType == ScriptReferenceType::SceneObject)
			{
				if(value)
				{
					ScriptSceneObject* scriptSceneObject =
						ScriptGameObjectManager::Instance().getOrCreateScriptSceneObject(static_object_cast<SceneObject>(value));
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
						ScriptGameObjectManager::Instance().getManagedScriptComponent(static_object_cast<ManagedComponent>(value));
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
						ScriptGameObjectManager::Instance().getBuiltinScriptComponent(static_object_cast<Component>(value));
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

	void* ManagedSerializableFieldDataReflectableRef::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRef)
		{
			const auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);

			if (!value)
				return nullptr;

			UINT32 rttiId = refTypeInfo->mRTIITypeId;
			ReflectableTypeInfo* info = ScriptAssemblyManager::Instance().getReflectableTypeInfo(rttiId);

			if (info == nullptr)
				return nullptr;

			return info->createCallback(value);
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataObject::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
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
						return MonoUtil::unbox(managedInstance); // Structs are passed as raw types because mono expects them as such
				}
				else
					return value->GetManagedInstance();
			}

			return nullptr;
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	void* ManagedSerializableFieldDataArray::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
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

	void* ManagedSerializableFieldDataList::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
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

	void* ManagedSerializableFieldDataDictionary::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
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

	MonoObject* ManagedSerializableFieldDataBool::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Bool))
			return MonoUtil::box(MonoUtil::getBoolClass(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataChar::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Char))
			return MonoUtil::box(MonoUtil::getCharClass(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataI8::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I8))
			return MonoUtil::box(MonoUtil::getSByteClass(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataU8::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U8))
			return MonoUtil::box(MonoUtil::getByteClass(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataI16::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I16))
			return MonoUtil::box(MonoUtil::getINT16Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataU16::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U16))
			return MonoUtil::box(MonoUtil::getUINT16Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataI32::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I32))
			return MonoUtil::box(MonoUtil::getINT32Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataU32::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U32))
			return MonoUtil::box(MonoUtil::getUINT32Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataI64::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I64))
			return MonoUtil::box(MonoUtil::getINT64Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataU64::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U64))
			return MonoUtil::box(MonoUtil::getUINT64Class(), &value);

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataFloat::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if (typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
		{
			auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
			if (primitiveTypeInfo->mType == ScriptPrimitiveType::Float)
				return MonoUtil::box(MonoUtil::getFloatClass(), &value);
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataDouble::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		if (typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
		{
			auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
			if (primitiveTypeInfo->mType == ScriptPrimitiveType::Double)
				return MonoUtil::box(MonoUtil::getDoubleClass(), &value);
		}

		BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
		return nullptr;
	}

	MonoObject* ManagedSerializableFieldDataString::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataResourceRef::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataGameObjectRef::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataReflectableRef::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataObject::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
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

	MonoObject* ManagedSerializableFieldDataArray::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataList::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	MonoObject* ManagedSerializableFieldDataDictionary::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
	{
		return (MonoObject*)getValue(typeInfo);
	}

	bool ManagedSerializableFieldDataBool::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataChar::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataI8::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataU8::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataI16::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataU16::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataI32::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataU32::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataI64::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataU64::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataFloat::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataDouble::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
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
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataGameObjectRef::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataReflectableRef::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		return compareFieldData(this, other);
	}

	bool ManagedSerializableFieldDataObject::Equals(const SPtr<ManagedSerializableFieldData>& other)
	{
		if (auto otherObj = rtti_cast<ManagedSerializableFieldDataObject>(other))
		{
			if(!value && !otherObj->value)
				return true;


			if((value == nullptr && otherObj->value) || (value && !otherObj->value))
				return false;

			return value->equals(*otherObj->value);
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
			while (newEnumerator.moveNext())
			{
				SPtr<ManagedSerializableFieldData> key = newEnumerator.getKey();
				if (value->contains(key))
				{
					if(!compareFieldData(value->GetFieldData(key), newEnumerator.getValue()))
						return false;
				}
				else
					return false;
			}

			auto oldEnumerator = value->GetEnumerator();
			while (oldEnumerator.moveNext())
			{
				SPtr<ManagedSerializableFieldData> key = oldEnumerator.getKey();
				if (!otherObj->value->contains(oldEnumerator.getKey()))
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
		return bs_hash(value.GetUuid());
	}

	size_t ManagedSerializableFieldDataGameObjectRef::GetHash()
	{
		return bs_hash(value.getInstanceId());
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
			value->serialize();
	}

	void ManagedSerializableFieldDataObject::Deserialize()
	{
		if (value != nullptr)
		{
			MonoObject* managedInstance = value->deserialize();
			value = ManagedSerializableObject::createFromExisting(managedInstance);
		}
	}

	void ManagedSerializableFieldDataArray::Serialize()
	{
		if (value != nullptr)
			value->serialize();
	}

	void ManagedSerializableFieldDataArray::Deserialize()
	{
		if (value != nullptr)
		{
			MonoObject* managedInstance = value->deserialize();
			value = ManagedSerializableArray::createFromExisting(managedInstance, value->GetTypeInfo());
		}
	}

	void ManagedSerializableFieldDataList::Serialize()
	{
		if (value != nullptr)
			value->serialize();
	}

	void ManagedSerializableFieldDataList::Deserialize()
	{
		if (value != nullptr)
		{
			MonoObject* managedInstance = value->deserialize();
			value = ManagedSerializableList::createFromExisting(managedInstance, value->GetTypeInfo());
		}
	}

	void ManagedSerializableFieldDataDictionary::Serialize()
	{
		if (value != nullptr)
			value->serialize();
	}

	void ManagedSerializableFieldDataDictionary::Deserialize()
	{
		if (value != nullptr)
		{
			MonoObject* managedInstance = value->deserialize();
			value = ManagedSerializableDictionary::createFromExisting(managedInstance, value->GetTypeInfo());
		}
	}

	RTTITypeBase* ManagedSerializableFieldKey::GetRttiStatic()
	{
		return ManagedSerializableFieldKeyRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldKey::GetRtti() const
	{
		return ManagedSerializableFieldKey::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldData::GetRttiStatic()
	{
		return ManagedSerializableFieldDataRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldData::GetRtti() const
	{
		return ManagedSerializableFieldData::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataEntry::GetRttiStatic()
	{
		return ManagedSerializableFieldDataEntryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataEntry::GetRtti() const
	{
		return ManagedSerializableFieldDataEntry::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataBool::GetRttiStatic()
	{
		return ManagedSerializableFieldDataBoolRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataBool::GetRtti() const
	{
		return ManagedSerializableFieldDataBool::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataChar::GetRttiStatic()
	{
		return ManagedSerializableFieldDataCharRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataChar::GetRtti() const
	{
		return ManagedSerializableFieldDataChar::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataI8::GetRttiStatic()
	{
		return ManagedSerializableFieldDataI8RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataI8::GetRtti() const
	{
		return ManagedSerializableFieldDataI8::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataU8::GetRttiStatic()
	{
		return ManagedSerializableFieldDataU8RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataU8::GetRtti() const
	{
		return ManagedSerializableFieldDataU8::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataI16::GetRttiStatic()
	{
		return ManagedSerializableFieldDataI16RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataI16::GetRtti() const
	{
		return ManagedSerializableFieldDataI16::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataU16::GetRttiStatic()
	{
		return ManagedSerializableFieldDataU16RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataU16::GetRtti() const
	{
		return ManagedSerializableFieldDataU16::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataI32::GetRttiStatic()
	{
		return ManagedSerializableFieldDataI32RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataI32::GetRtti() const
	{
		return ManagedSerializableFieldDataI32::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataU32::GetRttiStatic()
	{
		return ManagedSerializableFieldDataU32RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataU32::GetRtti() const
	{
		return ManagedSerializableFieldDataU32::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataI64::GetRttiStatic()
	{
		return ManagedSerializableFieldDataI64RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataI64::GetRtti() const
	{
		return ManagedSerializableFieldDataI64::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataU64::GetRttiStatic()
	{
		return ManagedSerializableFieldDataU64RTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataU64::GetRtti() const
	{
		return ManagedSerializableFieldDataU64::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataFloat::GetRttiStatic()
	{
		return ManagedSerializableFieldDataFloatRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataFloat::GetRtti() const
	{
		return ManagedSerializableFieldDataFloat::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataDouble::GetRttiStatic()
	{
		return ManagedSerializableFieldDataDoubleRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataDouble::GetRtti() const
	{
		return ManagedSerializableFieldDataDouble::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataString::GetRttiStatic()
	{
		return ManagedSerializableFieldDataStringRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataString::GetRtti() const
	{
		return ManagedSerializableFieldDataString::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataResourceRef::GetRttiStatic()
	{
		return ManagedSerializableFieldDataResourceRefRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataResourceRef::GetRtti() const
	{
		return ManagedSerializableFieldDataResourceRef::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataGameObjectRef::GetRttiStatic()
	{
		return ManagedSerializableFieldDataGameObjectRefRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataGameObjectRef::GetRtti() const
	{
		return ManagedSerializableFieldDataGameObjectRef::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataReflectableRef::GetRttiStatic()
	{
		return ManagedSerializableFieldDataReflectableRefRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataReflectableRef::GetRtti() const
	{
		return ManagedSerializableFieldDataReflectableRef::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataObject::GetRttiStatic()
	{
		return ManagedSerializableFieldDataObjectRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataObject::GetRtti() const
	{
		return ManagedSerializableFieldDataObject::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataArray::GetRttiStatic()
	{
		return ManagedSerializableFieldDataArrayRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataArray::GetRtti() const
	{
		return ManagedSerializableFieldDataArray::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataList::GetRttiStatic()
	{
		return ManagedSerializableFieldDataListRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataList::GetRtti() const
	{
		return ManagedSerializableFieldDataList::GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableFieldDataDictionary::GetRttiStatic()
	{
		return ManagedSerializableFieldDataDictionaryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableFieldDataDictionary::GetRtti() const
	{
		return ManagedSerializableFieldDataDictionary::GetRttiStatic();
	}
}
