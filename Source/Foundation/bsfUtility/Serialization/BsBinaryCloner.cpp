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
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"

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

		SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>();
		BinarySerializer bs;
		bs.encode(object, stream, shallow ? BinarySerializerFlag::Shallow : BinarySerializerFlag::None);

		stream->seek(0);
		SPtr<IReflectable> clonedObj = bs.decode(stream, (UINT32)stream->size());

		if (shallow)
		{
			FrameAlloc& alloc = gFrameAlloc();

			alloc.markFrame();
			restoreReferences(clonedObj.get(), alloc, referenceData);
			alloc.clear();
		}

		return clonedObj;
	}

	void BinaryCloner::gatherReferences(IReflectable* object, FrameAlloc& alloc, ObjectReferenceData& referenceData)
	{
		if (object == nullptr)
			return;

		RTTITypeBase* rtti = object->getRTTI();
		Stack<RTTITypeBase*> rttiInstances;
		while (rtti != nullptr)
		{
			RTTITypeBase* rttiInstance = rtti->_clone(alloc);

			rttiInstance->onSerializationStarted(object, nullptr);
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

			rttiInstance->onSerializationEnded(object, nullptr);
			alloc.destruct(rttiInstance);
		}
	}

	void BinaryCloner::restoreReferences(IReflectable* object, FrameAlloc& alloc, const ObjectReferenceData& referenceData)
	{
		for(auto iter = referenceData.subObjectData.rbegin(); iter != referenceData.subObjectData.rend(); ++iter)
		{
			const SubObjectReferenceData& subObject = *iter;

			if (!subObject.references.empty())
			{
				RTTITypeBase* rttiInstance = subObject.rtti->_clone(alloc);
				rttiInstance->onDeserializationStarted(object, nullptr);

				for (auto& reference : subObject.references)
				{
					auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(reference.fieldId.field);

					if (curField->isArray())
						curField->setArrayValue(rttiInstance, object, reference.fieldId.arrayIdx, reference.object);
					else
						curField->setValue(rttiInstance, object, reference.object);
				}

				rttiInstance->onDeserializationEnded(object, nullptr);
				alloc.destruct(rttiInstance);
			}
		}

		for (auto& subObject : referenceData.subObjectData)
		{
			if (!subObject.children.empty())
			{
				RTTITypeBase* rttiInstance = subObject.rtti->_clone(alloc);
				rttiInstance->onSerializationStarted(object, nullptr);

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

				rttiInstance->onSerializationEnded(object, nullptr);
				alloc.destruct(rttiInstance);
			}
		}
	}
}
