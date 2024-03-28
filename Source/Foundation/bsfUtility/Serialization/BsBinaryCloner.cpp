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

	FrameAllocator& allocator = GetFrameAllocator();
	allocator.MarkFrame();

	ObjectExternalReferences externalReferences;
	if(shallow)
		externalReferences = GatherExternalReferences(object, allocator);

	SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>();
	BinarySerializer bs;
	bs.Encode(object, stream, shallow ? BinarySerializerFlag::Shallow : BinarySerializerFlag::None);

	stream->Seek(0);
	SPtr<IReflectable> clonedObject = bs.Decode(stream, (u32)stream->Size());

	if(shallow)
		RestoreExternalReferences(clonedObject.get(), allocator, externalReferences);

	allocator.Clear();
	return clonedObject;
}

BinaryCloner::ObjectExternalReferences BinaryCloner::GatherExternalReferences(IReflectable* object, FrameAllocator& allocator)
{
	ObjectExternalReferences externalReferences;

	if(object == nullptr)
		return externalReferences;

	RTTITypeBase* rtti = object->GetRtti();
	Stack<RTTITypeBase*> rttiInstances;
	while(rtti != nullptr)
	{
		RTTITypeBase* rttiInstance = rtti->CloneInternal(allocator);

		rttiInstance->OnSerializationStarted(object, nullptr);
		SubObjectExternalReferences* subObjectReferences = nullptr;

		auto fnGetSubObjectReferences = [&subObjectReferences, &externalReferences, rtti]()
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

			if(field->Schema.IsIterator)
			{
				auto* const iteratorField = static_cast<RTTIIteratorField*>(field);
				const SPtr<IRTTIIterator> iterator = iteratorField->GetIterator(rttiInstance, object, allocator);

				u32 arrayIndex = 0;
				while(iterator != nullptr && iterator->IsValid())
				{
					const void* fieldValue = iterator->GetValue();

					if(field->Schema.IsArray)
					{
						if(iteratorField->IteratorSupportsSeekToIndex())
							referenceId.ArrayIndex = arrayIndex;
						else if(B3D_ENSURE(iteratorField->IteratorSupportsSeekToKey()))
							referenceId.MapKey = fieldValue;
					}

					for(u32 tupleElementIndex = 0; tupleElementIndex < (u32)field->Schema.FieldTypes.Size(); ++tupleElementIndex)
					{
						const RTTIFieldTypeSchema& fieldTypeSchema = field->Schema.FieldTypes[tupleElementIndex];

						referenceId.TupleElementIndex = tupleElementIndex;

						if(fieldTypeSchema.Type == SerializableFT_ReflectablePtr)
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
						else if(fieldTypeSchema.Type == SerializableFT_Reflectable)
						{
							const IReflectable& childObject = iteratorField->GetReflectable(fieldValue, tupleElementIndex);

							ObjectExternalReferences childObjectReferences = GatherExternalReferences(const_cast<IReflectable*>(&childObject), allocator);
							childObjectReferences.Id = referenceId;

							fnGetSubObjectReferences().ChildObjects.push_back(childObjectReferences);
						}
					}

					iterator->Increment();
					arrayIndex++;
				}
			}
			else // DEPRECATED
			{
				referenceId.TupleElementIndex = 0; // Not supporting tuple elements in this case

				if(field->Schema.IsArray)
				{
					const u32 arraySize = field->GetArraySize(rttiInstance, object);

					for(u32 arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
					{
						referenceId.ArrayIndex = arrayIndex;

						if(field->Schema.Type == SerializableFT_ReflectablePtr)
						{
							auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(field);
							SPtr<IReflectable> childObj = curField->GetArrayValue(rttiInstance, object, arrayIndex);

							if(childObj != nullptr)
							{
								ObjectReference reference;
								reference.Id = referenceId;
								reference.Object = childObj;

								fnGetSubObjectReferences().References.push_back(reference);
							}
						}
						else if(field->Schema.Type == SerializableFT_Reflectable)
						{
							auto* curField = static_cast<RTTIReflectableFieldBase*>(field);
							IReflectable* childObj = &curField->GetArrayValue(rttiInstance, object, arrayIndex);

							ObjectExternalReferences childExternalReferences = GatherExternalReferences(childObj, allocator);
							childExternalReferences.Id = referenceId;

							fnGetSubObjectReferences().ChildObjects.push_back(childExternalReferences);
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
							ObjectReference reference;
							reference.Id = referenceId;
							reference.Object = childObj;

							fnGetSubObjectReferences().References.push_back(reference);
						}
					}
					else if(field->Schema.Type == SerializableFT_Reflectable)
					{
						auto* curField = static_cast<RTTIReflectableFieldBase*>(field);
						IReflectable* childObj = &curField->GetValue(rttiInstance, object);

						ObjectExternalReferences childExternalReferences = GatherExternalReferences(childObj, allocator);
						childExternalReferences.Id = referenceId;

						fnGetSubObjectReferences().ChildObjects.push_back(childExternalReferences);
					}
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
		allocator.Destruct(rttiInstance);
	}

	return externalReferences;
}

void BinaryCloner::RestoreExternalReferences(IReflectable* object, FrameAllocator& allocator, const ObjectExternalReferences& externalReferences)
{
	for(auto it = externalReferences.SubObjectReferences.rbegin(); it != externalReferences.SubObjectReferences.rend(); ++it)
	{
		const SubObjectExternalReferences& subObject = *it;

		if(!subObject.References.empty())
		{
			RTTITypeBase* rttiInstance = subObject.Rtti->CloneInternal(allocator);
			rttiInstance->OnDeserializationStarted(object, nullptr);

			for(auto& reference : subObject.References)
			{
				RTTIField* const field = reference.Id.Field;
				if(field->Schema.IsIterator)
				{
					auto* const iteratorField = static_cast<RTTIIteratorField*>(field);
					const SPtr<IRTTIIterator> iterator = iteratorField->GetIterator(rttiInstance, object, allocator);

					if(field->Schema.IsArray)
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

					if(!B3D_ENSURE(reference.Id.TupleElementIndex < (u32)field->Schema.FieldTypes.Size()))
						continue;

					const RTTIFieldTypeSchema& fieldTypeSchema = field->Schema.FieldTypes[reference.Id.TupleElementIndex];
					if(!B3D_ENSURE(fieldTypeSchema.Type == SerializableFT_ReflectablePtr))
						continue;

					iteratorField->SetReflectablePointer(fieldValue, reference.Id.TupleElementIndex, reference.Object);
					iteratorField->SetIteratorValue(rttiInstance, object, allocator, *iterator, fieldValue);
				}
				else // DEPRECATED
				{
					auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(reference.Id.Field);
					if(curField->Schema.IsArray)
						curField->SetArrayValue(rttiInstance, object, reference.Id.ArrayIndex, reference.Object);
					else
						curField->SetValue(rttiInstance, object, reference.Object);
				}
			}

			rttiInstance->OnDeserializationEnded(object, nullptr);
			allocator.Destruct(rttiInstance);
		}
	}

	for(auto& subObject : externalReferences.SubObjectReferences)
	{
		if(!subObject.ChildObjects.empty())
		{
			RTTITypeBase* rttiInstance = subObject.Rtti->CloneInternal(allocator);
			rttiInstance->OnSerializationStarted(object, nullptr);

			for(auto& childObjectReferences : subObject.ChildObjects)
			{
				RTTIField* const field = childObjectReferences.Id.Field;

				if(field->Schema.IsIterator)
				{
					auto* const iteratorField = static_cast<RTTIIteratorField*>(field);
					const SPtr<IRTTIIterator> iterator = iteratorField->GetIterator(rttiInstance, object, allocator);

					if(field->Schema.IsArray)
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

					if(!B3D_ENSURE(childObjectReferences.Id.TupleElementIndex < (u32)field->Schema.FieldTypes.Size()))
						continue;

					const RTTIFieldTypeSchema& fieldTypeSchema = field->Schema.FieldTypes[childObjectReferences.Id.TupleElementIndex];

					if(!B3D_ENSURE(fieldTypeSchema.Type == SerializableFT_Reflectable))
						continue;

					const IReflectable& childObject = iteratorField->GetReflectable(fieldValue, childObjectReferences.Id.TupleElementIndex);

					RestoreExternalReferences(const_cast<IReflectable*>(&childObject), allocator, childObjectReferences);
				}
				else // DEPRECATED
				{
					auto* curField = static_cast<RTTIReflectableFieldBase*>(childObjectReferences.Id.Field);

					IReflectable* childObject = nullptr;
					if(curField->Schema.IsArray)
						childObject = &curField->GetArrayValue(rttiInstance, object, childObjectReferences.Id.ArrayIndex);
					else
						childObject = &curField->GetValue(rttiInstance, object);

					RestoreExternalReferences(childObject, allocator, childObjectReferences);
				}
			}

			rttiInstance->OnSerializationEnded(object, nullptr);
			allocator.Destruct(rttiInstance);
		}
	}
}
