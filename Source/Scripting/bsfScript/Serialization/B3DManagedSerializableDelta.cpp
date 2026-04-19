//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
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
ManagedSerializableDelta::ModifiedField::ModifiedField(const SPtr<ManagedTypeInfo>& parentType, const SPtr<ManagedMemberInfo>& fieldType, const SPtr<ManagedSerializableDelta::Modification>& modification)
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

ManagedSerializableDelta::ModifiedArrayEntry::ModifiedArrayEntry(u32 idx, const SPtr<ManagedSerializableDelta::Modification>& modification)
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
	const SPtr<ManagedSerializableFieldData>& key, const SPtr<ManagedSerializableDelta::Modification>& modification)
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

SPtr<ManagedSerializableDelta::ModifiedObject> ManagedSerializableDelta::ModifiedObject::Create()
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

SPtr<ManagedSerializableDelta::ModifiedArray> ManagedSerializableDelta::ModifiedArray::Create()
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

SPtr<ManagedSerializableDelta::ModifiedDictionary> ManagedSerializableDelta::ModifiedDictionary::Create()
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

ManagedSerializableDelta::ModifiedEntry::ModifiedEntry(const SPtr<ManagedSerializableFieldData>& value)
	: Value(value)
{}

SPtr<ManagedSerializableDelta::ModifiedEntry> ManagedSerializableDelta::ModifiedEntry::Create(const SPtr<ManagedSerializableFieldData>& value)
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

SPtr<ManagedSerializableDelta> ManagedSerializableDelta::Create(const ManagedSerializableObject* original, const ManagedSerializableObject* modified, RTTIOperationContext* context)
{
	B3D_ASSERT(original != nullptr && modified != nullptr);

	SPtr<ManagedObjectInfo> oldObjInfo = original->GetObjectInfo();
	SPtr<ManagedObjectInfo> newObjInfo = modified->GetObjectInfo();

	if(!oldObjInfo->TypeInfo->Matches(newObjInfo->TypeInfo))
		return nullptr;

	SPtr<ManagedSerializableDelta> output = B3DMakeShared<ManagedSerializableDelta>();
	SPtr<ModifiedObject> modifications = output->GenerateObjectDelta(original, modified, context);

	if(modifications != nullptr)
	{
		output->mModificationRoot->Entries = modifications->Entries;
		return output;
	}

	return nullptr;
}

SPtr<ManagedSerializableDelta::ModifiedObject> ManagedSerializableDelta::GenerateObjectDelta(const ManagedSerializableObject* original, const ManagedSerializableObject* modified, RTTIOperationContext* context)
{
	SPtr<ModifiedObject> output = nullptr;

	SPtr<ManagedObjectInfo> curObjInfo = modified->GetObjectInfo();
	while(curObjInfo != nullptr)
	{
		for(auto& member : curObjInfo->Members)
		{
			if(!member->IsSerializable())
				continue;

			u32 fieldTypeId = member->TypeInfo->GetTypeId();

			SPtr<ManagedSerializableFieldData> oldData = original->GetFieldData(member);
			SPtr<ManagedSerializableFieldData> newData = modified->GetFieldData(member);
			SPtr<Modification> newMod = GenerateFieldDelta(oldData, newData, fieldTypeId, context);

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

SPtr<ManagedSerializableDelta::Modification> ManagedSerializableDelta::GenerateFieldDelta(const SPtr<ManagedSerializableFieldData>& original, const SPtr<ManagedSerializableFieldData>& modified, u32 entryTypeId, RTTIOperationContext* context)
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

	SPtr<Modification> newMod = nullptr;
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
				SPtr<ManagedSerializableFieldDataObject> oldObjData =
					std::static_pointer_cast<ManagedSerializableFieldDataObject>(original);
				SPtr<ManagedSerializableFieldDataObject> newObjData =
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
				SPtr<ManagedSerializableFieldDataArray> oldArrayData =
					std::static_pointer_cast<ManagedSerializableFieldDataArray>(original);
				SPtr<ManagedSerializableFieldDataArray> newArrayData =
					std::static_pointer_cast<ManagedSerializableFieldDataArray>(modified);

				if(oldArrayData->Value != nullptr && newArrayData->Value != nullptr)
				{
					u32 oldLength = oldArrayData->Value->GetTotalLength();
					u32 newLength = newArrayData->Value->GetTotalLength();

					SPtr<ModifiedArray> arrayMods = nullptr;
					for(u32 i = 0; i < newLength; i++)
					{
						SPtr<Modification> arrayElemMod = nullptr;

						SPtr<ManagedSerializableFieldData> newArrayElem = newArrayData->Value->GetFieldData(i);
						if(i < oldLength)
						{
							SPtr<ManagedSerializableFieldData> oldArrayElem = oldArrayData->Value->GetFieldData(i);

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
				SPtr<ManagedSerializableFieldDataList> oldListData =
					std::static_pointer_cast<ManagedSerializableFieldDataList>(original);
				SPtr<ManagedSerializableFieldDataList> newListData =
					std::static_pointer_cast<ManagedSerializableFieldDataList>(modified);

				if(oldListData->Value != nullptr && newListData->Value != nullptr)
				{
					u32 oldLength = oldListData->Value->GetLength();
					u32 newLength = newListData->Value->GetLength();

					SPtr<ModifiedArray> listMods = nullptr;
					for(u32 i = 0; i < newLength; i++)
					{
						SPtr<Modification> listElemMod = nullptr;

						SPtr<ManagedSerializableFieldData> newListElem = newListData->Value->GetFieldData(i);
						if(i < oldLength)
						{
							SPtr<ManagedSerializableFieldData> oldListElem = oldListData->Value->GetFieldData(i);

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
				SPtr<ManagedSerializableFieldDataDictionary> oldDictData =
					std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(original);
				SPtr<ManagedSerializableFieldDataDictionary> newDictData =
					std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(modified);

				if(oldDictData->Value != nullptr && newDictData->Value != nullptr)
				{
					SPtr<ModifiedDictionary> dictMods = nullptr;

					auto newEnumerator = newDictData->Value->GetEnumerator();
					while(newEnumerator.MoveNext())
					{
						SPtr<Modification> dictElemMod = nullptr;

						SPtr<ManagedSerializableFieldData> key = newEnumerator.GetKey();
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
						SPtr<ManagedSerializableFieldData> key = oldEnumerator.GetKey();
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

void ManagedSerializableDelta::Apply(const SPtr<ManagedSerializableObject>& object)
{
	ApplyObjectDelta(mModificationRoot, object);
}

SPtr<ManagedSerializableFieldData> ManagedSerializableDelta::ApplyObjectDelta(const SPtr<ModifiedObject>& delta, const SPtr<ManagedSerializableObject>& object)
{
	SPtr<ManagedObjectInfo> objInfo = object->GetObjectInfo();
	for(auto& modEntry : delta->Entries)
	{
		SPtr<ManagedMemberInfo> fieldType = modEntry.FieldType;
		SPtr<ManagedTypeInfo> typeInfo = modEntry.ParentType;

		SPtr<ManagedMemberInfo> matchingFieldInfo = objInfo->FindMatchingField(fieldType, typeInfo);
		if(matchingFieldInfo == nullptr)
			continue; // Field no longer exists in the type

		SPtr<ManagedSerializableFieldData> origData = object->GetFieldData(matchingFieldInfo);

		SPtr<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, matchingFieldInfo->TypeInfo, origData);
		if(newData != nullptr)
			object->SetFieldData(matchingFieldInfo, newData);
	}

	return nullptr;
}

SPtr<ManagedSerializableFieldData> ManagedSerializableDelta::ApplyArrayDelta(const SPtr<ModifiedArray>& delta, const SPtr<ManagedSerializableArray>& object)
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

	SPtr<ManagedSerializableFieldData> newArray;
	if(needsResize)
	{
		object->Resize(delta->NewSizes);
		newArray = ManagedSerializableFieldData::Create(object->GetTypeInfo(), object->GetManagedInstance());
	}

	for(auto& modEntry : delta->Entries)
	{
		u32 arrayIdx = modEntry.Idx;

		SPtr<ManagedSerializableFieldData> origData = object->GetFieldData(arrayIdx);
		SPtr<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, object->GetTypeInfo()->ElementType, origData);

		if(newData != nullptr)
			object->SetFieldData(arrayIdx, newData);
	}

	return newArray;
}

SPtr<ManagedSerializableFieldData> ManagedSerializableDelta::ApplyListDelta(const SPtr<ModifiedArray>& delta, const SPtr<ManagedSerializableList>& object)
{
	bool needsResize = delta->NewSizes[0] != object->GetLength();

	SPtr<ManagedSerializableFieldData> newList;
	if(needsResize)
	{
		object->Resize(delta->NewSizes[0]);
		newList = ManagedSerializableFieldData::Create(object->GetTypeInfo(), object->GetManagedInstance());
	}

	for(auto& modEntry : delta->Entries)
	{
		u32 arrayIdx = modEntry.Idx;

		SPtr<ManagedSerializableFieldData> origData = object->GetFieldData(arrayIdx);
		SPtr<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, object->GetTypeInfo()->ElementType, origData);

		if(newData != nullptr)
			object->SetFieldData(arrayIdx, newData);
	}

	return newList;
}

SPtr<ManagedSerializableFieldData> ManagedSerializableDelta::ApplyDictionaryDelta(const SPtr<ModifiedDictionary>& delta, const SPtr<ManagedSerializableDictionary>& object)
{
	for(auto& modEntry : delta->Entries)
	{
		SPtr<ManagedSerializableFieldData> key = modEntry.Key;

		SPtr<ManagedSerializableFieldData> origData = object->GetFieldData(key);
		SPtr<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, object->GetTypeInfo()->ValueType, origData);

		if(newData != nullptr)
			object->SetFieldData(key, newData);
	}

	for(auto& key : delta->Removed)
	{
		object->RemoveFieldData(key);
	}

	return nullptr;
}

SPtr<ManagedSerializableFieldData> ManagedSerializableDelta::ApplyDiff(const SPtr<Modification>& delta, const SPtr<ManagedTypeInfo>& fieldType, const SPtr<ManagedSerializableFieldData>& fieldData)
{
	SPtr<ManagedSerializableFieldData> newData;
	switch(delta->GetTypeId())
	{
	case TID_ScriptModifiedObject:
		{
			SPtr<ManagedSerializableFieldDataObject> origObjData = std::static_pointer_cast<ManagedSerializableFieldDataObject>(fieldData);
			SPtr<ManagedSerializableObject> childObj = origObjData->Value;

			SPtr<ManagedTypeInfoObject> objTypeInfo =
				std::static_pointer_cast<ManagedTypeInfoObject>(fieldType);

			if(childObj == nullptr) // Object was deleted in original but we have modifications for it, so we create it
			{
				childObj = ManagedSerializableObject::CreateNew(objTypeInfo);
				newData = ManagedSerializableFieldData::Create(objTypeInfo, childObj->GetManagedInstance());
			}

			SPtr<ModifiedObject> childMod = std::static_pointer_cast<ModifiedObject>(delta);
			ApplyObjectDelta(childMod, childObj);
		}
		break;
	case TID_ScriptModifiedArray:
		{
			if(fieldType->GetTypeId() == TID_ManagedTypeInfoArray)
			{
				SPtr<ManagedSerializableFieldDataArray> origArrayData = std::static_pointer_cast<ManagedSerializableFieldDataArray>(fieldData);
				SPtr<ManagedSerializableArray> childArray = origArrayData->Value;

				SPtr<ManagedTypeInfoArray> arrayTypeInfo =
					std::static_pointer_cast<ManagedTypeInfoArray>(fieldType);

				SPtr<ModifiedArray> childMod = std::static_pointer_cast<ModifiedArray>(delta);
				if(childArray == nullptr) // Object was deleted in original but we have modifications for it, so we create it
					childArray = ManagedSerializableArray::CreateNew(arrayTypeInfo, childMod->OrigSizes);

				newData = ApplyArrayDelta(childMod, childArray);
			}
			else if(fieldType->GetTypeId() == TID_ManagedTypeInfoList)
			{
				SPtr<ManagedSerializableFieldDataList> origListData = std::static_pointer_cast<ManagedSerializableFieldDataList>(fieldData);
				SPtr<ManagedSerializableList> childList = origListData->Value;

				SPtr<ManagedTypeInfoList> listTypeInfo =
					std::static_pointer_cast<ManagedTypeInfoList>(fieldType);

				SPtr<ModifiedArray> childMod = std::static_pointer_cast<ModifiedArray>(delta);
				if(childList == nullptr) // Object was deleted in original but we have modifications for it, so we create it
					childList = ManagedSerializableList::CreateNew(listTypeInfo, childMod->OrigSizes[0]);

				newData = ApplyListDelta(childMod, childList);
			}
		}
		break;
	case TID_ScriptModifiedDictionary:
		{
			SPtr<ManagedSerializableFieldDataDictionary> origObjData = std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(fieldData);
			SPtr<ManagedSerializableDictionary> childDict = origObjData->Value;

			SPtr<ManagedTypeInfoDictionary> dictTypeInfo =
				std::static_pointer_cast<ManagedTypeInfoDictionary>(fieldType);

			if(childDict == nullptr) // Object was deleted in original but we have modifications for it, so we create it
			{
				childDict = ManagedSerializableDictionary::CreateNew(dictTypeInfo);
				newData = ManagedSerializableFieldData::Create(dictTypeInfo, childDict->GetManagedInstance());
			}

			SPtr<ModifiedDictionary> childMod = std::static_pointer_cast<ModifiedDictionary>(delta);
			ApplyDictionaryDelta(childMod, childDict);
		}
		break;
	default: // Modified field
		{
			SPtr<ModifiedEntry> childMod = std::static_pointer_cast<ModifiedEntry>(delta);
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
