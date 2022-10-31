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

using namespace bs;
template <class T>
bool compareFieldData(const T* a, const SPtr<ManagedSerializableFieldData>& b)
{
	if(rtti_is_of_type<T>(b))
	{
		auto castObj = std::static_pointer_cast<T>(b);
		return a->Value == castObj->Value;
	}

	return false;
}

bool compareFieldData(const SPtr<ManagedSerializableFieldData>& oldData, const SPtr<ManagedSerializableFieldData>& newData)
{
	if(!oldData)
		return !newData;
	else
	{
		if(!newData)
			return false;
	}

	return oldData->Equals(newData);
}

bool isPrimitiveOrEnumType(const SPtr<ManagedSerializableTypeInfo>& typeInfo, ScriptPrimitiveType underlyingType)
{
	if(const auto primitiveTypeInfo = rtti_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo.get()))
		return primitiveTypeInfo->MType == underlyingType;
	else if(const auto enumTypeInfo = rtti_cast<ManagedSerializableTypeInfoEnum>(typeInfo.get()))
		return enumTypeInfo->MUnderlyingType == underlyingType;

	return false;
}

ManagedSerializableFieldKey::ManagedSerializableFieldKey(u16 typeId, u16 fieldId)
	: MTypeId(typeId), MFieldId(fieldId)
{}

SPtr<ManagedSerializableFieldKey> ManagedSerializableFieldKey::Create(u16 typeId, u16 fieldId)
{
	SPtr<ManagedSerializableFieldKey> fieldKey = B3DMakeShared<ManagedSerializableFieldKey>(typeId, fieldId);
	return fieldKey;
}

SPtr<ManagedSerializableFieldDataEntry> ManagedSerializableFieldDataEntry::Create(const SPtr<ManagedSerializableFieldKey>& key, const SPtr<ManagedSerializableFieldData>& value)
{
	SPtr<ManagedSerializableFieldDataEntry> fieldDataEntry = B3DMakeShared<ManagedSerializableFieldDataEntry>();
	fieldDataEntry->MKey = key;
	fieldDataEntry->MValue = value;

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
	if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive || typeInfo->GetTypeId() == TID_SerializableTypeInfoEnum)
	{
		ScriptPrimitiveType primitiveType = ScriptPrimitiveType::I32;

		if(auto primitiveTypeInfo = rtti_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo.get()))
			primitiveType = primitiveTypeInfo->MType;
		else if(auto enumTypeInfo = rtti_cast<ManagedSerializableTypeInfoEnum>(typeInfo.get()))
			primitiveType = enumTypeInfo->MUnderlyingType;

		switch(primitiveType)
		{
		case ScriptPrimitiveType::Bool:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataBool>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::Char:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataChar>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::I8:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataI8>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::U8:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataU8>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::I16:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataI16>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::U16:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataU16>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::I32:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataI32>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::U32:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataU32>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::I64:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataI64>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::U64:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataU64>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::Float:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataFloat>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::Double:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataDouble>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ScriptPrimitiveType::String:
			{
				MonoString* strVal = (MonoString*)(value);

				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataString>();
				if(strVal != nullptr)
					fieldData->Value = MonoUtil::MonoToWString(strVal);
				else
					fieldData->IsNull = allowNull;

				return fieldData;
			}
		default:
			break;
		}
	}
	else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRef)
	{
		auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);
		switch(refTypeInfo->MType)
		{
		case ScriptReferenceType::SceneObject:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataGameObjectRef>();

				if(value != nullptr)
				{
					ScriptSceneObject* scriptSceneObject = ScriptSceneObject::ToNative(value);
					fieldData->Value = scriptSceneObject->GetNativeHandle();
				}

				return fieldData;
			}
		case ScriptReferenceType::ManagedComponentBase:
		case ScriptReferenceType::ManagedComponent:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataGameObjectRef>();

				if(value != nullptr)
				{
					ScriptManagedComponent* scriptComponent = ScriptManagedComponent::ToNative(value);
					fieldData->Value = scriptComponent->GetNativeHandle();
				}

				return fieldData;
			}
		case ScriptReferenceType::BuiltinComponentBase:
		case ScriptReferenceType::BuiltinComponent:
			{
				BuiltinComponentInfo* info = ScriptAssemblyManager::Instance().GetBuiltinComponentInfo(refTypeInfo->MRtiiTypeId);
				if(info == nullptr)
					return nullptr;

				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataGameObjectRef>();

				if(value != nullptr)
				{
					ScriptComponentBase* scriptComponent = ScriptComponent::ToNative(value);
					fieldData->Value = static_object_cast<GameObject>(scriptComponent->GetComponent());
				}

				return fieldData;
			}
		case ScriptReferenceType::ManagedResourceBase:
		case ScriptReferenceType::ManagedResource:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataResourceRef>();

				if(value != nullptr)
				{
					ScriptResourceBase* scriptResource = ScriptManagedResource::ToNative(value);
					fieldData->Value = scriptResource->GetGenericHandle();
				}

				return fieldData;
			}
		case ScriptReferenceType::BuiltinResourceBase:
		case ScriptReferenceType::BuiltinResource:
			{
				BuiltinResourceInfo* info = ScriptAssemblyManager::Instance().GetBuiltinResourceInfo(refTypeInfo->MRtiiTypeId);
				if(info == nullptr)
					return nullptr;

				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataResourceRef>();

				if(value != nullptr)
				{
					ScriptResourceBase* scriptResource = ScriptResource::ToNative(value);
					fieldData->Value = scriptResource->GetGenericHandle();
				}

				return fieldData;
			}
		case ScriptReferenceType::ReflectableObject:
			{
				ReflectableTypeInfo* info = ScriptAssemblyManager::Instance().GetReflectableTypeInfo(refTypeInfo->MRtiiTypeId);
				if(info == nullptr)
					return nullptr;

				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataReflectableRef>();

				if(value != nullptr)
				{
					ScriptReflectableBase* scriptReflectable = (ScriptReflectableBase*)ScriptObjectImpl::ToNative(value);
					fieldData->Value = scriptReflectable->GetReflectable();
				}

				return fieldData;
			}
		default:
			break;
		}
	}
	else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRRef)
	{
		auto fieldData = B3DMakeShared<ManagedSerializableFieldDataResourceRef>();

		if(value != nullptr)
		{
			ScriptRRefBase* scriptRRefBase = ScriptRRefBase::ToNative(value);
			fieldData->Value = scriptRRefBase->GetHandle();
		}

		return fieldData;
	}
	else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoObject)
	{
		auto fieldData = B3DMakeShared<ManagedSerializableFieldDataObject>();
		if(value != nullptr)
			fieldData->Value = ManagedSerializableObject::CreateFromExisting(value);
		else if(!allowNull)
			fieldData->Value = ManagedSerializableObject::CreateNew(std::static_pointer_cast<ManagedSerializableTypeInfoObject>(typeInfo));

		return fieldData;
	}
	else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoArray)
	{
		SPtr<ManagedSerializableTypeInfoArray> arrayTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoArray>(typeInfo);

		auto fieldData = B3DMakeShared<ManagedSerializableFieldDataArray>();
		if(value != nullptr)
			fieldData->Value = ManagedSerializableArray::CreateFromExisting(value, arrayTypeInfo);
		else if(!allowNull)
		{
			Vector<u32> sizes(arrayTypeInfo->MRank, 0);
			fieldData->Value = ManagedSerializableArray::CreateNew(arrayTypeInfo, sizes);
		}

		return fieldData;
	}
	else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoList)
	{
		SPtr<ManagedSerializableTypeInfoList> listTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoList>(typeInfo);

		auto fieldData = B3DMakeShared<ManagedSerializableFieldDataList>();
		if(value != nullptr)
			fieldData->Value = ManagedSerializableList::CreateFromExisting(value, listTypeInfo);
		else if(!allowNull)
			fieldData->Value = ManagedSerializableList::CreateNew(listTypeInfo, 0);

		return fieldData;
	}
	else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoDictionary)
	{
		SPtr<ManagedSerializableTypeInfoDictionary> dictTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(typeInfo);

		auto fieldData = B3DMakeShared<ManagedSerializableFieldDataDictionary>();
		if(value != nullptr)
			fieldData->Value = ManagedSerializableDictionary::CreateFromExisting(value, dictTypeInfo);
		else if(!allowNull)
			fieldData->Value = ManagedSerializableDictionary::CreateNew(dictTypeInfo);

		return fieldData;
	}

	return nullptr;
}

void* ManagedSerializableFieldDataBool::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Bool))
		return &Value;

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataChar::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Char))
		return &Value;

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataI8::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I8))
		return &Value;

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataU8::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U8))
		return &Value;

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataI16::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I16))
		return &Value;

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataU16::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U16))
		return &Value;

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataI32::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I32))
		return &Value;

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataU32::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U32))
		return &Value;

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataI64::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I64))
		return &Value;

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataU64::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U64))
		return &Value;

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataFloat::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
	{
		auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
		if(primitiveTypeInfo->MType == ScriptPrimitiveType::Float)
			return &Value;
	}

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataDouble::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
	{
		auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
		if(primitiveTypeInfo->MType == ScriptPrimitiveType::Double)
			return &Value;
	}

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataString::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
	{
		auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
		if(primitiveTypeInfo->MType == ScriptPrimitiveType::String)
		{
			if(!IsNull)
				return MonoUtil::WstringToMono(Value);
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

		if(!Value.IsLoaded())
			return nullptr;

		if(refTypeInfo->MType == ScriptReferenceType::ManagedResourceBase ||
		   refTypeInfo->MType == ScriptReferenceType::ManagedResource)
		{
			ScriptResourceBase* scriptResource = ScriptResourceManager::Instance().GetScriptResource(Value, false);
			assert(scriptResource != nullptr);

			return scriptResource->GetManagedInstance();
		}
		else if(refTypeInfo->MType == ScriptReferenceType::BuiltinResourceBase || refTypeInfo->MType == ScriptReferenceType::BuiltinResource)
		{
			ScriptResourceBase* scriptResource = ScriptResourceManager::Instance().GetScriptResource(Value, true);

			return scriptResource->GetManagedInstance();
		}
	}
	else if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRRef)
	{
		const auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRRef>(typeInfo);

		::MonoClass* resourceRRefClass = nullptr;
		if(refTypeInfo->MResourceType)
		{
			if(!typeInfo->IsTypeLoaded())
				return nullptr;

			resourceRRefClass = typeInfo->GetMonoClass();
			if(resourceRRefClass == nullptr)
				return nullptr;
		}

		// Note: Each reference ref ends up creating its own object instance. Perhaps share the same instance between
		// all references to the same resource?

		return ScriptRRefBase::Create(Value, resourceRRefClass);
	}

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataGameObjectRef::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_SerializableTypeInfoRef)
	{
		auto refTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoRef>(typeInfo);

		if(refTypeInfo->MType == ScriptReferenceType::SceneObject)
		{
			if(Value)
			{
				ScriptSceneObject* scriptSceneObject =
					ScriptGameObjectManager::Instance().GetOrCreateScriptSceneObject(static_object_cast<SceneObject>(Value));
				return scriptSceneObject->GetManagedInstance();
			}
			else
				return nullptr;
		}
		else if(refTypeInfo->MType == ScriptReferenceType::ManagedComponentBase || refTypeInfo->MType == ScriptReferenceType::ManagedComponent)
		{
			if(Value)
			{
				ScriptManagedComponent* scriptComponent =
					ScriptGameObjectManager::Instance().GetManagedScriptComponent(static_object_cast<ManagedComponent>(Value));
				assert(scriptComponent != nullptr);

				return scriptComponent->GetManagedInstance();
			}
			else
				return nullptr;
		}
		else if(refTypeInfo->MType == ScriptReferenceType::BuiltinComponentBase || refTypeInfo->MType == ScriptReferenceType::BuiltinComponent)
		{
			if(Value)
			{
				ScriptComponentBase* scriptComponent =
					ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(static_object_cast<Component>(Value));
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

		if(!Value)
			return nullptr;

		u32 rttiId = refTypeInfo->MRtiiTypeId;
		ReflectableTypeInfo* info = ScriptAssemblyManager::Instance().GetReflectableTypeInfo(rttiId);

		if(info == nullptr)
			return nullptr;

		return info->CreateCallback(Value);
	}

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

void* ManagedSerializableFieldDataObject::GetValue(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_SerializableTypeInfoObject)
	{
		auto objectTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoObject>(typeInfo);

		if(Value != nullptr)
		{
			if(objectTypeInfo->MValueType)
			{
				MonoObject* managedInstance = Value->GetManagedInstance();

				if(managedInstance != nullptr)
					return MonoUtil::Unbox(managedInstance); // Structs are passed as raw types because mono expects them as such
			}
			else
				return Value->GetManagedInstance();
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

		if(Value != nullptr)
			return Value->GetManagedInstance();

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

		if(Value != nullptr)
			return Value->GetManagedInstance();

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

		if(Value != nullptr)
			return Value->GetManagedInstance();

		return nullptr;
	}

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataBool::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Bool))
		return MonoUtil::Box(MonoUtil::GetBoolClass(), &Value);

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataChar::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::Char))
		return MonoUtil::Box(MonoUtil::GetCharClass(), &Value);

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataI8::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I8))
		return MonoUtil::Box(MonoUtil::GetSByteClass(), &Value);

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataU8::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U8))
		return MonoUtil::Box(MonoUtil::GetByteClass(), &Value);

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataI16::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I16))
		return MonoUtil::Box(MonoUtil::GetInT16Class(), &Value);

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataU16::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U16))
		return MonoUtil::Box(MonoUtil::GetUinT16Class(), &Value);

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataI32::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I32))
		return MonoUtil::Box(MonoUtil::GetInT32Class(), &Value);

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataU32::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U32))
		return MonoUtil::Box(MonoUtil::GetUinT32Class(), &Value);

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataI64::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::I64))
		return MonoUtil::Box(MonoUtil::GetInT64Class(), &Value);

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataU64::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(isPrimitiveOrEnumType(typeInfo, ScriptPrimitiveType::U64))
		return MonoUtil::Box(MonoUtil::GetUinT64Class(), &Value);

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataFloat::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
	{
		auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
		if(primitiveTypeInfo->MType == ScriptPrimitiveType::Float)
			return MonoUtil::Box(MonoUtil::GetFloatClass(), &Value);
	}

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataDouble::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_SerializableTypeInfoPrimitive)
	{
		auto primitiveTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoPrimitive>(typeInfo);
		if(primitiveTypeInfo->MType == ScriptPrimitiveType::Double)
			return MonoUtil::Box(MonoUtil::GetDoubleClass(), &Value);
	}

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataString::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataResourceRef::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataGameObjectRef::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataReflectableRef::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataObject::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_SerializableTypeInfoObject)
	{
		auto objectTypeInfo = std::static_pointer_cast<ManagedSerializableTypeInfoObject>(typeInfo);

		if(Value != nullptr)
			return Value->GetManagedInstance();

		return nullptr;
	}

	BS_EXCEPT(InvalidParametersException, "Requesting an invalid type in serializable field.");
	return nullptr;
}

MonoObject* ManagedSerializableFieldDataArray::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataList::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataDictionary::GetValueBoxed(const SPtr<ManagedSerializableTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
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
	if(rtti_is_of_type<ManagedSerializableFieldDataString>(other))
	{
		auto castObj = std::static_pointer_cast<ManagedSerializableFieldDataString>(other);
		return (IsNull == true && IsNull == castObj->IsNull) || Value == castObj->Value;
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
	if(auto otherObj = rtti_cast<ManagedSerializableFieldDataObject>(other))
	{
		if(!Value && !otherObj->Value)
			return true;

		if((Value == nullptr && otherObj->Value) || (Value && !otherObj->Value))
			return false;

		return Value->Equals(*otherObj->Value);
	}

	return false;
}

bool ManagedSerializableFieldDataArray::Equals(const SPtr<ManagedSerializableFieldData>& other)
{
	if(auto otherObj = rtti_cast<ManagedSerializableFieldDataArray>(other))
	{
		if(!Value && !otherObj->Value)
			return true;

		if((!Value && otherObj->Value) || (Value && !otherObj->Value))
			return false;

		u32 oldLength = Value->GetTotalLength();
		u32 newLength = otherObj->Value->GetTotalLength();

		if(oldLength != newLength)
			return false;

		for(u32 i = 0; i < newLength; i++)
		{
			SPtr<ManagedSerializableFieldData> oldData = Value->GetFieldData(i);
			SPtr<ManagedSerializableFieldData> newData = otherObj->Value->GetFieldData(i);

			if(compareFieldData(oldData, newData))
				return false;
		}

		return true;
	}

	return false;
}

bool ManagedSerializableFieldDataList::Equals(const SPtr<ManagedSerializableFieldData>& other)
{
	if(auto otherObj = rtti_cast<ManagedSerializableFieldDataList>(other))
	{
		if(!Value && !otherObj->Value)
			return true;

		if((!Value && otherObj->Value) || (Value && !otherObj->Value))
			return false;

		u32 oldLength = Value->GetLength();
		u32 newLength = otherObj->Value->GetLength();

		if(oldLength != newLength)
			return false;

		for(u32 i = 0; i < newLength; i++)
		{
			SPtr<ManagedSerializableFieldData> oldData = Value->GetFieldData(i);
			SPtr<ManagedSerializableFieldData> newData = otherObj->Value->GetFieldData(i);

			if(compareFieldData(oldData, newData))
				return false;
		}

		return true;
	}

	return false;
}

bool ManagedSerializableFieldDataDictionary::Equals(const SPtr<ManagedSerializableFieldData>& other)
{
	if(auto otherObj = rtti_cast<ManagedSerializableFieldDataDictionary>(other))
	{
		if(!Value && !otherObj->Value)
			return true;

		if((!Value && otherObj->Value) || (Value && !otherObj->Value))
			return false;

		auto newEnumerator = otherObj->Value->GetEnumerator();
		while(newEnumerator.MoveNext())
		{
			SPtr<ManagedSerializableFieldData> key = newEnumerator.GetKey();
			if(Value->Contains(key))
			{
				if(!compareFieldData(Value->GetFieldData(key), newEnumerator.GetValue()))
					return false;
			}
			else
				return false;
		}

		auto oldEnumerator = Value->GetEnumerator();
		while(oldEnumerator.MoveNext())
		{
			SPtr<ManagedSerializableFieldData> key = oldEnumerator.GetKey();
			if(!otherObj->Value->Contains(oldEnumerator.GetKey()))
				return false;
		}

		return true;
	}

	return false;
	;
}

size_t ManagedSerializableFieldDataBool::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataChar::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataI8::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataU8::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataI16::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataU16::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataI32::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataU32::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataI64::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataU64::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataFloat::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataDouble::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataString::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataResourceRef::GetHash()
{
	return bs_hash(Value.GetUuid());
}

size_t ManagedSerializableFieldDataGameObjectRef::GetHash()
{
	return bs_hash(Value.GetInstanceId());
}

size_t ManagedSerializableFieldDataReflectableRef::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataObject::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataArray::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataList::GetHash()
{
	return bs_hash(Value);
}

size_t ManagedSerializableFieldDataDictionary::GetHash()
{
	return bs_hash(Value);
}

void ManagedSerializableFieldDataObject::Serialize()
{
	if(Value != nullptr)
		Value->Serialize();
}

void ManagedSerializableFieldDataObject::Deserialize()
{
	if(Value != nullptr)
	{
		MonoObject* managedInstance = Value->Deserialize();
		Value = ManagedSerializableObject::CreateFromExisting(managedInstance);
	}
}

void ManagedSerializableFieldDataArray::Serialize()
{
	if(Value != nullptr)
		Value->Serialize();
}

void ManagedSerializableFieldDataArray::Deserialize()
{
	if(Value != nullptr)
	{
		MonoObject* managedInstance = Value->Deserialize();
		Value = ManagedSerializableArray::CreateFromExisting(managedInstance, Value->GetTypeInfo());
	}
}

void ManagedSerializableFieldDataList::Serialize()
{
	if(Value != nullptr)
		Value->Serialize();
}

void ManagedSerializableFieldDataList::Deserialize()
{
	if(Value != nullptr)
	{
		MonoObject* managedInstance = Value->Deserialize();
		Value = ManagedSerializableList::CreateFromExisting(managedInstance, Value->GetTypeInfo());
	}
}

void ManagedSerializableFieldDataDictionary::Serialize()
{
	if(Value != nullptr)
		Value->Serialize();
}

void ManagedSerializableFieldDataDictionary::Deserialize()
{
	if(Value != nullptr)
	{
		MonoObject* managedInstance = Value->Deserialize();
		Value = ManagedSerializableDictionary::CreateFromExisting(managedInstance, Value->GetTypeInfo());
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
