//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsBinaryCloner.h"
#include "Reflection/BsIReflectable.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIField.h"
#include "Reflection/BsRTTIPlainField.h"
#include "Reflection/BsRTTIReflectableField.h"
#include "Reflection/BsRTTIReflectablePtrField.h"
#include "Reflection/BsRTTIManagedDataBlockField.h"
#include "Serialization/BsMemorySerializer.h"

namespace bs
{
	SPtr<IReflectable> BinaryCloner::clone(IReflectable* object, bool shallow)
	{
		if (object == nullptr)
			return nullptr;

		ObjectReferenceData referenceData;
		if (shallow)
		{
			FrameAlloc& alloc = gFrameAlloc();

			alloc.markFrame();
			gatherReferences(object, alloc, referenceData);
			alloc.clear();
		}

		std::function<void*(UINT32)> allocator = &MemoryAllocator<GenAlloc>::allocate;

		MemorySerializer ms;
		UINT32 dataSize = 0;
		UINT8* data = ms.encode(object, dataSize, allocator, shallow);
		SPtr<IReflectable> clonedObj = ms.decode(data, dataSize);

		if (shallow)
		{
			FrameAlloc& alloc = gFrameAlloc();

			alloc.markFrame();
			restoreReferences(clonedObj.get(), alloc, referenceData);
			alloc.clear();
		}

		bs_free(data);
		return clonedObj;
	}

	void BinaryCloner::gatherReferences(IReflectable* object, FrameAlloc& alloc, ObjectReferenceData& referenceData)
	{
		static const UnorderedMap<String, UINT64> dummyParams;

		if (object == nullptr)
			return;

		RTTITypeBase* rtti = object->getRTTI();
		Stack<RTTITypeBase*> rttiInstances;
		while (rtti != nullptr)
		{
			RTTITypeBase* rttiInstance = rtti->_clone(alloc);

			rttiInstance->onSerializationStarted(object, dummyParams);
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
					const UINT32 numElements = field->getArraySize(rttiInstance, object);

					for (UINT32 j = 0; j < numElements; j++)
					{
						fieldId.arrayIdx = j;

						if (field->mType == SerializableFT_ReflectablePtr)
						{
							auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(field);
							SPtr<IReflectable> childObj = curField->getArrayValue(rttiInstance, object, j);

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
							auto* curField = static_cast<RTTIReflectableFieldBase*>(field);
							IReflectable* childObj = &curField->getArrayValue(rttiInstance, object, j);
							
							if (subObjectData == nullptr)
							{
								referenceData.subObjectData.push_back(SubObjectReferenceData());
								subObjectData = &referenceData.subObjectData[referenceData.subObjectData.size() - 1];
								subObjectData->rtti = rtti;
							}

							subObjectData->children.push_back(ObjectReferenceData());
							ObjectReferenceData& childData = subObjectData->children.back();
							childData.fieldId = fieldId;

							gatherReferences(childObj, alloc, childData);
						}
					}
				}
				else
				{
					if (field->mType == SerializableFT_ReflectablePtr)
					{
						auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(field);
						SPtr<IReflectable> childObj = curField->getValue(rttiInstance, object);

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
						auto* curField = static_cast<RTTIReflectableFieldBase*>(field);
						IReflectable* childObj = &curField->getValue(rttiInstance, object);

						if (subObjectData == nullptr)
						{
							referenceData.subObjectData.push_back(SubObjectReferenceData());
							subObjectData = &referenceData.subObjectData[referenceData.subObjectData.size() - 1];
							subObjectData->rtti = rtti;
						}

						subObjectData->children.push_back(ObjectReferenceData());
						ObjectReferenceData& childData = subObjectData->children.back();
						childData.fieldId = fieldId;

						gatherReferences(childObj, alloc, childData);
					}
				}
			}

			rttiInstances.push(rttiInstance);
			rtti = rtti->getBaseClass();
		}

		while (!rttiInstances.empty())
		{
			RTTITypeBase* rttiInstance = rttiInstances.top();
			rttiInstances.pop();

			rttiInstance->onSerializationEnded(object, dummyParams);
			alloc.destruct(rttiInstance);
		}
	}

	void BinaryCloner::restoreReferences(IReflectable* object, FrameAlloc& alloc, const ObjectReferenceData& referenceData)
	{
		static const UnorderedMap<String, UINT64> dummyParams;

		for(auto iter = referenceData.subObjectData.rbegin(); iter != referenceData.subObjectData.rend(); ++iter)
		{
			const SubObjectReferenceData& subObject = *iter;

			if (!subObject.references.empty())
			{
				RTTITypeBase* rttiInstance = subObject.rtti->_clone(alloc);
				rttiInstance->onDeserializationStarted(object, dummyParams);

				for (auto& reference : subObject.references)
				{
					auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(reference.fieldId.field);

					if (curField->isArray())
						curField->setArrayValue(rttiInstance, object, reference.fieldId.arrayIdx, reference.object);
					else
						curField->setValue(rttiInstance, object, reference.object);
				}

				rttiInstance->onDeserializationEnded(object, dummyParams);
				alloc.destruct(rttiInstance);
			}
		}

		for (auto& subObject : referenceData.subObjectData)
		{
			if (!subObject.children.empty())
			{
				RTTITypeBase* rttiInstance = subObject.rtti->_clone(alloc);
				rttiInstance->onSerializationStarted(object, dummyParams);

				for (auto& childObjectData : subObject.children)
				{
					auto* curField = static_cast<RTTIReflectableFieldBase*>(childObjectData.fieldId.field);

					IReflectable* childObj = nullptr;
					if (curField->isArray())
						childObj = &curField->getArrayValue(rttiInstance, object, childObjectData.fieldId.arrayIdx);
					else
						childObj = &curField->getValue(rttiInstance, object);

					restoreReferences(childObj, alloc, childObjectData);
				}

				rttiInstance->onSerializationEnded(object, dummyParams);
				alloc.destruct(rttiInstance);
			}
		}
	}
}