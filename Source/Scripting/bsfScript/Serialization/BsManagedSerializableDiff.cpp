//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsManagedSerializableDiff.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Serialization/BsManagedSerializableField.h"
#include "Serialization/BsManagedSerializableArray.h"
#include "Serialization/BsManagedSerializableList.h"
#include "Serialization/BsManagedSerializableDictionary.h"
#include "RTTI/BsManagedSerializableDiffRTTI.h"

namespace bs
{
	ManagedSerializableDiff::ModifiedField::ModifiedField(const SPtr<ManagedSerializableTypeInfo>& parentType, const SPtr<ManagedSerializableMemberInfo>& fieldType, const SPtr<ManagedSerializableDiff::Modification>& modification)
		: ParentType(parentType), FieldType(fieldType), Modification(modification)
	{}

	RTTITypeBase* ManagedSerializableDiff::ModifiedField::GetRttiStatic()
	{
		return ModifiedFieldRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedField::GetRtti() const
	{
		return GetRttiStatic();
	}

	ManagedSerializableDiff::ModifiedArrayEntry::ModifiedArrayEntry(u32 idx, const SPtr<ManagedSerializableDiff::Modification>& modification)
		: Idx(idx), Modification(modification)
	{}

	RTTITypeBase* ManagedSerializableDiff::ModifiedArrayEntry::GetRttiStatic()
	{
		return ModifiedArrayEntryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedArrayEntry::GetRtti() const
	{
		return GetRttiStatic();
	}

	ManagedSerializableDiff::ModifiedDictionaryEntry::ModifiedDictionaryEntry(
		const SPtr<ManagedSerializableFieldData>& key, const SPtr<ManagedSerializableDiff::Modification>& modification)
		: Key(key), Modification(modification)
	{}

	RTTITypeBase* ManagedSerializableDiff::ModifiedDictionaryEntry::GetRttiStatic()
	{
		return ModifiedDictionaryEntryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedDictionaryEntry::GetRtti() const
	{
		return GetRttiStatic();
	}

	RTTITypeBase* ManagedSerializableDiff::Modification::GetRttiStatic()
	{
		return ModificationRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::Modification::GetRtti() const
	{
		return GetRttiStatic();
	}

	SPtr<ManagedSerializableDiff::ModifiedObject> ManagedSerializableDiff::ModifiedObject::Create()
	{
		return bs_shared_ptr_new<ModifiedObject>();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedObject::GetRttiStatic()
	{
		return ModifiedObjectRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedObject::GetRtti() const
	{
		return GetRttiStatic();
	}

	SPtr<ManagedSerializableDiff::ModifiedArray> ManagedSerializableDiff::ModifiedArray::Create()
	{
		return bs_shared_ptr_new<ModifiedArray>();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedArray::GetRttiStatic()
	{
		return ModifiedArrayRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedArray::GetRtti() const
	{
		return GetRttiStatic();
	}

	SPtr<ManagedSerializableDiff::ModifiedDictionary> ManagedSerializableDiff::ModifiedDictionary::Create()
	{
		return bs_shared_ptr_new<ModifiedDictionary>();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedDictionary::GetRttiStatic()
	{
		return ModifiedDictionaryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedDictionary::GetRtti() const
	{
		return GetRttiStatic();
	}

	ManagedSerializableDiff::ModifiedEntry::ModifiedEntry(const SPtr<ManagedSerializableFieldData>& value)
		: Value(value)
	{}

	SPtr<ManagedSerializableDiff::ModifiedEntry> ManagedSerializableDiff::ModifiedEntry::Create(const SPtr<ManagedSerializableFieldData>& value)
	{
		return bs_shared_ptr_new<ModifiedEntry>(value);
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedEntry::GetRttiStatic()
	{
		return ModifiedEntryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedEntry::GetRtti() const
	{
		return GetRttiStatic();
	}

	ManagedSerializableDiff::ManagedSerializableDiff()
		: mModificationRoot(ModifiedObject::Create())
	{
	}

	SPtr<ManagedSerializableDiff> ManagedSerializableDiff::Create(const ManagedSerializableObject* oldObj, const ManagedSerializableObject* newObj)
	{
		assert(oldObj != nullptr && newObj != nullptr);

		SPtr<ManagedSerializableObjectInfo> oldObjInfo = oldObj->GetObjectInfo();
		SPtr<ManagedSerializableObjectInfo> newObjInfo = newObj->GetObjectInfo();

		if(!oldObjInfo->MTypeInfo->Matches(newObjInfo->MTypeInfo))
			return nullptr;

		SPtr<ManagedSerializableDiff> output = bs_shared_ptr_new<ManagedSerializableDiff>();
		SPtr<ModifiedObject> modifications = output->GenerateDiff(oldObj, newObj);

		if(modifications != nullptr)
		{
			output->mModificationRoot->Entries = modifications->Entries;
			return output;
		}

		return nullptr;
	}

	SPtr<ManagedSerializableDiff::ModifiedObject> ManagedSerializableDiff::GenerateDiff(const ManagedSerializableObject* oldObj, const ManagedSerializableObject* newObj)
	{
		SPtr<ModifiedObject> output = nullptr;

		SPtr<ManagedSerializableObjectInfo> curObjInfo = newObj->GetObjectInfo();
		while(curObjInfo != nullptr)
		{
			for(auto& field : curObjInfo->MFields)
			{
				if(!field.second->IsSerializable())
					continue;

				u32 fieldTypeId = field.second->MTypeInfo->GetTypeId();

				SPtr<ManagedSerializableFieldData> oldData = oldObj->GetFieldData(field.second);
				SPtr<ManagedSerializableFieldData> newData = newObj->GetFieldData(field.second);
				SPtr<Modification> newMod = GenerateDiff(oldData, newData, fieldTypeId);

				if(newMod != nullptr)
				{
					if(output == nullptr)
						output = ModifiedObject::Create();

					output->Entries.push_back(ModifiedField(curObjInfo->MTypeInfo, field.second, newMod));
				}
			}

			curObjInfo = curObjInfo->MBaseClass;
		}

		return output;
	}

	SPtr<ManagedSerializableDiff::Modification> ManagedSerializableDiff::GenerateDiff(
		const SPtr<ManagedSerializableFieldData>& oldData, const SPtr<ManagedSerializableFieldData>& newData,
		u32 entryTypeId)
	{
		bool isPrimitive = entryTypeId == TID_SerializableTypeInfoPrimitive ||
			entryTypeId == TID_SerializableTypeInfoRef ||
			entryTypeId == TID_SerializableTypeInfoEnum ||
			entryTypeId == TID_SerializableTypeInfoRRef;

		// It's possible the field data is null if the class structure changed (i.e. new field was added that is not present
		// in serialized data). Check for this case first to ensure field data is valid for the remainder of the method.
		if(oldData == nullptr)
		{
			if(newData == nullptr)
				return nullptr;
			else
				return ModifiedEntry::Create(newData);
		}
		else
		{
			if(newData == nullptr)
				return nullptr;
		}

		SPtr<Modification> newMod = nullptr;
		if(isPrimitive)
		{
			if(!oldData->Equals(newData))
				newMod = ModifiedEntry::Create(newData);
		}
		else
		{
			switch(entryTypeId)
			{
			case TID_SerializableTypeInfoObject:
				{
					SPtr<ManagedSerializableFieldDataObject> oldObjData =
						std::static_pointer_cast<ManagedSerializableFieldDataObject>(oldData);
					SPtr<ManagedSerializableFieldDataObject> newObjData =
						std::static_pointer_cast<ManagedSerializableFieldDataObject>(newData);

					if(oldObjData->Value != nullptr && newObjData->Value != nullptr)
					{
						newMod = GenerateDiff(oldObjData->Value.get(), newObjData->Value.get());
					}
					else if(oldObjData->Value == nullptr && newObjData->Value == nullptr)
					{
						// No change
					}
					else // We either record null if new value is null, or the entire object if old value is null
					{
						newMod = ModifiedEntry::Create(newData);
					}
				}
				break;
			case TID_SerializableTypeInfoArray:
				{
					SPtr<ManagedSerializableFieldDataArray> oldArrayData =
						std::static_pointer_cast<ManagedSerializableFieldDataArray>(oldData);
					SPtr<ManagedSerializableFieldDataArray> newArrayData =
						std::static_pointer_cast<ManagedSerializableFieldDataArray>(newData);

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

								u32 arrayElemTypeId = newArrayData->Value->GetTypeInfo()->MElementType->GetTypeId();
								arrayElemMod = GenerateDiff(oldArrayElem, newArrayElem, arrayElemTypeId);
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
						newMod = ModifiedEntry::Create(newData);
					}
				}
				break;
			case TID_SerializableTypeInfoList:
				{
					SPtr<ManagedSerializableFieldDataList> oldListData =
						std::static_pointer_cast<ManagedSerializableFieldDataList>(oldData);
					SPtr<ManagedSerializableFieldDataList> newListData =
						std::static_pointer_cast<ManagedSerializableFieldDataList>(newData);

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

								u32 arrayElemTypeId = newListData->Value->GetTypeInfo()->MElementType->GetTypeId();
								listElemMod = GenerateDiff(oldListElem, newListElem, arrayElemTypeId);
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
						newMod = ModifiedEntry::Create(newData);
					}
				}
				break;
			case TID_SerializableTypeInfoDictionary:
				{
					SPtr<ManagedSerializableFieldDataDictionary> oldDictData =
						std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(oldData);
					SPtr<ManagedSerializableFieldDataDictionary> newDictData =
						std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(newData);

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
								u32 dictElemTypeId = newDictData->Value->GetTypeInfo()->MValueType->GetTypeId();

								dictElemMod = GenerateDiff(oldDictData->Value->GetFieldData(key), newEnumerator.GetValue(), dictElemTypeId);
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
						newMod = ModifiedEntry::Create(newData);
					}
				}
				break;
			default:
				assert(false); // Invalid type
				break;
			}
		}

		return newMod;
	}

	void ManagedSerializableDiff::Apply(const SPtr<ManagedSerializableObject>& obj)
	{
		ApplyDiff(mModificationRoot, obj);
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDiff::ApplyDiff(const SPtr<ModifiedObject>& mod, const SPtr<ManagedSerializableObject>& obj)
	{
		SPtr<ManagedSerializableObjectInfo> objInfo = obj->GetObjectInfo();
		for(auto& modEntry : mod->Entries)
		{
			SPtr<ManagedSerializableMemberInfo> fieldType = modEntry.FieldType;
			SPtr<ManagedSerializableTypeInfo> typeInfo = modEntry.ParentType;

			SPtr<ManagedSerializableMemberInfo> matchingFieldInfo = objInfo->FindMatchingField(fieldType, typeInfo);
			if(matchingFieldInfo == nullptr)
				continue; // Field no longer exists in the type

			SPtr<ManagedSerializableFieldData> origData = obj->GetFieldData(matchingFieldInfo);

			SPtr<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, matchingFieldInfo->MTypeInfo, origData);
			if(newData != nullptr)
				obj->SetFieldData(matchingFieldInfo, newData);
		}

		return nullptr;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDiff::ApplyDiff(const SPtr<ModifiedArray>& mod, const SPtr<ManagedSerializableArray>& obj)
	{
		bool needsResize = false;

		for(u32 i = 0; i < (u32)mod->NewSizes.size(); i++)
		{
			if(mod->NewSizes[i] != obj->GetLength(i))
			{
				needsResize = true;
				break;
			}
		}

		SPtr<ManagedSerializableFieldData> newArray;
		if(needsResize)
		{
			obj->Resize(mod->NewSizes);
			newArray = ManagedSerializableFieldData::Create(obj->GetTypeInfo(), obj->GetManagedInstance());
		}

		for(auto& modEntry : mod->Entries)
		{
			u32 arrayIdx = modEntry.Idx;

			SPtr<ManagedSerializableFieldData> origData = obj->GetFieldData(arrayIdx);
			SPtr<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, obj->GetTypeInfo()->MElementType, origData);

			if(newData != nullptr)
				obj->SetFieldData(arrayIdx, newData);
		}

		return newArray;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDiff::ApplyDiff(const SPtr<ModifiedArray>& mod, const SPtr<ManagedSerializableList>& obj)
	{
		bool needsResize = mod->NewSizes[0] != obj->GetLength();

		SPtr<ManagedSerializableFieldData> newList;
		if(needsResize)
		{
			obj->Resize(mod->NewSizes[0]);
			newList = ManagedSerializableFieldData::Create(obj->GetTypeInfo(), obj->GetManagedInstance());
		}

		for(auto& modEntry : mod->Entries)
		{
			u32 arrayIdx = modEntry.Idx;

			SPtr<ManagedSerializableFieldData> origData = obj->GetFieldData(arrayIdx);
			SPtr<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, obj->GetTypeInfo()->MElementType, origData);

			if(newData != nullptr)
				obj->SetFieldData(arrayIdx, newData);
		}

		return newList;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDiff::ApplyDiff(const SPtr<ModifiedDictionary>& mod, const SPtr<ManagedSerializableDictionary>& obj)
	{
		for(auto& modEntry : mod->Entries)
		{
			SPtr<ManagedSerializableFieldData> key = modEntry.Key;

			SPtr<ManagedSerializableFieldData> origData = obj->GetFieldData(key);
			SPtr<ManagedSerializableFieldData> newData = ApplyDiff(modEntry.Modification, obj->GetTypeInfo()->MValueType, origData);

			if(newData != nullptr)
				obj->SetFieldData(key, newData);
		}

		for(auto& key : mod->Removed)
		{
			obj->RemoveFieldData(key);
		}

		return nullptr;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDiff::ApplyDiff(const SPtr<Modification>& mod, const SPtr<ManagedSerializableTypeInfo>& fieldType, const SPtr<ManagedSerializableFieldData>& origData)
	{
		SPtr<ManagedSerializableFieldData> newData;
		switch(mod->GetTypeId())
		{
		case TID_ScriptModifiedObject:
			{
				SPtr<ManagedSerializableFieldDataObject> origObjData = std::static_pointer_cast<ManagedSerializableFieldDataObject>(origData);
				SPtr<ManagedSerializableObject> childObj = origObjData->Value;

				SPtr<ManagedSerializableTypeInfoObject> objTypeInfo =
					std::static_pointer_cast<ManagedSerializableTypeInfoObject>(fieldType);

				if(childObj == nullptr) // Object was deleted in original but we have modifications for it, so we create it
				{
					childObj = ManagedSerializableObject::CreateNew(objTypeInfo);
					newData = ManagedSerializableFieldData::Create(objTypeInfo, childObj->GetManagedInstance());
				}

				SPtr<ModifiedObject> childMod = std::static_pointer_cast<ModifiedObject>(mod);
				ApplyDiff(childMod, childObj);
			}
			break;
		case TID_ScriptModifiedArray:
			{
				if(fieldType->GetTypeId() == TID_SerializableTypeInfoArray)
				{
					SPtr<ManagedSerializableFieldDataArray> origArrayData = std::static_pointer_cast<ManagedSerializableFieldDataArray>(origData);
					SPtr<ManagedSerializableArray> childArray = origArrayData->Value;

					SPtr<ManagedSerializableTypeInfoArray> arrayTypeInfo =
						std::static_pointer_cast<ManagedSerializableTypeInfoArray>(fieldType);

					SPtr<ModifiedArray> childMod = std::static_pointer_cast<ModifiedArray>(mod);
					if(childArray == nullptr) // Object was deleted in original but we have modifications for it, so we create it
						childArray = ManagedSerializableArray::CreateNew(arrayTypeInfo, childMod->OrigSizes);

					newData = ApplyDiff(childMod, childArray);
				}
				else if(fieldType->GetTypeId() == TID_SerializableTypeInfoList)
				{
					SPtr<ManagedSerializableFieldDataList> origListData = std::static_pointer_cast<ManagedSerializableFieldDataList>(origData);
					SPtr<ManagedSerializableList> childList = origListData->Value;

					SPtr<ManagedSerializableTypeInfoList> listTypeInfo =
						std::static_pointer_cast<ManagedSerializableTypeInfoList>(fieldType);

					SPtr<ModifiedArray> childMod = std::static_pointer_cast<ModifiedArray>(mod);
					if(childList == nullptr) // Object was deleted in original but we have modifications for it, so we create it
						childList = ManagedSerializableList::CreateNew(listTypeInfo, childMod->OrigSizes[0]);

					newData = ApplyDiff(childMod, childList);
				}
			}
			break;
		case TID_ScriptModifiedDictionary:
			{
				SPtr<ManagedSerializableFieldDataDictionary> origObjData = std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(origData);
				SPtr<ManagedSerializableDictionary> childDict = origObjData->Value;

				SPtr<ManagedSerializableTypeInfoDictionary> dictTypeInfo =
					std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(fieldType);

				if(childDict == nullptr) // Object was deleted in original but we have modifications for it, so we create it
				{
					childDict = ManagedSerializableDictionary::CreateNew(dictTypeInfo);
					newData = ManagedSerializableFieldData::Create(dictTypeInfo, childDict->GetManagedInstance());
				}

				SPtr<ModifiedDictionary> childMod = std::static_pointer_cast<ModifiedDictionary>(mod);
				ApplyDiff(childMod, childDict);
			}
			break;
		default: // Modified field
			{
				SPtr<ModifiedEntry> childMod = std::static_pointer_cast<ModifiedEntry>(mod);
				newData = childMod->Value;
			}
			break;
		}

		return newData;
	}

	RTTITypeBase* ManagedSerializableDiff::GetRttiStatic()
	{
		return ManagedSerializableDiffRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::GetRtti() const
	{
		return ManagedSerializableDiff::GetRttiStatic();
	}
} // namespace bs
