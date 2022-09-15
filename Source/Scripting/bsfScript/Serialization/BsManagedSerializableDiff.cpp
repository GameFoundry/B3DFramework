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
	ManagedSerializableDiff::ModifiedField::ModifiedField(const SPtr<ManagedSerializableTypeInfo>& parentType,
		const SPtr<ManagedSerializableMemberInfo>& fieldType, const SPtr<Modification>& modification)
		:parentType(parentType), fieldType(fieldType), modification(modification)
	{ }

	RTTITypeBase* ManagedSerializableDiff::ModifiedField::GetRttiStatic()
	{
		return ModifiedFieldRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedField::GetRtti() const
	{
		return GetRttiStatic();
	}

	ManagedSerializableDiff::ModifiedArrayEntry::ModifiedArrayEntry(UINT32 idx, const SPtr<Modification>& modification)
		:idx(idx), modification(modification)
	{ }

	RTTITypeBase* ManagedSerializableDiff::ModifiedArrayEntry::GetRttiStatic()
	{
		return ModifiedArrayEntryRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableDiff::ModifiedArrayEntry::GetRtti() const
	{
		return GetRttiStatic();
	}

	ManagedSerializableDiff::ModifiedDictionaryEntry::ModifiedDictionaryEntry(
		const SPtr<ManagedSerializableFieldData>& key, const SPtr<Modification>& modification)
		:key(key), modification(modification)
	{ }

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
		:value(value)
	{ }

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

	SPtr<ManagedSerializableDiff> ManagedSerializableDiff::Create(const ManagedSerializableObject* oldObj,
		const ManagedSerializableObject* newObj)
	{
		assert(oldObj != nullptr && newObj != nullptr);

		SPtr<ManagedSerializableObjectInfo> oldObjInfo = oldObj->GetObjectInfo();
		SPtr<ManagedSerializableObjectInfo> newObjInfo = newObj->GetObjectInfo();

		if (!oldObjInfo->mTypeInfo->matches(newObjInfo->mTypeInfo))
			return nullptr;

		SPtr<ManagedSerializableDiff> output = bs_shared_ptr_new<ManagedSerializableDiff>();
		SPtr<ModifiedObject> modifications = output->generateDiff(oldObj, newObj);

		if (modifications != nullptr)
		{
			output->mModificationRoot->entries = modifications->entries;
			return output;
		}
		
		return nullptr;
	}

	SPtr<ManagedSerializableDiff::ModifiedObject> ManagedSerializableDiff::GenerateDiff
		(const ManagedSerializableObject* oldObj, const ManagedSerializableObject* newObj)
	{
		SPtr<ModifiedObject> output = nullptr;

		SPtr<ManagedSerializableObjectInfo> curObjInfo = newObj->GetObjectInfo();
		while (curObjInfo != nullptr)
		{
			for (auto& field : curObjInfo->mFields)
			{
				if (!field.second->isSerializable())
					continue;

				UINT32 fieldTypeId = field.second->mTypeInfo->GetTypeId();

				SPtr<ManagedSerializableFieldData> oldData = oldObj->GetFieldData(field.second);
				SPtr<ManagedSerializableFieldData> newData = newObj->GetFieldData(field.second);
				SPtr<Modification> newMod = generateDiff(oldData, newData, fieldTypeId);
				
				if (newMod != nullptr)
				{
					if (output == nullptr)
						output = ModifiedObject::Create();

					output->entries.push_back(ModifiedField(curObjInfo->mTypeInfo, field.second, newMod));
				}
			}

			curObjInfo = curObjInfo->mBaseClass;
		}

		return output;
	}

	SPtr<ManagedSerializableDiff::Modification> ManagedSerializableDiff::GenerateDiff(
		const SPtr<ManagedSerializableFieldData>& oldData, const SPtr<ManagedSerializableFieldData>& newData,
		UINT32 entryTypeId)
	{
		bool isPrimitive = entryTypeId == TID_SerializableTypeInfoPrimitive ||
			entryTypeId == TID_SerializableTypeInfoRef ||
			entryTypeId == TID_SerializableTypeInfoEnum ||
			entryTypeId == TID_SerializableTypeInfoRRef;

		// It's possible the field data is null if the class structure changed (i.e. new field was added that is not present
		// in serialized data). Check for this case first to ensure field data is valid for the remainder of the method.
		if(oldData == nullptr)
		{
			if (newData == nullptr)
				return nullptr;
			else
				return ModifiedEntry::Create(newData);
		}
		else
		{
			if (newData == nullptr)
				return nullptr;
		}

		SPtr<Modification> newMod = nullptr;
		if (isPrimitive)
		{
			if (!oldData->equals(newData))
				newMod = ModifiedEntry::Create(newData);
		}
		else
		{
			switch (entryTypeId)
			{
			case TID_SerializableTypeInfoObject:
			{
				SPtr<ManagedSerializableFieldDataObject> oldObjData =
					std::static_pointer_cast<ManagedSerializableFieldDataObject>(oldData);
				SPtr<ManagedSerializableFieldDataObject> newObjData =
					std::static_pointer_cast<ManagedSerializableFieldDataObject>(newData);

				if (oldObjData->value != nullptr && newObjData->value != nullptr)
				{
					newMod = generateDiff(oldObjData->value.get(), newObjData->value.get());
				}
				else if (oldObjData->value == nullptr && newObjData->value == nullptr)
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

				if (oldArrayData->value != nullptr && newArrayData->value != nullptr)
				{
					UINT32 oldLength = oldArrayData->value->GetTotalLength();
					UINT32 newLength = newArrayData->value->GetTotalLength();

					SPtr<ModifiedArray> arrayMods = nullptr;
					for (UINT32 i = 0; i < newLength; i++)
					{
						SPtr<Modification> arrayElemMod = nullptr;

						SPtr<ManagedSerializableFieldData> newArrayElem = newArrayData->value->GetFieldData(i);
						if (i < oldLength)
						{
							SPtr<ManagedSerializableFieldData> oldArrayElem = oldArrayData->value->GetFieldData(i);

							UINT32 arrayElemTypeId = newArrayData->value->GetTypeInfo()->mElementType->GetTypeId();
							arrayElemMod = generateDiff(oldArrayElem, newArrayElem, arrayElemTypeId);
						}
						else
						{
							arrayElemMod = ModifiedEntry::Create(newArrayElem);
						}

						if (arrayElemMod != nullptr)
						{
							if (arrayMods == nullptr)
								arrayMods = ModifiedArray::Create();

							arrayMods->entries.push_back(ModifiedArrayEntry(i, arrayElemMod));
						}
					}

					if (oldLength != newLength)
					{
						if (arrayMods == nullptr)
							arrayMods = ModifiedArray::Create();
					}

					if (arrayMods != nullptr)
					{
						arrayMods->origSizes = oldArrayData->value->GetLengths();
						arrayMods->newSizes = newArrayData->value->GetLengths();
					}

					newMod = arrayMods;
				}
				else if (oldArrayData->value == nullptr && newArrayData->value == nullptr)
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

				if (oldListData->value != nullptr && newListData->value != nullptr)
				{
					UINT32 oldLength = oldListData->value->GetLength();
					UINT32 newLength = newListData->value->GetLength();

					SPtr<ModifiedArray> listMods = nullptr;
					for (UINT32 i = 0; i < newLength; i++)
					{
						SPtr<Modification> listElemMod = nullptr;

						SPtr<ManagedSerializableFieldData> newListElem = newListData->value->GetFieldData(i);
						if (i < oldLength)
						{
							SPtr<ManagedSerializableFieldData> oldListElem = oldListData->value->GetFieldData(i);

							UINT32 arrayElemTypeId = newListData->value->GetTypeInfo()->mElementType->GetTypeId();
							listElemMod = generateDiff(oldListElem, newListElem, arrayElemTypeId);
						}
						else
						{
							listElemMod = ModifiedEntry::Create(newListElem);
						}

						if (listElemMod != nullptr)
						{
							if (listMods == nullptr)
								listMods = ModifiedArray::Create();

							listMods->entries.push_back(ModifiedArrayEntry(i, listElemMod));
						}
					}

					if (oldLength != newLength)
					{
						if (listMods == nullptr)
							listMods = ModifiedArray::Create();
					}

					if (listMods != nullptr)
					{
						listMods->origSizes.push_back(oldLength);
						listMods->newSizes.push_back(newLength);
					}

					newMod = listMods;
				}
				else if (oldListData->value == nullptr && newListData->value == nullptr)
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

				if (oldDictData->value != nullptr && newDictData->value != nullptr)
				{
					SPtr<ModifiedDictionary> dictMods = nullptr;

					auto newEnumerator = newDictData->value->GetEnumerator();
					while (newEnumerator.moveNext())
					{
						SPtr<Modification> dictElemMod = nullptr;

						SPtr<ManagedSerializableFieldData> key = newEnumerator.getKey();
						if (oldDictData->value->contains(key))
						{
							UINT32 dictElemTypeId = newDictData->value->GetTypeInfo()->mValueType->GetTypeId();

							dictElemMod = generateDiff(oldDictData->value->GetFieldData(key),
								newEnumerator.getValue(), dictElemTypeId);
						}
						else
						{
							dictElemMod = ModifiedEntry::Create(newEnumerator.getValue());
						}

						if (dictElemMod != nullptr)
						{
							if (dictMods == nullptr)
								dictMods = ModifiedDictionary::Create();

							dictMods->entries.push_back(ModifiedDictionaryEntry(key, dictElemMod));
						}
					}

					auto oldEnumerator = oldDictData->value->GetEnumerator();
					while (oldEnumerator.moveNext())
					{
						SPtr<ManagedSerializableFieldData> key = oldEnumerator.getKey();
						if (!newDictData->value->contains(oldEnumerator.getKey()))
						{
							if (dictMods == nullptr)
								dictMods = ModifiedDictionary::Create();

							dictMods->removed.push_back(key);
						}
					}

					newMod = dictMods;
				}
				else if (oldDictData->value == nullptr && newDictData->value == nullptr)
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
		applyDiff(mModificationRoot, obj);
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDiff::ApplyDiff(const SPtr<ModifiedObject>& mod, const SPtr<ManagedSerializableObject>& obj)
	{
		SPtr<ManagedSerializableObjectInfo> objInfo = obj->GetObjectInfo();
		for (auto& modEntry : mod->entries)
		{
			SPtr<ManagedSerializableMemberInfo> fieldType = modEntry.fieldType;
			SPtr<ManagedSerializableTypeInfo> typeInfo = modEntry.parentType;

			SPtr<ManagedSerializableMemberInfo> matchingFieldInfo = objInfo->findMatchingField(fieldType, typeInfo);
			if (matchingFieldInfo == nullptr)
				continue; // Field no longer exists in the type

			SPtr<ManagedSerializableFieldData> origData = obj->GetFieldData(matchingFieldInfo);

			SPtr<ManagedSerializableFieldData> newData = applyDiff(modEntry.modification, matchingFieldInfo->mTypeInfo, origData);
			if (newData != nullptr)
				obj->SetFieldData(matchingFieldInfo, newData);
		}

		return nullptr;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDiff::ApplyDiff(const SPtr<ModifiedArray>& mod, const SPtr<ManagedSerializableArray>& obj)
	{
		bool needsResize = false;

		for (UINT32 i = 0; i < (UINT32)mod->newSizes.size(); i++)
		{
			if (mod->newSizes[i] != obj->GetLength(i))
			{
				needsResize = true;
				break;
			}
		}

		SPtr<ManagedSerializableFieldData> newArray;
		if (needsResize)
		{
			obj->resize(mod->newSizes);
			newArray = ManagedSerializableFieldData::Create(obj->GetTypeInfo(), obj->GetManagedInstance());
		}

		for (auto& modEntry : mod->entries)
		{
			UINT32 arrayIdx = modEntry.idx;

			SPtr<ManagedSerializableFieldData> origData = obj->GetFieldData(arrayIdx);
			SPtr<ManagedSerializableFieldData> newData = applyDiff(modEntry.modification, obj->GetTypeInfo()->mElementType, origData);

			if (newData != nullptr)
				obj->SetFieldData(arrayIdx, newData);
		}

		return newArray;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDiff::ApplyDiff(const SPtr<ModifiedArray>& mod, const SPtr<ManagedSerializableList>& obj)
	{
		bool needsResize = mod->newSizes[0] != obj->GetLength();

		SPtr<ManagedSerializableFieldData> newList;
		if (needsResize)
		{
			obj->resize(mod->newSizes[0]);
			newList = ManagedSerializableFieldData::Create(obj->GetTypeInfo(), obj->GetManagedInstance());
		}

		for (auto& modEntry : mod->entries)
		{
			UINT32 arrayIdx = modEntry.idx;

			SPtr<ManagedSerializableFieldData> origData = obj->GetFieldData(arrayIdx);
			SPtr<ManagedSerializableFieldData> newData = applyDiff(modEntry.modification, obj->GetTypeInfo()->mElementType, origData);

			if (newData != nullptr)
				obj->SetFieldData(arrayIdx, newData);
		}

		return newList;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDiff::ApplyDiff(const SPtr<ModifiedDictionary>& mod, const SPtr<ManagedSerializableDictionary>& obj)
	{
		for (auto& modEntry : mod->entries)
		{
			SPtr<ManagedSerializableFieldData> key = modEntry.key;

			SPtr<ManagedSerializableFieldData> origData = obj->GetFieldData(key);
			SPtr<ManagedSerializableFieldData> newData = applyDiff(modEntry.modification, obj->GetTypeInfo()->mValueType, origData);

			if (newData != nullptr)
				obj->SetFieldData(key, newData);
		}

		for (auto& key : mod->removed)
		{
			obj->removeFieldData(key);
		}

		return nullptr;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDiff::ApplyDiff(const SPtr<Modification>& mod, const SPtr<ManagedSerializableTypeInfo>& fieldType,
		const SPtr<ManagedSerializableFieldData>& origData)
	{
		SPtr<ManagedSerializableFieldData> newData;
		switch (mod->GetTypeId())
		{
		case TID_ScriptModifiedObject:
		{
			SPtr<ManagedSerializableFieldDataObject> origObjData = std::static_pointer_cast<ManagedSerializableFieldDataObject>(origData);
			SPtr<ManagedSerializableObject> childObj = origObjData->value;

			SPtr<ManagedSerializableTypeInfoObject> objTypeInfo =
				std::static_pointer_cast<ManagedSerializableTypeInfoObject>(fieldType);

			if (childObj == nullptr) // Object was deleted in original but we have modifications for it, so we create it
			{
				childObj = ManagedSerializableObject::createNew(objTypeInfo);
				newData = ManagedSerializableFieldData::Create(objTypeInfo, childObj->GetManagedInstance());
			}

			SPtr<ModifiedObject> childMod = std::static_pointer_cast<ModifiedObject>(mod);
			applyDiff(childMod, childObj);
		}
			break;
		case TID_ScriptModifiedArray:
		{
			if (fieldType->GetTypeId() == TID_SerializableTypeInfoArray)
			{
				SPtr<ManagedSerializableFieldDataArray> origArrayData = std::static_pointer_cast<ManagedSerializableFieldDataArray>(origData);
				SPtr<ManagedSerializableArray> childArray = origArrayData->value;

				SPtr<ManagedSerializableTypeInfoArray> arrayTypeInfo =
					std::static_pointer_cast<ManagedSerializableTypeInfoArray>(fieldType);

				SPtr<ModifiedArray> childMod = std::static_pointer_cast<ModifiedArray>(mod);
				if (childArray == nullptr) // Object was deleted in original but we have modifications for it, so we create it
					childArray = ManagedSerializableArray::createNew(arrayTypeInfo, childMod->origSizes);

				newData = applyDiff(childMod, childArray);
			}
			else if (fieldType->GetTypeId() == TID_SerializableTypeInfoList)
			{
				SPtr<ManagedSerializableFieldDataList> origListData = std::static_pointer_cast<ManagedSerializableFieldDataList>(origData);
				SPtr<ManagedSerializableList> childList = origListData->value;

				SPtr<ManagedSerializableTypeInfoList> listTypeInfo =
					std::static_pointer_cast<ManagedSerializableTypeInfoList>(fieldType);

				SPtr<ModifiedArray> childMod = std::static_pointer_cast<ModifiedArray>(mod);
				if (childList == nullptr) // Object was deleted in original but we have modifications for it, so we create it
					childList = ManagedSerializableList::createNew(listTypeInfo, childMod->origSizes[0]);

				newData = applyDiff(childMod, childList);
			}
		}
			break;
		case TID_ScriptModifiedDictionary:
		{
			SPtr<ManagedSerializableFieldDataDictionary> origObjData = std::static_pointer_cast<ManagedSerializableFieldDataDictionary>(origData);
			SPtr<ManagedSerializableDictionary> childDict = origObjData->value;

			SPtr<ManagedSerializableTypeInfoDictionary> dictTypeInfo =
				std::static_pointer_cast<ManagedSerializableTypeInfoDictionary>(fieldType);

			if (childDict == nullptr) // Object was deleted in original but we have modifications for it, so we create it
			{
				childDict = ManagedSerializableDictionary::createNew(dictTypeInfo);
				newData = ManagedSerializableFieldData::Create(dictTypeInfo, childDict->GetManagedInstance());
			}

			SPtr<ModifiedDictionary> childMod = std::static_pointer_cast<ModifiedDictionary>(mod);
			applyDiff(childMod, childDict);
		}
			break;
		default: // Modified field
		{
			SPtr<ModifiedEntry> childMod = std::static_pointer_cast<ModifiedEntry>(mod);
			newData = childMod->value;
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
}
