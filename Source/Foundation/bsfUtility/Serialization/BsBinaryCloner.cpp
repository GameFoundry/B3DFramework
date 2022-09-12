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
	SPtr<IReflectable> BinaryCloner::Clone(IReflectable* object, bool shallow)
	{
		if (object == nullptr)
			return nullptr;

		ObjectReferenceData referenceData;
		if (shallow)
		{
			FrameAlloc& alloc = gFrameAlloc();

			alloc.MarkFrame();
			gatherReferences(object, alloc, referenceData);
			alloc.Clear();
		}

		SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>();
		BinarySerializer bs;
		bs.Encode(object, stream, shallow ? BinarySerializerFlag::Shallow : BinarySerializerFlag::None);

		stream->Seek(0);
		SPtr<IReflectable> clonedObj = bs.Decode(stream, (UINT32)stream->Size());

		if (shallow)
		{
			FrameAlloc& alloc = gFrameAlloc();

			alloc.MarkFrame();
			restoreReferences(clonedObj.Get(), alloc, referenceData);
			alloc.Clear();
		}

		return clonedObj;
	}

	void BinaryCloner::GatherReferences(IReflectable* object, FrameAlloc& alloc, ObjectReferenceData& referenceData)
	{
		if (object == nullptr)
			return;

		RTTITypeBase* rtti = object->GetRTTI();
		Stack<RTTITypeBase*> rttiInstances;
		while (rtti != nullptr)
		{
			RTTITypeBase* rttiInstance = rtti->_clone(alloc);

			rttiInstance->OnSerializationStarted(object, nullptr);
			SubObjectReferenceData* subObjectData = nullptr;

			UINT32 numFields = rtti->GetNumFields();
			for (UINT32 i = 0; i < numFields; i++)
			{
				RTTIField* field = rtti->GetField(i);
				FieldId fieldId;
				fieldId.field = field;
				fieldId.arrayIdx = -1;

				if (field->schema.isArray)
				{
					const UINT32 numElements = field->GetArraySize(rttiInstance, object);

					for (UINT32 j = 0; j < numElements; j++)
					{
						fieldId.arrayIdx = j;

						if (field->schema.type == SerializableFT_ReflectablePtr)
						{
							auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(field);
							SPtr<IReflectable> childObj = curField->GetArrayValue(rttiInstance, object, j);

							if (childObj != nullptr)
							{
								if (subObjectData == nullptr)
								{
									referenceData.subObjectData.push_back(SubObjectReferenceData());
									subObjectData = &referenceData.subObjectData[referenceData.subObjectData.Size() - 1];
									subObjectData->rtti = rtti;
								}

								subObjectData->references.push_back(ObjectReference());
								ObjectReference& reference = subObjectData->references.Back();
								reference.fieldId = fieldId;
								reference.object = childObj;
							}
						}
						else if (field->schema.type == SerializableFT_Reflectable)
						{
							auto* curField = static_cast<RTTIReflectableFieldBase*>(field);
							IReflectable* childObj = &curField->GetArrayValue(rttiInstance, object, j);
							
							if (subObjectData == nullptr)
							{
								referenceData.subObjectData.push_back(SubObjectReferenceData());
								subObjectData = &referenceData.subObjectData[referenceData.subObjectData.Size() - 1];
								subObjectData->rtti = rtti;
							}

							subObjectData->children.push_back(ObjectReferenceData());
							ObjectReferenceData& childData = subObjectData->children.Back();
							childData.fieldId = fieldId;

							gatherReferences(childObj, alloc, childData);
						}
					}
				}
				else
				{
					if (field->schema.type == SerializableFT_ReflectablePtr)
					{
						auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(field);
						SPtr<IReflectable> childObj = curField->GetValue(rttiInstance, object);

						if (childObj != nullptr)
						{
							if (subObjectData == nullptr)
							{
								referenceData.subObjectData.push_back(SubObjectReferenceData());
								subObjectData = &referenceData.subObjectData[referenceData.subObjectData.Size() - 1];
								subObjectData->rtti = rtti;
							}

							subObjectData->references.push_back(ObjectReference());
							ObjectReference& reference = subObjectData->references.Back();
							reference.fieldId = fieldId;
							reference.object = childObj;
						}
					}
					else if (field->schema.type == SerializableFT_Reflectable)
					{
						auto* curField = static_cast<RTTIReflectableFieldBase*>(field);
						IReflectable* childObj = &curField->GetValue(rttiInstance, object);

						if (subObjectData == nullptr)
						{
							referenceData.subObjectData.push_back(SubObjectReferenceData());
							subObjectData = &referenceData.subObjectData[referenceData.subObjectData.Size() - 1];
							subObjectData->rtti = rtti;
						}

						subObjectData->children.push_back(ObjectReferenceData());
						ObjectReferenceData& childData = subObjectData->children.Back();
						childData.fieldId = fieldId;

						gatherReferences(childObj, alloc, childData);
					}
				}
			}

			rttiInstances.Push(rttiInstance);
			rtti = rtti->GetBaseClass();
		}

		while (!rttiInstances.Empty())
		{
			RTTITypeBase* rttiInstance = rttiInstances.Top();
			rttiInstances.Pop();

			rttiInstance->OnSerializationEnded(object, nullptr);
			alloc.Destruct(rttiInstance);
		}
	}

	void BinaryCloner::RestoreReferences(IReflectable* object, FrameAlloc& alloc, const ObjectReferenceData& referenceData)
	{
		for(auto iter = referenceData.subObjectData.Rbegin(); iter != referenceData.subObjectData.rend(); ++iter)
		{
			const SubObjectReferenceData& subObject = *iter;

			if (!subObject.references.Empty())
			{
				RTTITypeBase* rttiInstance = subObject.rtti->_clone(alloc);
				rttiInstance->OnDeserializationStarted(object, nullptr);

				for (auto& reference : subObject.references)
				{
					auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(reference.fieldId.field);

					if (curField->schema.isArray)
						curField->SetArrayValue(rttiInstance, object, reference.fieldId.arrayIdx, reference.object);
					else
						curField->SetValue(rttiInstance, object, reference.object);
				}

				rttiInstance->OnDeserializationEnded(object, nullptr);
				alloc.Destruct(rttiInstance);
			}
		}

		for (auto& subObject : referenceData.subObjectData)
		{
			if (!subObject.children.Empty())
			{
				RTTITypeBase* rttiInstance = subObject.rtti->_clone(alloc);
				rttiInstance->OnSerializationStarted(object, nullptr);

				for (auto& childObjectData : subObject.children)
				{
					auto* curField = static_cast<RTTIReflectableFieldBase*>(childObjectData.fieldId.field);

					IReflectable* childObj = nullptr;
					if (curField->schema.isArray)
						childObj = &curField->GetArrayValue(rttiInstance, object, childObjectData.fieldId.arrayIdx);
					else
						childObj = &curField->GetValue(rttiInstance, object);

					restoreReferences(childObj, alloc, childObjectData);
				}

				rttiInstance->OnSerializationEnded(object, nullptr);
				alloc.Destruct(rttiInstance);
			}
		}
	}
}
