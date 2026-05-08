//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DManagedSerializableDelta.h"
#include "Serialization/B3DManagedSerializableObject.h"
#include "Serialization/B3DManagedTypeInfo.h"
#include "Serialization/B3DManagedSerializableField.h"
#include "Serialization/B3DManagedSerializableArray.h"
#include "Serialization/B3DManagedSerializableList.h"
#include "Serialization/B3DManagedSerializableDictionary.h"
#include "RTTI/B3DManagedSerializableDeltaRTTI.h"

using namespace b3d;
ManagedSerializableDelta::ModifiedField::ModifiedField(const TShared<ManagedTypeInfo>& parentType, const TShared<ManagedMemberInfo>& fieldType, const TShared<ManagedSerializableDelta::Modification>& modification)
	: ParentType(parentType), FieldType(fieldType), Modification(modification)
{}

RTTIType* ManagedSerializableDelta::ModifiedField::GetRttiStatic()
{
	return ModifiedFieldRTTI::Instance();
}

RTTIType* ManagedSerializableDelta::ModifiedField::GetRtti() const
{
	return GetRttiStatic();
}

ManagedSerializableDelta::ModifiedArrayEntry::ModifiedArrayEntry(u32 idx, const TShared<ManagedSerializableDelta::Modification>& modification)
	: Idx(idx), Modification(modification)
{}

RTTIType* ManagedSerializableDelta::ModifiedArrayEntry::GetRttiStatic()
{
	return ModifiedArrayEntryRTTI::Instance();
}

RTTIType* ManagedSerializableDelta::ModifiedArrayEntry::GetRtti() const
{
	return GetRttiStatic();
}

ManagedSerializableDelta::ModifiedDictionaryEntry::ModifiedDictionaryEntry(
	const TShared<ManagedSerializableFieldData>& key, const TShared<ManagedSerializableDelta::Modification>& modification)
	: Key(key), Modification(modification)
{}

RTTIType* ManagedSerializableDelta::ModifiedDictionaryEntry::GetRttiStatic()
{
	return ModifiedDictionaryEntryRTTI::Instance();
}

RTTIType* ManagedSerializableDelta::ModifiedDictionaryEntry::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* ManagedSerializableDelta::Modification::GetRttiStatic()
{
	return ModificationRTTI::Instance();
}

RTTIType* ManagedSerializableDelta::Modification::GetRtti() const
{
	return GetRttiStatic();
}

TShared<ManagedSerializableDelta::ModifiedObject> ManagedSerializableDelta::ModifiedObject::Create()
{
	return B3DMakeShared<ModifiedObject>();
}

RTTIType* ManagedSerializableDelta::ModifiedObject::GetRttiStatic()
{
	return ModifiedObjectRTTI::Instance();
}

RTTIType* ManagedSerializableDelta::ModifiedObject::GetRtti() const
{
	return GetRttiStatic();
}

TShared<ManagedSerializableDelta::ModifiedArray> ManagedSerializableDelta::ModifiedArray::Create()
{
	return B3DMakeShared<ModifiedArray>();
}

RTTIType* ManagedSerializableDelta::ModifiedArray::GetRttiStatic()
{
	return ModifiedArrayRTTI::Instance();
}

RTTIType* ManagedSerializableDelta::ModifiedArray::GetRtti() const
{
	return GetRttiStatic();
}

TShared<ManagedSerializableDelta::ModifiedDictionary> ManagedSerializableDelta::ModifiedDictionary::Create()
{
	return B3DMakeShared<ModifiedDictionary>();
}

RTTIType* ManagedSerializableDelta::ModifiedDictionary::GetRttiStatic()
{
	return ModifiedDictionaryRTTI::Instance();
}

RTTIType* ManagedSerializableDelta::ModifiedDictionary::GetRtti() const
{
	return GetRttiStatic();
}

ManagedSerializableDelta::ModifiedEntry::ModifiedEntry(const TShared<ManagedSerializableFieldData>& value)
	: Value(value)
{}

TShared<ManagedSerializableDelta::ModifiedEntry> ManagedSerializableDelta::ModifiedEntry::Create(const TShared<ManagedSerializableFieldData>& value)
{
	return B3DMakeShared<ModifiedEntry>(value);
}

RTTIType* ManagedSerializableDelta::ModifiedEntry::GetRttiStatic()
{
	return ModifiedEntryRTTI::Instance();
}

RTTIType* ManagedSerializableDelta::ModifiedEntry::GetRtti() const
{
	return GetRttiStatic();
}

ManagedSerializableDelta::ManagedSerializableDelta()
	: mModificationRoot(ModifiedObject::Create())
{
}

TShared<ManagedSerializableDelta> ManagedSerializableDelta::Create(const ManagedSerializableObject* original, const ManagedSerializableObject* modified, RTTIOperationContext* context)
{
	B3D_ASSERT(original != nullptr && modified != nullptr);

	TShared<ManagedObjectInfo> oldObjInfo = original->GetObjectInfo();
	TShared<ManagedObjectInfo> newObjInfo = modified->GetObjectInfo();

	if(!oldObjInfo->TypeInfo->Matches(newObjInfo->TypeInfo))
		return nullptr;

	TShared<ManagedSerializableDelta> output = B3DMakeShared<ManagedSerializableDelta>();
	TShared<ModifiedObject> modifications = output->GenerateObjectDelta(original, modified, context);

	if(modifications != nullptr)
	{
		output->mModificationRoot->Entries = modifications->Entries;
		return output;
	}

	return nullptr;
}

TShared<ManagedSerializableDelta::ModifiedObject> ManagedSerializableDelta::GenerateObjectDelta(const ManagedSerializableObject* original, const ManagedSerializableObject* modified, RTTIOperationContext* context)
{
	TShared<ModifiedObject> output = nullptr;

	TShared<ManagedObjectInfo> curObjInfo = modified->GetObjectInfo();
	while(curObjInfo != nullptr)
	{
		for(auto& member : curObjInfo->Members)
		{
			if(!member->IsSerializable())
				continue;

			u32 fieldTypeId = member->TypeInfo->GetTypeId();

			TShared<ManagedSerializableFieldData> oldData = original->GetFieldData(member);
			TShared<ManagedSerializableFieldData> newData = modified->GetFieldData(member);
			TShared<Modification> newMod = GenerateFieldDelta(oldData, newData, fieldTypeId, context);

			if(newMod != nullptr)
			{
				if(output == nullptr)
					output = ModifiedObject::Create();

				output->Entries.push_back(ModifiedField(curObjInfo->TypeInfo, member, newMod));
			}
		}

		curObjInfo = curObjInfo->BaseClass;
	}

	return output;
}

TShared<ManagedSerializableDelta::Modification> ManagedSerializableDelta::GenerateFieldDelta(const TShared<ManagedSerializableFieldData>& original, const TShared<ManagedSerializableFieldData>& modified, u32 entryTypeId, RTTIOperationContext* context)
{
	bool isPrimitive = entryTypeId == TID_ManagedTypeInfoPrimitive ||
		entryTypeId == TID_ManagedTypeInfoReference ||
		entryTypeId == TID_ManagedTypeInfoEnum ||
		entryTypeId == TID_ManagedTypeInfoResourceReference;

	// It's possible the field data is null if the class structure changed (i.e. new field was added that is not present
	// in serialized data). Check for this case first to ensure field data is valid for the remainder of the method.
	if(original == nullptr)
	{
		if(modified == nullptr)
			return nullptr;
		else
			return ModifiedEntry::Create(modified);
	}
	else
	{
		if(modified == nullptr)
			return nullptr;
	}

	TShared<Modification> newMod = nullptr;
	if(isPrimitive)
	{
		if(!original->Equals(modified, context))
			newMod = ModifiedEntry::Create(modified);
	}
	else
	{
		switch(entryTypeId)
		{
		case TID_ManagedTypeInfoObject:
			{
				TShared<ManagedSerializableFieldDataObject> oldObjData =
					std::static_pointer_cast<ManagedSerializableFieldDataObject>(original);
				TShared<ManagedSerializableFieldDataObject> newObjData =
					std::static_pointer_cast<ManagedSerializableFieldDataObject>(modified);

				if(oldObjData->Value != nullptr && newObjData->Value != nullptr)
				{
					newMod = GenerateObjectDelta(oldObjData->Value.get(), newObjData->Value.get(), context);
				}
				else if(oldObjData->Value == nullptr && newObjData->Value == nullptr)
				{
					// No change
				}
				else // We either record null if new value is null, or the entire object if old value is null
				{
					newMod = ModifiedEntry::Create(modified);
				}
			}
			break;
		case TID_ManagedTypeInfoArray:
			{
				TShared<ManagedSerializableFieldDataArray> oldArrayData =
					std::static_pointer_cast<ManagedSerializableFieldDataArray>(original);
				TShared<ManagedSerializableFieldDataArray> newArrayData =
					std::static_pointer_cast<ManagedSerializableFieldDataArray>(modified);

				if(oldArrayData->Value != nullptr && newArrayData->Value != nullptr)
				{
					u32 oldLength = oldArrayData->Value->GetTotalLength();
					u32 newLength = newArrayData->Value->GetTotalLength();

					TShared<ModifiedArray> arrayMods = nullptr;
					for(u32 i = 0; i < newLength; i++)
					{
						TShared<Modification> arrayElemMod = nullptr;

						TShared<ManagedSerializableFieldData> newArrayElem = newArrayData->Value->GetFieldData(i);
						if(i < oldLength)
						{
							TShared<ManagedSerializableFieldData> oldArrayElem = oldArrayData->Value->GetFieldData(i);

							u32 arrayElemTypeId = newArrayData->Value->GetTypeInfo()->ElementType->GetTypeId();
							arrayElemMod = GenerateFieldDelta(oldArrayElem, newArrayElem, arrayElemTypeId, context);
						}
						else
						{
							arrayElemMod = ModifiedEntry::Create(newArrayElem);
						}

						if(arrayElemMod != nullptr)
						{
							if(arrayMods == nullptr)
								arrayMods = ModifiedArray::Create();

							arrayMods->Entries.push_back(ModifiedArrayEntry(i, arrayElemMod));
						}
					}

					if(oldLength != newLength)
					{
						if(arrayMods == nullptr)
							arrayMods = ModifiedArray::Create();
					}

					if(arrayMods != nullptr)
					{
						arrayMods->OrigSizes = oldArrayData->Value->GetLengths();
						arrayMods->NewSizes = newArrayData->Value->GetLengths();
					}

					newMod = arrayMods;
				}
				else if(oldArrayData->Value == nullptr && newArrayData->Value == nullptr)
				{
					// No change
				}
				else // We either record null if new value is null, or the entire array if old value is null
				{
					newMod = ModifiedEntry::Create(modified);
				}
			}
			break;
		case TID_ManagedTypeInfoList:
			{
				TShared<ManagedSerializableFieldDataList> oldListData =
					std::static_pointer_cast<ManagedSerializableFieldDataList>(original);
				TShared<ManagedSerializableFieldDataList> newListData =
					std::static_pointer_cast<ManagedSerializableFieldDataList>(modified);

				if(oldListData->Value != nullptr && newListData->Value != nullptr)
				{
					u32 oldLength = oldListData->Value->GetLength();
					u32 newLength = newListData->Value->GetLength();

					TShared<ModifiedArray> listMods = nullptr;
					for(u32 i = 0; i < newLength; i++)
					{
						TShared<Modification> listElemMod = nullptr;

						TShared<ManagedSerializableFieldData> newListElem = newListData->Value->GetFieldData(i);
						if(i < oldLength)
						{
							TShared<ManagedSerializableFieldData> oldListElem = oldListData->Value->GetFieldData(i);

							u32 arrayElemTypeId = newListData->Value->GetTypeInfo()->ElementType->GetTypeId();
							listElemMod = GenerateFieldDelta(oldListElem, newListElem, arrayElemTypeId, context);
						}
						else
						{
							listElemMod = ModifiedEntry::Create(newListElem);
						}

						if(listElemMod != nullptr)
						{
							if(listMods == nullptr)
								listMods = ModifiedArray::Create();

							listMods->Entries.push_back(ModifiedArrayEntry(i, listElemMod));
						}
					}

					if(oldLength != newLength)
					{
						if(listMods == nullptr)
							listMods = ModifiedArray::Create();
					}

					if(listMods != nullptr)
					{
						listMods->OrigSizes.push_back(oldLength);
						listMods->NewSizes.push_back(newLength);
					}

					newMod = listMods;
				}
				else if(oldListData->Value == nullptr && newListData->Value == nullptr)
				{
					// No change
				}
				else // We either record null if new value is null, or the entire list if old value is null
				{
					newMod = ModifiedEntry::Create(modified);
				}
			}
			break;
		case TID_ManagedTypeInfoDictionary:
			{
				TShared<ManagedSerializableFieldDataDictionary> oldDictData =
					std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(original);
				TShared<ManagedSerializableFieldDataDictionary> newDictData =
					std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(modified);

				if(oldDictData->Value != nullptr && newDictData->Value != nullptr)
				{
					TShared<ModifiedDictionary> dictMods = nullptr;

					auto newEnumerator = newDictData->Value->GetEnumerator();
					while(newEnumerator.MoveNext())
					{
						TShared<Modification> dictElemMod = nullptr;

						TShared<ManagedSerializableFieldData> key = newEnumerator.GetKey();
						if(oldDictData->Value->Contains(key))
						{
							u32 dictElemTypeId = newDictData->Value->GetTypeInfo()->ValueType->GetTypeId();

							dictElemMod = GenerateFieldDelta(oldDictData->Value->GetFieldData(key), newEnumerator.GetValue(), dictElemTypeId, context);
						}
						else
						{
							dictElemMod = ModifiedEntry::Create(newEnumerator.GetValue());
						}

						if(dictElemMod != nullptr)
						{
							if(dictMods == nullptr)
								dictMods = ModifiedDictionary::Create();

							dictMods->Entries.push_back(ModifiedDictionaryEntry(key, dictElemMod));
						}
					}

					auto oldEnumerator = oldDictData->Value->GetEnumerator();
					while(oldEnumerator.MoveNext())
					{
						TShared<ManagedSerializableFieldData> key = oldEnumerator.GetKey();
						if(!newDictData->Value->Contains(oldEnumerator.GetKey()))
						{
							if(dictMods == nullptr)
								dictMods = ModifiedDictionary::Create();

							dictMods->Removed.push_back(key);
						}
					}

					newMod = dictMods;
				}
				else if(oldDictData->Value == nullptr && newDictData->Value == nullptr)
				{
					// No change
				}
				else // We either record null if new value is null, or the entire dictionary if old value is null
				{
					newMod = ModifiedEntry::Create(modified);
				}
			}
			break;
		default:
			B3D_ASSERT(false); // Invalid type
			break;
		}
	}

	return newMod;
}

void ManagedSerializableDelta::Apply(const TShared<ManagedSerializableObject>& object)
{
	ApplyObjectDelta(mModificationRoot, object);
}

TShared<ManagedSerializableFieldData> ManagedSerializableDelta::ApplyObjectDelta(const TShared<ModifiedObject>& delta, const TShared<ManagedSerializableObject>& object)
{
	TShared<ManagedObjectInfo> objInfo = object->GetObjectInfo();
	for(auto& modEntry : delta->Entries)
	{
		TShared<ManagedMemberInfo> fieldType = modEntry.FieldType;
		TShared<ManagedTypeInfo> typeInfo = modEntry.ParentType;

		TShared<ManagedMemberInfo> matchingFieldInfo = objInfo->FindMatchingField(fieldType, typeInfo);
		if(matchingFieldInfo == nullptr)
			continue; // Field no longer exists in the type

		TShared<ManagedSerializableFieldData> origData = object->GetFieldData(matchingFieldInfo);

		TShared<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, matchingFieldInfo->TypeInfo, origData);
		if(newData != nullptr)
			object->SetFieldData(matchingFieldInfo, newData);
	}

	return nullptr;
}

TShared<ManagedSerializableFieldData> ManagedSerializableDelta::ApplyArrayDelta(const TShared<ModifiedArray>& delta, const TShared<ManagedSerializableArray>& object)
{
	bool needsResize = false;

	for(u32 i = 0; i < (u32)delta->NewSizes.size(); i++)
	{
		if(delta->NewSizes[i] != object->GetLength(i))
		{
			needsResize = true;
			break;
		}
	}

	TShared<ManagedSerializableFieldData> newArray;
	if(needsResize)
	{
		object->Resize(delta->NewSizes);
		newArray = ManagedSerializableFieldData::Create(object->GetTypeInfo(), object->GetManagedInstance());
	}

	for(auto& modEntry : delta->Entries)
	{
		u32 arrayIdx = modEntry.Idx;

		TShared<ManagedSerializableFieldData> origData = object->GetFieldData(arrayIdx);
		TShared<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, object->GetTypeInfo()->ElementType, origData);

		if(newData != nullptr)
			object->SetFieldData(arrayIdx, newData);
	}

	return newArray;
}

TShared<ManagedSerializableFieldData> ManagedSerializableDelta::ApplyListDelta(const TShared<ModifiedArray>& delta, const TShared<ManagedSerializableList>& object)
{
	bool needsResize = delta->NewSizes[0] != object->GetLength();

	TShared<ManagedSerializableFieldData> newList;
	if(needsResize)
	{
		object->Resize(delta->NewSizes[0]);
		newList = ManagedSerializableFieldData::Create(object->GetTypeInfo(), object->GetManagedInstance());
	}

	for(auto& modEntry : delta->Entries)
	{
		u32 arrayIdx = modEntry.Idx;

		TShared<ManagedSerializableFieldData> origData = object->GetFieldData(arrayIdx);
		TShared<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, object->GetTypeInfo()->ElementType, origData);

		if(newData != nullptr)
			object->SetFieldData(arrayIdx, newData);
	}

	return newList;
}

TShared<ManagedSerializableFieldData> ManagedSerializableDelta::ApplyDictionaryDelta(const TShared<ModifiedDictionary>& delta, const TShared<ManagedSerializableDictionary>& object)
{
	for(auto& modEntry : delta->Entries)
	{
		TShared<ManagedSerializableFieldData> key = modEntry.Key;

		TShared<ManagedSerializableFieldData> origData = object->GetFieldData(key);
		TShared<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, object->GetTypeInfo()->ValueType, origData);

		if(newData != nullptr)
			object->SetFieldData(key, newData);
	}

	for(auto& key : delta->Removed)
	{
		object->RemoveFieldData(key);
	}

	return nullptr;
}

TShared<ManagedSerializableFieldData> ManagedSerializableDelta::ApplyDiff(const TShared<Modification>& delta, const TShared<ManagedTypeInfo>& fieldType, const TShared<ManagedSerializableFieldData>& fieldData)
{
	TShared<ManagedSerializableFieldData> newData;
	switch(delta->GetTypeId())
	{
	case TID_ScriptModifiedObject:
		{
			TShared<ManagedSerializableFieldDataObject> origObjData = std::static_pointer_cast<ManagedSerializableFieldDataObject>(fieldData);
			TShared<ManagedSerializableObject> childObj = origObjData->Value;

			TShared<ManagedTypeInfoObject> objTypeInfo =
				std::static_pointer_cast<ManagedTypeInfoObject>(fieldType);

			if(childObj == nullptr) // Object was deleted in original but we have modifications for it, so we create it
			{
				childObj = ManagedSerializableObject::CreateNew(objTypeInfo);
				newData = ManagedSerializableFieldData::Create(objTypeInfo, childObj->GetManagedInstance());
			}

			TShared<ModifiedObject> childMod = std::static_pointer_cast<ModifiedObject>(delta);
			ApplyObjectDelta(childMod, childObj);
		}
		break;
	case TID_ScriptModifiedArray:
		{
			if(fieldType->GetTypeId() == TID_ManagedTypeInfoArray)
			{
				TShared<ManagedSerializableFieldDataArray> origArrayData = std::static_pointer_cast<ManagedSerializableFieldDataArray>(fieldData);
				TShared<ManagedSerializableArray> childArray = origArrayData->Value;

				TShared<ManagedTypeInfoArray> arrayTypeInfo =
					std::static_pointer_cast<ManagedTypeInfoArray>(fieldType);

				TShared<ModifiedArray> childMod = std::static_pointer_cast<ModifiedArray>(delta);
				if(childArray == nullptr) // Object was deleted in original but we have modifications for it, so we create it
					childArray = ManagedSerializableArray::CreateNew(arrayTypeInfo, childMod->OrigSizes);

				newData = ApplyArrayDelta(childMod, childArray);
			}
			else if(fieldType->GetTypeId() == TID_ManagedTypeInfoList)
			{
				TShared<ManagedSerializableFieldDataList> origListData = std::static_pointer_cast<ManagedSerializableFieldDataList>(fieldData);
				TShared<ManagedSerializableList> childList = origListData->Value;

				TShared<ManagedTypeInfoList> listTypeInfo =
					std::static_pointer_cast<ManagedTypeInfoList>(fieldType);

				TShared<ModifiedArray> childMod = std::static_pointer_cast<ModifiedArray>(delta);
				if(childList == nullptr) // Object was deleted in original but we have modifications for it, so we create it
					childList = ManagedSerializableList::CreateNew(listTypeInfo, childMod->OrigSizes[0]);

				newData = ApplyListDelta(childMod, childList);
			}
		}
		break;
	case TID_ScriptModifiedDictionary:
		{
			TShared<ManagedSerializableFieldDataDictionary> origObjData = std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(fieldData);
			TShared<ManagedSerializableDictionary> childDict = origObjData->Value;

			TShared<ManagedTypeInfoDictionary> dictTypeInfo =
				std::static_pointer_cast<ManagedTypeInfoDictionary>(fieldType);

			if(childDict == nullptr) // Object was deleted in original but we have modifications for it, so we create it
			{
				childDict = ManagedSerializableDictionary::CreateNew(dictTypeInfo);
				newData = ManagedSerializableFieldData::Create(dictTypeInfo, childDict->GetManagedInstance());
			}

			TShared<ModifiedDictionary> childMod = std::static_pointer_cast<ModifiedDictionary>(delta);
			ApplyDictionaryDelta(childMod, childDict);
		}
		break;
	default: // Modified field
		{
			TShared<ModifiedEntry> childMod = std::static_pointer_cast<ModifiedEntry>(delta);
			newData = childMod->Value;
		}
		break;
	}

	return newData;
}

RTTIType* ManagedSerializableDelta::GetRttiStatic()
{
	return ManagedSerializableDeltaRTTI::Instance();
}

RTTIType* ManagedSerializableDelta::GetRtti() const
{
	return ManagedSerializableDelta::GetRttiStatic();
}
