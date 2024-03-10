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

using namespace bs;

SPtr<IReflectable> BinaryCloner::Clone(IReflectable* object, bool shallow)
{
	if(object == nullptr)
		return nullptr;

	ObjectReferenceData referenceData;
	if(shallow)
	{
		FrameAllocator& alloc = GetFrameAllocator();

		alloc.MarkFrame();
		GatherReferences(object, alloc, referenceData);
		alloc.Clear();
	}

	SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer bs;
	bs.Encode(object, stream, shallow ? BinarySerializerFlag::Shallow : BinarySerializerFlag::None);

	stream->Seek(0);
	SPtr<IReflectable> clonedObj = bs.Decode(stream, (u32)stream->Size());

	if(shallow)
	{
		FrameAllocator& alloc = GetFrameAllocator();

		alloc.MarkFrame();
		RestoreReferences(clonedObj.get(), alloc, referenceData);
		alloc.Clear();
	}

	return clonedObj;
}

void BinaryCloner::GatherReferences(IReflectable* object, FrameAllocator& alloc, ObjectReferenceData& referenceData)
{
	if(object == nullptr)
		return;

	RTTITypeBase* rtti = object->GetRtti();
	Stack<RTTITypeBase*> rttiInstances;
	while(rtti != nullptr)
	{
		RTTITypeBase* rttiInstance = rtti->CloneInternal(alloc);

		rttiInstance->OnSerializationStarted(object, nullptr);
		SubObjectReferenceData* subObjectData = nullptr;

		u32 numFields = rtti->GetFieldCount();
		for(u32 i = 0; i < numFields; i++)
		{
			RTTIField* field = rtti->GetField(i);
			FieldId fieldId;
			fieldId.Field = field;
			fieldId.ArrayIdx = -1;

			if(field->Schema.IsArray)
			{
				const u32 numElements = field->GetArraySize(rttiInstance, object);

				for(u32 j = 0; j < numElements; j++)
				{
					fieldId.ArrayIdx = j;

					if(field->Schema.Type == SerializableFT_ReflectablePtr)
					{
						auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(field);
						SPtr<IReflectable> childObj = curField->GetArrayValue(rttiInstance, object, j);

						if(childObj != nullptr)
						{
							if(subObjectData == nullptr)
							{
								referenceData.SubObjectData.push_back(SubObjectReferenceData());
								subObjectData = &referenceData.SubObjectData[referenceData.SubObjectData.size() - 1];
								subObjectData->Rtti = rtti;
							}

							subObjectData->References.push_back(ObjectReference());
							ObjectReference& reference = subObjectData->References.back();
							reference.FieldId = fieldId;
							reference.Object = childObj;
						}
					}
					else if(field->Schema.Type == SerializableFT_Reflectable)
					{
						auto* curField = static_cast<RTTIReflectableFieldBase*>(field);
						IReflectable* childObj = &curField->GetArrayValue(rttiInstance, object, j);

						if(subObjectData == nullptr)
						{
							referenceData.SubObjectData.push_back(SubObjectReferenceData());
							subObjectData = &referenceData.SubObjectData[referenceData.SubObjectData.size() - 1];
							subObjectData->Rtti = rtti;
						}

						subObjectData->Children.push_back(ObjectReferenceData());
						ObjectReferenceData& childData = subObjectData->Children.back();
						childData.FieldId = fieldId;

						GatherReferences(childObj, alloc, childData);
					}
				}
			}
			else
			{
				if(field->Schema.Type == SerializableFT_ReflectablePtr)
				{
					auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(field);
					SPtr<IReflectable> childObj = curField->GetValue(rttiInstance, object);

					if(childObj != nullptr)
					{
						if(subObjectData == nullptr)
						{
							referenceData.SubObjectData.push_back(SubObjectReferenceData());
							subObjectData = &referenceData.SubObjectData[referenceData.SubObjectData.size() - 1];
							subObjectData->Rtti = rtti;
						}

						subObjectData->References.push_back(ObjectReference());
						ObjectReference& reference = subObjectData->References.back();
						reference.FieldId = fieldId;
						reference.Object = childObj;
					}
				}
				else if(field->Schema.Type == SerializableFT_Reflectable)
				{
					auto* curField = static_cast<RTTIReflectableFieldBase*>(field);
					IReflectable* childObj = &curField->GetValue(rttiInstance, object);

					if(subObjectData == nullptr)
					{
						referenceData.SubObjectData.push_back(SubObjectReferenceData());
						subObjectData = &referenceData.SubObjectData[referenceData.SubObjectData.size() - 1];
						subObjectData->Rtti = rtti;
					}

					subObjectData->Children.push_back(ObjectReferenceData());
					ObjectReferenceData& childData = subObjectData->Children.back();
					childData.FieldId = fieldId;

					GatherReferences(childObj, alloc, childData);
				}
			}
		}

		rttiInstances.push(rttiInstance);
		rtti = rtti->GetBaseClass();
	}

	while(!rttiInstances.empty())
	{
		RTTITypeBase* rttiInstance = rttiInstances.top();
		rttiInstances.pop();

		rttiInstance->OnSerializationEnded(object, nullptr);
		alloc.Destruct(rttiInstance);
	}
}

void BinaryCloner::RestoreReferences(IReflectable* object, FrameAllocator& alloc, const ObjectReferenceData& referenceData)
{
	for(auto iter = referenceData.SubObjectData.rbegin(); iter != referenceData.SubObjectData.rend(); ++iter)
	{
		const SubObjectReferenceData& subObject = *iter;

		if(!subObject.References.empty())
		{
			RTTITypeBase* rttiInstance = subObject.Rtti->CloneInternal(alloc);
			rttiInstance->OnDeserializationStarted(object, nullptr);

			for(auto& reference : subObject.References)
			{
				auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(reference.FieldId.Field);

				if(curField->Schema.IsArray)
					curField->SetArrayValue(rttiInstance, object, reference.FieldId.ArrayIdx, reference.Object);
				else
					curField->SetValue(rttiInstance, object, reference.Object);
			}

			rttiInstance->OnDeserializationEnded(object, nullptr);
			alloc.Destruct(rttiInstance);
		}
	}

	for(auto& subObject : referenceData.SubObjectData)
	{
		if(!subObject.Children.empty())
		{
			RTTITypeBase* rttiInstance = subObject.Rtti->CloneInternal(alloc);
			rttiInstance->OnSerializationStarted(object, nullptr);

			for(auto& childObjectData : subObject.Children)
			{
				auto* curField = static_cast<RTTIReflectableFieldBase*>(childObjectData.FieldId.Field);

				IReflectable* childObj = nullptr;
				if(curField->Schema.IsArray)
					childObj = &curField->GetArrayValue(rttiInstance, object, childObjectData.FieldId.ArrayIdx);
				else
					childObj = &curField->GetValue(rttiInstance, object);

				RestoreReferences(childObj, alloc, childObjectData);
			}

			rttiInstance->OnSerializationEnded(object, nullptr);
			alloc.Destruct(rttiInstance);
		}
	}
}
