//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsBinaryCloner.h"
#include "BsIReflectable.h"
#include "BsRTTIType.h"
#include "BsRTTIField.h"
#include "BsRTTIPlainField.h"
#include "BsRTTIReflectableField.h"
#include "BsRTTIReflectablePtrField.h"
#include "BsRTTIManagedDataBlockField.h"
#include "BsMemorySerializer.h"

namespace BansheeEngine
{
	SPtr<IReflectable> BinaryCloner::clone(IReflectable* object, bool shallow)
	{
		if (object == nullptr)
			return nullptr;

		ObjectReferenceData referenceData;
		if (shallow)
			gatherReferences(object, referenceData);

		std::function<void*(UINT32)> allocator = &MemoryAllocator<GenAlloc>::allocate;

		MemorySerializer ms;
		UINT32 dataSize = 0;
		UINT8* data = ms.encode(object, dataSize, allocator, shallow);
		SPtr<IReflectable> clonedObj = ms.decode(data, dataSize);

		if (shallow)
			restoreReferences(clonedObj.get(), referenceData);

		bs_free(data);
		return clonedObj;
	}

	void BinaryCloner::gatherReferences(IReflectable* object, ObjectReferenceData& referenceData)
	{
		static const UnorderedMap<String, UINT64> dummyParams;

		if (object == nullptr)
			return;

		RTTITypeBase* rtti = object->getRTTI();
		Stack<RTTITypeBase*> rttiTypes;
		while (rtti != nullptr)
		{
			rtti->onSerializationStarted(object, dummyParams);
			SubObjectReferenceData* subObjectData = nullptr;

			UINT32 numFields = rtti->getNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* field = rtti->getField(i);
				FieldId fieldId;
				fieldId.field = field;
				fieldId.arrayIdx = -1;

				if (field->isArray())
				{
					UINT32 numElements = field->getArraySize(object);

					for (UINT32 j = 0; j < numElements; j++)
					{
						fieldId.arrayIdx = j;

						if (field->mType == SerializableFT_ReflectablePtr)
						{
							RTTIReflectablePtrFieldBase* curField = static_cast<RTTIReflectablePtrFieldBase*>(field);
							SPtr<IReflectable> childObj = curField->getArrayValue(object, j);

							if (childObj != nullptr)
							{
								if (subObjectData == nullptr)
								{
									referenceData.subObjectData.push_back(SubObjectReferenceData());
									subObjectData = &referenceData.subObjectData[referenceData.subObjectData.size() - 1];
									subObjectData->rtti = rtti;
								}

								subObjectData->references.push_back(ObjectReference());
								ObjectReference& reference = subObjectData->references.back();
								reference.fieldId = fieldId;
								reference.object = childObj;
							}
						}
						else if (field->mType == SerializableFT_Reflectable)
						{
							RTTIReflectableFieldBase* curField = static_cast<RTTIReflectableFieldBase*>(field);
							IReflectable* childObj = &curField->getArrayValue(object, j);
							
							if (subObjectData == nullptr)
							{
								referenceData.subObjectData.push_back(SubObjectReferenceData());
								subObjectData = &referenceData.subObjectData[referenceData.subObjectData.size() - 1];
								subObjectData->rtti = rtti;
							}

							subObjectData->children.push_back(ObjectReferenceData());
							ObjectReferenceData& childData = subObjectData->children.back();
							childData.fieldId = fieldId;

							gatherReferences(childObj, childData);
						}
					}
				}
				else
				{
					if (field->mType == SerializableFT_ReflectablePtr)
					{
						RTTIReflectablePtrFieldBase* curField = static_cast<RTTIReflectablePtrFieldBase*>(field);
						SPtr<IReflectable> childObj = curField->getValue(object);

						if (childObj != nullptr)
						{
							if (subObjectData == nullptr)
							{
								referenceData.subObjectData.push_back(SubObjectReferenceData());
								subObjectData = &referenceData.subObjectData[referenceData.subObjectData.size() - 1];
								subObjectData->rtti = rtti;
							}

							subObjectData->references.push_back(ObjectReference());
							ObjectReference& reference = subObjectData->references.back();
							reference.fieldId = fieldId;
							reference.object = childObj;
						}
					}
					else if (field->mType == SerializableFT_Reflectable)
					{
						RTTIReflectableFieldBase* curField = static_cast<RTTIReflectableFieldBase*>(field);
						IReflectable* childObj = &curField->getValue(object);

						if (subObjectData == nullptr)
						{
							referenceData.subObjectData.push_back(SubObjectReferenceData());
							subObjectData = &referenceData.subObjectData[referenceData.subObjectData.size() - 1];
							subObjectData->rtti = rtti;
						}

						subObjectData->children.push_back(ObjectReferenceData());
						ObjectReferenceData& childData = subObjectData->children.back();
						childData.fieldId = fieldId;

						gatherReferences(childObj, childData);
					}
				}
			}

			rttiTypes.push(rtti);
			rtti = rtti->getBaseClass();
		}

		while (!rttiTypes.empty())
		{
			rtti = rttiTypes.top();
			rttiTypes.pop();

			rtti->onSerializationEnded(object, dummyParams);
		}
	}

	void BinaryCloner::restoreReferences(IReflectable* object, const ObjectReferenceData& referenceData)
	{
		static const UnorderedMap<String, UINT64> dummyParams;

		for(auto iter = referenceData.subObjectData.rbegin(); iter != referenceData.subObjectData.rend(); ++iter)
		{
			const SubObjectReferenceData& subObject = *iter;

			if (subObject.references.size() > 0)
			{
				subObject.rtti->onDeserializationStarted(object, dummyParams);

				for (auto& reference : subObject.references)
				{
					RTTIReflectablePtrFieldBase* curField = static_cast<RTTIReflectablePtrFieldBase*>(reference.fieldId.field);

					if (curField->isArray())
						curField->setArrayValue(object, reference.fieldId.arrayIdx, reference.object);
					else
						curField->setValue(object, reference.object);
				}

				subObject.rtti->onDeserializationEnded(object, dummyParams);
			}
		}

		for (auto& subObject : referenceData.subObjectData)
		{
			if (subObject.children.size() > 0)
			{
				subObject.rtti->onSerializationStarted(object, dummyParams);

				for (auto& childObjectData : subObject.children)
				{
					RTTIReflectableFieldBase* curField = static_cast<RTTIReflectableFieldBase*>(childObjectData.fieldId.field);

					IReflectable* childObj = nullptr;
					if (curField->isArray())
						childObj = &curField->getArrayValue(object, childObjectData.fieldId.arrayIdx);
					else
						childObj = &curField->getValue(object);

					restoreReferences(childObj, childObjectData);
				}

				subObject.rtti->onSerializationEnded(object, dummyParams);
			}
		}
	}
}