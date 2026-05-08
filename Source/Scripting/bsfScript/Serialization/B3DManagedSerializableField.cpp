//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DManagedSerializableField.h"
#include "Serialization/B3DManagedTypeInfo.h"
#include "RTTI/B3DManagedSerializableFieldRTTI.h"
#include "B3DMonoUtil.h"
#include "B3DMonoManager.h"
#include "B3DScriptResourceManager.h"
#include "B3DScriptObjectWrapper.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptResourceWrapper.h"
#include "Wrappers/B3DScriptManagedResource.h"
#include "Wrappers/B3DScriptSceneObject.h"
#include "Wrappers/B3DScriptComponent.h"
#include "Wrappers/B3DScriptManagedComponent.h"
#include "Serialization/B3DManagedSerializableObject.h"
#include "Serialization/B3DManagedSerializableArray.h"
#include "Serialization/B3DManagedSerializableList.h"
#include "Serialization/B3DManagedSerializableDictionary.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Utility/B3DUtility.h"
#include "Wrappers/B3DScriptRRefBase.h"

using namespace b3d;
template <class T>
bool CompareFieldData(const T* a, const TShared<ManagedSerializableFieldData>& b)
{
	if(B3DRTTIIsOfType<T>(b))
	{
		auto castObj = std::static_pointer_cast<T>(b);
		return a->Value == castObj->Value;
	}

	return false;
}

bool CompareFieldData(const TShared<ManagedSerializableFieldData>& oldData, const TShared<ManagedSerializableFieldData>& newData, RTTIOperationContext* context)
{
	if(!oldData)
		return !newData;
	else
	{
		if(!newData)
			return false;
	}

	return oldData->Equals(newData, context);
}

bool IsPrimitiveOrEnumType(const TShared<ManagedTypeInfo>& typeInfo, ManagedPrimitiveType underlyingType)
{
	if(const auto primitiveTypeInfo = B3DRTTICast<ManagedTypeInfoPrimitive>(typeInfo.get()))
		return primitiveTypeInfo->PrimitiveType == underlyingType;
	else if(const auto enumTypeInfo = B3DRTTICast<ManagedTypeInfoEnum>(typeInfo.get()))
		return enumTypeInfo->UnderlyingType == underlyingType;

	return false;
}

ManagedSerializableFieldKey::ManagedSerializableFieldKey(u16 typeId, u16 fieldId)
	: MTypeId(typeId), MFieldId(fieldId)
{}

TShared<ManagedSerializableFieldKey> ManagedSerializableFieldKey::Create(u16 typeId, u16 fieldId)
{
	TShared<ManagedSerializableFieldKey> fieldKey = B3DMakeShared<ManagedSerializableFieldKey>(typeId, fieldId);
	return fieldKey;
}

TShared<ManagedSerializableFieldDataEntry> ManagedSerializableFieldDataEntry::Create(const TShared<ManagedSerializableFieldKey>& key, const TShared<ManagedSerializableFieldData>& value)
{
	TShared<ManagedSerializableFieldDataEntry> fieldDataEntry = B3DMakeShared<ManagedSerializableFieldDataEntry>();
	fieldDataEntry->MKey = key;
	fieldDataEntry->MValue = value;

	return fieldDataEntry;
}

TShared<ManagedSerializableFieldData> ManagedSerializableFieldData::Create(const TShared<ManagedTypeInfo>& typeInfo, MonoObject* value)
{
	return Create(typeInfo, value, true);
}

TShared<ManagedSerializableFieldData> ManagedSerializableFieldData::CreateDefault(const TShared<ManagedTypeInfo>& typeInfo)
{
	return Create(typeInfo, nullptr, false);
}

TShared<ManagedSerializableFieldData> ManagedSerializableFieldData::Create(const TShared<ManagedTypeInfo>& typeInfo, MonoObject* value, bool allowNull)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoPrimitive || typeInfo->GetTypeId() == TID_ManagedTypeInfoEnum)
	{
		ManagedPrimitiveType primitiveType = ManagedPrimitiveType::I32;

		if(auto primitiveTypeInfo = B3DRTTICast<ManagedTypeInfoPrimitive>(typeInfo.get()))
			primitiveType = primitiveTypeInfo->PrimitiveType;
		else if(auto enumTypeInfo = B3DRTTICast<ManagedTypeInfoEnum>(typeInfo.get()))
			primitiveType = enumTypeInfo->UnderlyingType;

		switch(primitiveType)
		{
		case ManagedPrimitiveType::Bool:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataBool>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::Char:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataChar>();
				if(value != nullptr)
				{
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));
					fieldData->Value32 = fieldData->Value;
				}

				return fieldData;
			}
		case ManagedPrimitiveType::I8:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataI8>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::U8:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataU8>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::I16:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataI16>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::U16:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataU16>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::I32:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataI32>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::U32:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataU32>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::I64:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataI64>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::U64:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataU64>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::Float:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataFloat>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::Double:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataDouble>();
				if(value != nullptr)
					memcpy(&fieldData->Value, MonoUtil::Unbox(value), sizeof(fieldData->Value));

				return fieldData;
			}
		case ManagedPrimitiveType::String:
			{
				MonoString* strVal = (MonoString*)(value);

				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataString>();
				if(strVal != nullptr)
				{
					fieldData->Value = MonoUtil::MonoToWString(strVal);

					fieldData->Value32 = U32String(fieldData->Value.size(), '0');
					for(size_t i = 0; i < fieldData->Value.size(); ++i)
						fieldData->Value32[i] = fieldData->Value[i];
				}
				else
					fieldData->IsNull = allowNull;

				return fieldData;
			}
		default:
			break;
		}
	}
	else if(typeInfo->GetTypeId() == TID_ManagedTypeInfoReference)
	{
		auto refTypeInfo = std::static_pointer_cast<ManagedTypeInfoReference>(typeInfo);
		switch(refTypeInfo->ReferenceType)
		{
		case ManagedReferenceType::SceneObject:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataGameObjectRef>();

				if(value != nullptr)
				{
					ScriptSceneObject* scriptSceneObject = ScriptSceneObject::GetScriptObjectWrapper(value);
					fieldData->Value = scriptSceneObject->GetNativeObjectAsHandle();
				}

				return fieldData;
			}
		case ManagedReferenceType::ManagedComponentBase:
		case ManagedReferenceType::ManagedComponent:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataGameObjectRef>();

				if(value != nullptr)
				{
					ScriptManagedComponent* scriptComponent = ScriptManagedComponent::GetScriptObjectWrapper(value);
					fieldData->Value = scriptComponent->GetBaseNativeObjectAsHandle();
				}

				return fieldData;
			}
		case ManagedReferenceType::BuiltinComponentBase:
		case ManagedReferenceType::BuiltinComponent:
			{
				const ScriptTypeMetaData* const scriptWrapperObjectMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(refTypeInfo->TypeRTTIId);
				if(scriptWrapperObjectMetaData == nullptr)
					return nullptr;

				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataGameObjectRef>();

				if(value != nullptr)
				{
					ScriptGameObjectWrapper* const scriptGameObjectWrapper = ScriptGameObjectWrapper::GetScriptObjectWrapper(*scriptWrapperObjectMetaData, value);
					fieldData->Value = scriptGameObjectWrapper->GetBaseNativeObjectAsHandle();
				}

				return fieldData;
			}
		case ManagedReferenceType::ManagedResourceBase:
		case ManagedReferenceType::ManagedResource:
			{
				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataResourceRef>();

				if(value != nullptr)
				{
					ScriptResourceWrapper* const scriptResource = ScriptManagedResource::GetScriptObjectWrapper(value);
					fieldData->Value = scriptResource->GetBaseNativeObjectAsHandle();
				}

				return fieldData;
			}
		case ManagedReferenceType::BuiltinResourceBase:
		case ManagedReferenceType::BuiltinResource:
			{
				const ScriptTypeMetaData* const scriptWrapperObjectMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(refTypeInfo->TypeRTTIId);
				if(scriptWrapperObjectMetaData == nullptr)
					return nullptr;

				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataResourceRef>();

				if(value != nullptr)
				{
					ScriptResourceWrapper* scriptResource = ScriptResourceWrapper::GetScriptObjectWrapper(*scriptWrapperObjectMetaData, value);
					fieldData->Value = scriptResource->GetBaseNativeObjectAsHandle();
				}

				return fieldData;
			}
		case ManagedReferenceType::ReflectableObject:
			{
				const ScriptTypeMetaData* scriptWrapperMetaData = ScriptAssemblyManager::Instance().GetScriptWrapperMetaData(refTypeInfo->TypeRTTIId);
				if(scriptWrapperMetaData == nullptr)
					return nullptr;

				auto fieldData = B3DMakeShared<ManagedSerializableFieldDataReflectableRef>();

				if(value != nullptr)
				{
					const ScriptReflectableWrapper* const scriptReflectableWrapper = ScriptReflectableWrapper::GetScriptObjectWrapper(*scriptWrapperMetaData, value);
					if(!B3D_ENSURE(scriptReflectableWrapper != nullptr))
						return nullptr;

					fieldData->Value = scriptReflectableWrapper->GetBaseNativeObjectAsShared();
				}

				return fieldData;
			}
		default:
			break;
		}
	}
	else if(typeInfo->GetTypeId() == TID_ManagedTypeInfoResourceReference)
	{
		auto fieldData = B3DMakeShared<ManagedSerializableFieldDataResourceRef>();

		if(value != nullptr)
		{
			ScriptRRefBase* scriptRRefBase = ScriptRRefBase::GetScriptObjectWrapper(value);
			fieldData->Value = scriptRRefBase->GetNativeObject();
		}

		return fieldData;
	}
	else if(typeInfo->GetTypeId() == TID_ManagedTypeInfoObject)
	{
		auto fieldData = B3DMakeShared<ManagedSerializableFieldDataObject>();
		if(value != nullptr)
			fieldData->Value = ManagedSerializableObject::CreateFromExisting(value);
		else if(!allowNull)
			fieldData->Value = ManagedSerializableObject::CreateNew(std::static_pointer_cast<ManagedTypeInfoObject>(typeInfo));

		return fieldData;
	}
	else if(typeInfo->GetTypeId() == TID_ManagedTypeInfoArray)
	{
		TShared<ManagedTypeInfoArray> arrayTypeInfo = std::static_pointer_cast<ManagedTypeInfoArray>(typeInfo);

		auto fieldData = B3DMakeShared<ManagedSerializableFieldDataArray>();
		if(value != nullptr)
			fieldData->Value = ManagedSerializableArray::CreateFromExisting(value, arrayTypeInfo);
		else if(!allowNull)
		{
			Vector<u32> sizes(arrayTypeInfo->Rank, 0);
			fieldData->Value = ManagedSerializableArray::CreateNew(arrayTypeInfo, sizes);
		}

		return fieldData;
	}
	else if(typeInfo->GetTypeId() == TID_ManagedTypeInfoList)
	{
		TShared<ManagedTypeInfoList> listTypeInfo = std::static_pointer_cast<ManagedTypeInfoList>(typeInfo);

		auto fieldData = B3DMakeShared<ManagedSerializableFieldDataList>();
		if(value != nullptr)
			fieldData->Value = ManagedSerializableList::CreateFromExisting(value, listTypeInfo);
		else if(!allowNull)
			fieldData->Value = ManagedSerializableList::CreateNew(listTypeInfo, 0);

		return fieldData;
	}
	else if(typeInfo->GetTypeId() == TID_ManagedTypeInfoDictionary)
	{
		TShared<ManagedTypeInfoDictionary> dictTypeInfo = std::static_pointer_cast<ManagedTypeInfoDictionary>(typeInfo);

		auto fieldData = B3DMakeShared<ManagedSerializableFieldDataDictionary>();
		if(value != nullptr)
			fieldData->Value = ManagedSerializableDictionary::CreateFromExisting(value, dictTypeInfo);
		else if(!allowNull)
			fieldData->Value = ManagedSerializableDictionary::CreateNew(dictTypeInfo);

		return fieldData;
	}

	return nullptr;
}

void* ManagedSerializableFieldDataBool::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::Bool))
		return &Value;

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataChar::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::Char))
		return &Value;

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataI8::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::I8))
		return &Value;

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataU8::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::U8))
		return &Value;

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataI16::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::I16))
		return &Value;

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataU16::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::U16))
		return &Value;

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataI32::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::I32))
		return &Value;

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataU32::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::U32))
		return &Value;

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataI64::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::I64))
		return &Value;

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataU64::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::U64))
		return &Value;

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataFloat::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoPrimitive)
	{
		auto primitiveTypeInfo = std::static_pointer_cast<ManagedTypeInfoPrimitive>(typeInfo);
		if(primitiveTypeInfo->PrimitiveType == ManagedPrimitiveType::Float)
			return &Value;
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataDouble::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoPrimitive)
	{
		auto primitiveTypeInfo = std::static_pointer_cast<ManagedTypeInfoPrimitive>(typeInfo);
		if(primitiveTypeInfo->PrimitiveType == ManagedPrimitiveType::Double)
			return &Value;
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataString::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoPrimitive)
	{
		auto primitiveTypeInfo = std::static_pointer_cast<ManagedTypeInfoPrimitive>(typeInfo);
		if(primitiveTypeInfo->PrimitiveType == ManagedPrimitiveType::String)
		{
			if(!IsNull)
				return MonoUtil::WstringToMono(Value);
			else
				return nullptr;
		}
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataResourceRef::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoReference)
	{
		const auto refTypeInfo = std::static_pointer_cast<ManagedTypeInfoReference>(typeInfo);

		if(!Value.IsLoaded())
			return nullptr;

		return ScriptResourceWrapper::GetOrCreateScriptObject(Value);
	}
	else if(typeInfo->GetTypeId() == TID_ManagedTypeInfoResourceReference)
	{
		const auto refTypeInfo = std::static_pointer_cast<ManagedTypeInfoResourceReference>(typeInfo);

		::MonoClass* resourceRRefClass = nullptr;
		if(refTypeInfo->ResourceType)
		{
			if(!typeInfo->IsTypeLoaded())
				return nullptr;

			resourceRRefClass = typeInfo->GetMonoClass();
			if(resourceRRefClass == nullptr)
				return nullptr;
		}

		// Note: Each reference ref ends up creating its own object instance. Perhaps share the same instance between
		// all references to the same resource?

		return ScriptRRefBase::CreateScriptObject(Value, resourceRRefClass);
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataGameObjectRef::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoReference)
	{
		if(Value)
			return ScriptGameObject::GetOrCreateScriptObject(Value);

		return nullptr;
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataReflectableRef::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoReference)
	{
		if(!Value)
			return nullptr;

		return ScriptReflectableWrapper::GetOrCreateScriptObject(Value);
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataObject::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoObject)
	{
		auto objectTypeInfo = std::static_pointer_cast<ManagedTypeInfoObject>(typeInfo);

		if(Value != nullptr)
		{
			if(objectTypeInfo->IsValueType)
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

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataArray::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoArray)
	{
		auto objectTypeInfo = std::static_pointer_cast<ManagedTypeInfoArray>(typeInfo);

		if(Value != nullptr)
			return Value->GetManagedInstance();

		return nullptr;
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataList::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoList)
	{
		auto listTypeInfo = std::static_pointer_cast<ManagedTypeInfoList>(typeInfo);

		if(Value != nullptr)
			return Value->GetManagedInstance();

		return nullptr;
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

void* ManagedSerializableFieldDataDictionary::GetValue(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoDictionary)
	{
		auto dictionaryTypeInfo = std::static_pointer_cast<ManagedTypeInfoDictionary>(typeInfo);

		if(Value != nullptr)
			return Value->GetManagedInstance();

		return nullptr;
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataBool::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::Bool))
		return MonoUtil::Box(MonoUtil::GetBoolClass(), &Value);

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataChar::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::Char))
		return MonoUtil::Box(MonoUtil::GetCharClass(), &Value);

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataI8::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::I8))
		return MonoUtil::Box(MonoUtil::GetSByteClass(), &Value);

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataU8::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::U8))
		return MonoUtil::Box(MonoUtil::GetByteClass(), &Value);

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataI16::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::I16))
		return MonoUtil::Box(MonoUtil::GetInt16Class(), &Value);

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataU16::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::U16))
		return MonoUtil::Box(MonoUtil::GetUint16Class(), &Value);

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataI32::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::I32))
		return MonoUtil::Box(MonoUtil::GetInt32Class(), &Value);

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataU32::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::U32))
		return MonoUtil::Box(MonoUtil::GetUint32Class(), &Value);

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataI64::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::I64))
		return MonoUtil::Box(MonoUtil::GetInt64Class(), &Value);

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataU64::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(IsPrimitiveOrEnumType(typeInfo, ManagedPrimitiveType::U64))
		return MonoUtil::Box(MonoUtil::GetUint64Class(), &Value);

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataFloat::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoPrimitive)
	{
		auto primitiveTypeInfo = std::static_pointer_cast<ManagedTypeInfoPrimitive>(typeInfo);
		if(primitiveTypeInfo->PrimitiveType == ManagedPrimitiveType::Float)
			return MonoUtil::Box(MonoUtil::GetFloatClass(), &Value);
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataDouble::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoPrimitive)
	{
		auto primitiveTypeInfo = std::static_pointer_cast<ManagedTypeInfoPrimitive>(typeInfo);
		if(primitiveTypeInfo->PrimitiveType == ManagedPrimitiveType::Double)
			return MonoUtil::Box(MonoUtil::GetDoubleClass(), &Value);
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataString::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataResourceRef::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataGameObjectRef::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataReflectableRef::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataObject::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	if(typeInfo->GetTypeId() == TID_ManagedTypeInfoObject)
	{
		auto objectTypeInfo = std::static_pointer_cast<ManagedTypeInfoObject>(typeInfo);

		if(Value != nullptr)
			return Value->GetManagedInstance();

		return nullptr;
	}

	B3D_LOG(Fatal, LogScript, "Requesting an invalid type in serializable field.");
}

MonoObject* ManagedSerializableFieldDataArray::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataList::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

MonoObject* ManagedSerializableFieldDataDictionary::GetValueBoxed(const TShared<ManagedTypeInfo>& typeInfo)
{
	return (MonoObject*)GetValue(typeInfo);
}

bool ManagedSerializableFieldDataBool::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataChar::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataI8::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataU8::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataI16::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataU16::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataI32::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataU32::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataI64::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataU64::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataFloat::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataDouble::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataString::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	if(B3DRTTIIsOfType<ManagedSerializableFieldDataString>(other))
	{
		auto castObj = std::static_pointer_cast<ManagedSerializableFieldDataString>(other);
		return (IsNull == true && IsNull == castObj->IsNull) || Value == castObj->Value;
	}

	return false;
}

bool ManagedSerializableFieldDataResourceRef::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataGameObjectRef::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	const TShared<ManagedSerializableFieldDataGameObjectRef>& exactOther = B3DRTTICast<ManagedSerializableFieldDataGameObjectRef>(other);
	if(exactOther != nullptr)
	{
		UUID myId = Value.GetId();
		UUID otherId = exactOther->Value.GetId();

		// Remap UUIDs if remapping is provided
		if(RTTIOperationEngineContext* serializationContext = B3DRTTICast<RTTIOperationEngineContext>(context))
		{
			if(auto found = serializationContext->GameObjectIdRemapping.find(myId); found != serializationContext->GameObjectIdRemapping.end())
				myId = found->second;

			if(auto found = serializationContext->GameObjectIdRemapping.find(otherId); found != serializationContext->GameObjectIdRemapping.end())
				otherId = found->second;

			return myId == otherId;
		}
	}

	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataReflectableRef::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	return CompareFieldData(this, other);
}

bool ManagedSerializableFieldDataObject::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	if(auto otherObj = B3DRTTICast<ManagedSerializableFieldDataObject>(other))
	{
		if(!Value && !otherObj->Value)
			return true;

		if((Value == nullptr && otherObj->Value) || (Value && !otherObj->Value))
			return false;

		return Value->Equals(*otherObj->Value, context);
	}

	return false;
}

bool ManagedSerializableFieldDataArray::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	if(auto otherObj = B3DRTTICast<ManagedSerializableFieldDataArray>(other))
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
			TShared<ManagedSerializableFieldData> oldData = Value->GetFieldData(i);
			TShared<ManagedSerializableFieldData> newData = otherObj->Value->GetFieldData(i);

			if(CompareFieldData(oldData, newData, context))
				return false;
		}

		return true;
	}

	return false;
}

bool ManagedSerializableFieldDataList::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	if(auto otherObj = B3DRTTICast<ManagedSerializableFieldDataList>(other))
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
			TShared<ManagedSerializableFieldData> oldData = Value->GetFieldData(i);
			TShared<ManagedSerializableFieldData> newData = otherObj->Value->GetFieldData(i);

			if(CompareFieldData(oldData, newData, context))
				return false;
		}

		return true;
	}

	return false;
}

bool ManagedSerializableFieldDataDictionary::Equals(const TShared<ManagedSerializableFieldData>& other, RTTIOperationContext* context)
{
	if(auto otherObj = B3DRTTICast<ManagedSerializableFieldDataDictionary>(other))
	{
		if(!Value && !otherObj->Value)
			return true;

		if((!Value && otherObj->Value) || (Value && !otherObj->Value))
			return false;

		auto newEnumerator = otherObj->Value->GetEnumerator();
		while(newEnumerator.MoveNext())
		{
			TShared<ManagedSerializableFieldData> key = newEnumerator.GetKey();
			if(Value->Contains(key))
			{
				if(!CompareFieldData(Value->GetFieldData(key), newEnumerator.GetValue(), context))
					return false;
			}
			else
				return false;
		}

		auto oldEnumerator = Value->GetEnumerator();
		while(oldEnumerator.MoveNext())
		{
			TShared<ManagedSerializableFieldData> key = oldEnumerator.GetKey();
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
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataChar::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataI8::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataU8::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataI16::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataU16::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataI32::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataU32::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataI64::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataU64::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataFloat::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataDouble::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataString::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataResourceRef::GetHash()
{
	return B3DHash(Value.GetId());
}

size_t ManagedSerializableFieldDataGameObjectRef::GetHash()
{
	return B3DHash(Value.GetId());
}

size_t ManagedSerializableFieldDataReflectableRef::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataObject::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataArray::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataList::GetHash()
{
	return B3DHash(Value);
}

size_t ManagedSerializableFieldDataDictionary::GetHash()
{
	return B3DHash(Value);
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

RTTIType* ManagedSerializableFieldKey::GetRttiStatic()
{
	return ManagedSerializableFieldKeyRTTI::Instance();
}

RTTIType* ManagedSerializableFieldKey::GetRtti() const
{
	return ManagedSerializableFieldKey::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldData::GetRttiStatic()
{
	return ManagedSerializableFieldDataRTTI::Instance();
}

RTTIType* ManagedSerializableFieldData::GetRtti() const
{
	return ManagedSerializableFieldData::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataEntry::GetRttiStatic()
{
	return ManagedSerializableFieldDataEntryRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataEntry::GetRtti() const
{
	return ManagedSerializableFieldDataEntry::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataBool::GetRttiStatic()
{
	return ManagedSerializableFieldDataBoolRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataBool::GetRtti() const
{
	return ManagedSerializableFieldDataBool::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataChar::GetRttiStatic()
{
	return ManagedSerializableFieldDataCharRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataChar::GetRtti() const
{
	return ManagedSerializableFieldDataChar::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataI8::GetRttiStatic()
{
	return ManagedSerializableFieldDataI8RTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataI8::GetRtti() const
{
	return ManagedSerializableFieldDataI8::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataU8::GetRttiStatic()
{
	return ManagedSerializableFieldDataU8RTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataU8::GetRtti() const
{
	return ManagedSerializableFieldDataU8::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataI16::GetRttiStatic()
{
	return ManagedSerializableFieldDataI16RTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataI16::GetRtti() const
{
	return ManagedSerializableFieldDataI16::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataU16::GetRttiStatic()
{
	return ManagedSerializableFieldDataU16RTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataU16::GetRtti() const
{
	return ManagedSerializableFieldDataU16::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataI32::GetRttiStatic()
{
	return ManagedSerializableFieldDataI32RTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataI32::GetRtti() const
{
	return ManagedSerializableFieldDataI32::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataU32::GetRttiStatic()
{
	return ManagedSerializableFieldDataU32RTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataU32::GetRtti() const
{
	return ManagedSerializableFieldDataU32::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataI64::GetRttiStatic()
{
	return ManagedSerializableFieldDataI64RTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataI64::GetRtti() const
{
	return ManagedSerializableFieldDataI64::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataU64::GetRttiStatic()
{
	return ManagedSerializableFieldDataU64RTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataU64::GetRtti() const
{
	return ManagedSerializableFieldDataU64::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataFloat::GetRttiStatic()
{
	return ManagedSerializableFieldDataFloatRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataFloat::GetRtti() const
{
	return ManagedSerializableFieldDataFloat::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataDouble::GetRttiStatic()
{
	return ManagedSerializableFieldDataDoubleRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataDouble::GetRtti() const
{
	return ManagedSerializableFieldDataDouble::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataString::GetRttiStatic()
{
	return ManagedSerializableFieldDataStringRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataString::GetRtti() const
{
	return ManagedSerializableFieldDataString::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataResourceRef::GetRttiStatic()
{
	return ManagedSerializableFieldDataResourceRefRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataResourceRef::GetRtti() const
{
	return ManagedSerializableFieldDataResourceRef::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataGameObjectRef::GetRttiStatic()
{
	return ManagedSerializableFieldDataGameObjectRefRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataGameObjectRef::GetRtti() const
{
	return ManagedSerializableFieldDataGameObjectRef::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataReflectableRef::GetRttiStatic()
{
	return ManagedSerializableFieldDataReflectableRefRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataReflectableRef::GetRtti() const
{
	return ManagedSerializableFieldDataReflectableRef::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataObject::GetRttiStatic()
{
	return ManagedSerializableFieldDataObjectRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataObject::GetRtti() const
{
	return ManagedSerializableFieldDataObject::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataArray::GetRttiStatic()
{
	return ManagedSerializableFieldDataArrayRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataArray::GetRtti() const
{
	return ManagedSerializableFieldDataArray::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataList::GetRttiStatic()
{
	return ManagedSerializableFieldDataListRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataList::GetRtti() const
{
	return ManagedSerializableFieldDataList::GetRttiStatic();
}

RTTIType* ManagedSerializableFieldDataDictionary::GetRttiStatic()
{
	return ManagedSerializableFieldDataDictionaryRTTI::Instance();
}

RTTIType* ManagedSerializableFieldDataDictionary::GetRtti() const
{
	return ManagedSerializableFieldDataDictionary::GetRttiStatic();
}
