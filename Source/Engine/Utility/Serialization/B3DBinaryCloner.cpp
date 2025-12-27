//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DBinaryCloner.h"
#include "Reflection/B3DIReflectable.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIField.h"
#include "Serialization/B3DBinarySerializer.h"
#include "FileSystem/B3DDataStream.h"

using namespace b3d;

SPtr<IReflectable> BinaryCloner::Clone(IReflectable* object, bool shallow)
{
	if(object == nullptr)
		return nullptr;

	FrameAllocator& allocator = GetFrameAllocator();
	allocator.MarkFrame();

	ObjectExternalReferences externalReferences;
	if(shallow)
	{
		RTTIOperationContext rttiOperationContext;
		externalReferences = GatherExternalReferences(object, allocator, rttiOperationContext);
	}

	SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer bs;
	bs.Encode(object, stream, shallow ? BinarySerializerFlag::Shallow : BinarySerializerFlag::None);

	stream->Seek(0);
	SPtr<IReflectable> clonedObject = bs.Decode(stream, (u32)stream->Size());

	if(shallow)
	{
		RTTIOperationContext rttiOperationContext;
		RestoreExternalReferences(clonedObject.get(), allocator, externalReferences, rttiOperationContext);
	}

	allocator.Clear();
	return clonedObject;
}

BinaryCloner::ObjectExternalReferences BinaryCloner::GatherExternalReferences(IReflectable* object, FrameAllocator& allocator, RTTIOperationContext& rttiOperationContext)
{
	ObjectExternalReferences externalReferences;

	if(object == nullptr)
		return externalReferences;

	RTTIType* rtti = object->GetRtti();
	Stack<RTTIType*> rttiInstances;
	while(rtti != nullptr)
	{
		RTTIType* rttiInstance = rtti->CloneInternal(allocator);

		rttiInstance->NotifyOperationStarted(*object, RTTIOperationType::GatherReferences, rttiOperationContext);
		SubObjectExternalReferences* subObjectReferences = nullptr;

		auto fnGetSubObjectReferences = [&]()
		{
			if(subObjectReferences == nullptr)
			{
				externalReferences.SubObjectReferences.push_back(SubObjectExternalReferences());
				subObjectReferences = &externalReferences.SubObjectReferences[externalReferences.SubObjectReferences.size() - 1];
				subObjectReferences->Rtti = rtti;
			}

			return *subObjectReferences;
		};

		const u32 fieldCount = rtti->GetFieldCount();
		for(u32 fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
		{
			RTTIField* const field = rtti->GetField(fieldIndex);

			ReferenceId referenceId;
			referenceId.Field = field;

			if(field->Schema.FieldType == RTTIFieldType::Iterable)
			{
				auto* const iteratorField = static_cast<RTTIIteratorField*>(field);
				const SPtr<IRTTIIterator> iterator = iteratorField->GetIterator(rttiInstance, object, allocator);

				u32 arrayIndex = 0;
				while(iterator != nullptr && iterator->IsValid())
				{
					const void* fieldValue = iterator->GetValue();

					if(field->Schema.IsContainer)
					{
						if(iteratorField->IteratorSupportsSeekToIndex())
							referenceId.ArrayIndex = arrayIndex;
						else if(B3D_ENSURE(iteratorField->IteratorSupportsSeekToKey()))
							referenceId.MapKey = fieldValue;
					}

					for(u32 tupleElementIndex = 0; tupleElementIndex < (u32)field->Schema.FieldDataTypes.Size(); ++tupleElementIndex)
					{
						const RTTIFieldDataTypeSchema& fieldTypeSchema = field->Schema.FieldDataTypes[tupleElementIndex];

						referenceId.TupleElementIndex = tupleElementIndex;

						if(fieldTypeSchema.Type == RTTIFieldDataType::ReflectablePointer)
						{
							SPtr<IReflectable> childObject = iteratorField->GetReflectablePointer(fieldValue, tupleElementIndex);

							if(childObject != nullptr)
							{
								ObjectReference reference;
								reference.Id = referenceId;
								reference.Object = childObject;

								fnGetSubObjectReferences().References.push_back(reference);

							}
						}
						else if(fieldTypeSchema.Type == RTTIFieldDataType::Reflectable)
						{
							const IReflectable& childObject = iteratorField->GetReflectable(fieldValue, tupleElementIndex);

							ObjectExternalReferences childObjectReferences = GatherExternalReferences(const_cast<IReflectable*>(&childObject), allocator, rttiOperationContext);
							childObjectReferences.Id = referenceId;

							fnGetSubObjectReferences().ChildObjects.push_back(childObjectReferences);
						}
					}

					iterator->Increment();
					arrayIndex++;
				}
			}
		}

		rttiInstances.push(rttiInstance);
		rtti = rtti->GetBaseClass();
	}

	while(!rttiInstances.empty())
	{
		RTTIType* rttiInstance = rttiInstances.top();
		rttiInstances.pop();

		rttiInstance->NotifyOperationEnded(*object, RTTIOperationType::GatherReferences, rttiOperationContext);
		allocator.Destruct(rttiInstance);
	}

	return externalReferences;
}

void BinaryCloner::RestoreExternalReferences(IReflectable* object, FrameAllocator& allocator, const ObjectExternalReferences& externalReferences, RTTIOperationContext& rttiOperationContext)
{
	for(auto it = externalReferences.SubObjectReferences.rbegin(); it != externalReferences.SubObjectReferences.rend(); ++it)
	{
		const SubObjectExternalReferences& subObject = *it;

		if(!subObject.References.empty())
		{
			RTTIType* rttiInstance = subObject.Rtti->CloneInternal(allocator);
			rttiInstance->NotifyOperationStarted(*object, RTTIOperationType::Patch, rttiOperationContext);

			for(auto& reference : subObject.References)
			{
				RTTIField* const field = reference.Id.Field;
				if(B3D_ENSURE(field->Schema.FieldType == RTTIFieldType::Iterable))
				{
					auto* const iteratorField = static_cast<RTTIIteratorField*>(field);
					const SPtr<IRTTIIterator> iterator = iteratorField->GetIterator(rttiInstance, object, allocator);

					if(field->Schema.IsContainer)
					{
						if(iteratorField->IteratorSupportsSeekToIndex())
						{
							B3D_ASSERT(reference.Id.ArrayIndex != ~0u);
							if(!B3D_ENSURE(iterator->SeekToIndex(reference.Id.ArrayIndex)))
								continue;
						}
						else if(B3D_ENSURE(iteratorField->IteratorSupportsSeekToKey()))
						{
							B3D_ASSERT(reference.Id.MapKey != nullptr);
							if(!B3D_ENSURE(iterator->SeekToKey(reference.Id.MapKey)))
								continue;
						}
					}

					void* fieldValue = iteratorField->GetIteratorValueCopy(rttiInstance, object, allocator, *iterator);

					if(!B3D_ENSURE(reference.Id.TupleElementIndex < (u32)field->Schema.FieldDataTypes.Size()))
						continue;

					const RTTIFieldDataTypeSchema& fieldTypeSchema = field->Schema.FieldDataTypes[reference.Id.TupleElementIndex];
					if(!B3D_ENSURE(fieldTypeSchema.Type == RTTIFieldDataType::ReflectablePointer))
						continue;

					iteratorField->SetReflectablePointer(fieldValue, reference.Id.TupleElementIndex, reference.Object);
					iteratorField->SetIteratorValue(rttiInstance, object, allocator, *iterator, fieldValue);
				}
			}

			rttiInstance->NotifyOperationEnded(*object, RTTIOperationType::Patch, rttiOperationContext);
			allocator.Destruct(rttiInstance);
		}
	}

	for(auto& subObject : externalReferences.SubObjectReferences)
	{
		if(!subObject.ChildObjects.empty())
		{
			RTTIType* rttiInstance = subObject.Rtti->CloneInternal(allocator);
			rttiInstance->NotifyOperationStarted(*object, RTTIOperationType::GatherReferences, rttiOperationContext);

			for(auto& childObjectReferences : subObject.ChildObjects)
			{
				RTTIField* const field = childObjectReferences.Id.Field;

				if(B3D_ENSURE(field->Schema.FieldType == RTTIFieldType::Iterable))
				{
					auto* const iteratorField = static_cast<RTTIIteratorField*>(field);
					const SPtr<IRTTIIterator> iterator = iteratorField->GetIterator(rttiInstance, object, allocator);

					if(field->Schema.IsContainer)
					{
						if(iteratorField->IteratorSupportsSeekToIndex())
						{
							B3D_ASSERT(childObjectReferences.Id.ArrayIndex != ~0u);
							if(!B3D_ENSURE(iterator->SeekToIndex(childObjectReferences.Id.ArrayIndex)))
								continue;
						}
						else if(B3D_ENSURE(iteratorField->IteratorSupportsSeekToKey()))
						{
							B3D_ASSERT(childObjectReferences.Id.MapKey != nullptr);
							if(!B3D_ENSURE(iterator->SeekToKey(childObjectReferences.Id.MapKey)))
								continue;
						}
					}

					const void* fieldValue = iteratorField->GetIteratorValue(rttiInstance, object, allocator, *iterator);

					if(!B3D_ENSURE(childObjectReferences.Id.TupleElementIndex < (u32)field->Schema.FieldDataTypes.Size()))
						continue;

					const RTTIFieldDataTypeSchema& fieldTypeSchema = field->Schema.FieldDataTypes[childObjectReferences.Id.TupleElementIndex];

					if(!B3D_ENSURE(fieldTypeSchema.Type == RTTIFieldDataType::Reflectable))
						continue;

					const IReflectable& childObject = iteratorField->GetReflectable(fieldValue, childObjectReferences.Id.TupleElementIndex);

					RestoreExternalReferences(const_cast<IReflectable*>(&childObject), allocator, childObjectReferences, rttiOperationContext);
				}
			}

			rttiInstance->NotifyOperationEnded(*object, RTTIOperationType::GatherReferences, rttiOperationContext);
			allocator.Destruct(rttiInstance);
		}
	}
}
