//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsIntermediateSerializer.h"
#include "FileSystem/BsDataStream.h"
#include "Reflection/BsRTTIReflectablePtrField.h"
#include "Reflection/BsRTTIType.h"

using namespace bs;

IntermediateSerializer::IntermediateSerializer()
	: mAllocator(&GetFrameAllocator())
{}

SPtr<IReflectable> IntermediateSerializer::Decode(const SerializedObject* serializedObject, SerializationContext* context)
{
	mContext = context;

	mAllocator->MarkFrame();
	mDeserializedObjectMap.clear();

	SPtr<IReflectable> output;
	RTTITypeBase* type = IReflectable::GetRTTITypeFromTypeId(serializedObject->GetRootTypeId());
	if(type != nullptr)
	{
		output = type->NewRttiObject();
		auto iterNewObj = mDeserializedObjectMap.insert(std::make_pair(serializedObject, ObjectDeserializationData(output, serializedObject)));

		iterNewObj.first->second.DeserializationInProgress = true;
		DeserializeReflectableObject(output, serializedObject);
		iterNewObj.first->second.DeserializationInProgress = false;
		iterNewObj.first->second.IsDeserialized = true;
	}

	// Go through the remaining objects (should be only ones with weak refs)
	for(auto iter = mDeserializedObjectMap.begin(); iter != mDeserializedObjectMap.end(); ++iter)
	{
		ObjectDeserializationData& objToDecode = iter->second;

		if(objToDecode.IsDeserialized)
			continue;

		objToDecode.DeserializationInProgress = true;
		DeserializeReflectableObject(objToDecode.Object, objToDecode.SerializedObject);
		objToDecode.DeserializationInProgress = false;
		objToDecode.IsDeserialized = true;
	}

	mDeserializedObjectMap.clear();
	mAllocator->Clear();

	return output;
}

SPtr<SerializedObject> IntermediateSerializer::Encode(IReflectable* object, SerializedObjectEncodeFlags flags, SerializationContext* context)
{
	mContext = context;

	return SerializeReflectableObject(*object, flags, context, *mAllocator);
}

void IntermediateSerializer::DeserializeReflectableObject(const SPtr<IReflectable>& object, const SerializedObject* serializableObject)
{
	const u32 subobjectCount = (u32)serializableObject->SubObjects.size();
	if(subobjectCount == 0)
		return;

	FrameStack<RTTITypeBase*> rttiInstances;
	for(i32 subObjectIdx = (i32)subobjectCount - 1; subObjectIdx >= 0; subObjectIdx--)
	{
		const SerializedSubObject& subObject = serializableObject->SubObjects[subObjectIdx];

		RTTITypeBase* rtti = IReflectable::GetRTTITypeFromTypeId(subObject.TypeId);
		if(rtti == nullptr)
			continue;

		RTTITypeBase* rttiInstance = rtti->CloneInternal(*mAllocator);
		rttiInstance->OnDeserializationStarted(object.get(), mContext);
		rttiInstances.push(rttiInstance);

		const u32 fieldCount = rtti->GetFieldCount();
		for(u32 fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
		{
			RTTIField* curGenericField = rtti->GetField(fieldIndex);

			auto iterFindFieldData = subObject.FieldEntries.find(curGenericField->Schema.Id);
			if(iterFindFieldData == subObject.FieldEntries.end())
				continue;

			RTTIIteratorField* iteratorField = nullptr;
			if(curGenericField->Schema.IsIterator)
				iteratorField = static_cast<RTTIIteratorField*>(curGenericField);

			SPtr<IRTTIIterator> iterator;
			if(iteratorField != nullptr)
				iterator = iteratorField->GetIterator(rttiInstance, object.get(), *mAllocator);

			const SPtr<ISerialized>& serializedEntry = iterFindFieldData->second.Value;
			if(curGenericField->Schema.IsArray)
			{
				SPtr<SerializedArray> serializedArray = std::static_pointer_cast<SerializedArray>(serializedEntry);

				if(iteratorField == nullptr)
				{
					const u32 elementCount = (u32)serializedArray->ElementCount;
					curGenericField->SetArraySize(rttiInstance, object.get(), elementCount);
				}

				for(const auto& serializedArrayEntryPair : serializedArray->Entries)
				{
					SPtr<ISerialized> serializedArrayEntryValue = serializedArrayEntryPair.second.Value;
					SPtr<SerializedTuple> serializedTuple;

					const bool isTuple = curGenericField->Schema.FieldTypes.Size() > 1;
					if(isTuple)
						serializedTuple = std::static_pointer_cast<SerializedTuple>(serializedArrayEntryValue);

					void* fieldValue = nullptr;
					if(iteratorField != nullptr && iterator != nullptr)
						fieldValue = iteratorField->CreateEmptyFieldValue(*mAllocator);

					for(u32 fieldTypeIndex = 0; fieldTypeIndex < (u32)curGenericField->Schema.FieldTypes.Size(); ++fieldTypeIndex)
					{
						SPtr<ISerialized> serializedTupleValue;
						if(isTuple)
						{
							if(B3D_ENSURE(serializedTuple != nullptr && fieldTypeIndex > serializedTuple->Values.Size()))
								serializedTupleValue = serializedTuple->Values[fieldTypeIndex];
						}
						else
						{
							serializedTupleValue = serializedArrayEntryValue;
						}

						switch(curGenericField->Schema.Type)
						{
						case SerializableFT_ReflectablePtr:
							{
								SPtr<SerializedObject> referencedSerializedObject = std::static_pointer_cast<SerializedObject>(serializedTupleValue);

								SPtr<IReflectable> referencedObject;
								if(curGenericField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef))
									referencedObject = GetReflectableObject(referencedSerializedObject);
								else
									referencedObject = GetOrDeserializeReflectableObject(referencedSerializedObject);

								if(iteratorField != nullptr)
								{
									iteratorField->SetReflectablePointer(fieldValue, fieldTypeIndex, referencedObject);
								}
								else
								{
									auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);
									curField->SetArrayValue(rttiInstance, object.get(), serializedArrayEntryPair.first, referencedObject);
								}
							}
							break;
						case SerializableFT_Reflectable:
							{
								SPtr<SerializedObject> referencedSerializedObject = std::static_pointer_cast<SerializedObject>(serializedTupleValue);
								RTTITypeBase* childRtti = nullptr;

								if(referencedSerializedObject != nullptr)
									childRtti = IReflectable::GetRTTITypeFromTypeId(referencedSerializedObject->GetRootTypeId());

								if(childRtti != nullptr)
								{
									SPtr<IReflectable> newObject = childRtti->NewRttiObject();
									DeserializeReflectableObject(newObject, referencedSerializedObject.get());

									if(iteratorField != nullptr)
									{
										iteratorField->SetReflectable(fieldValue, fieldTypeIndex, *newObject);
									}
									else
									{
										auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);
										curField->SetArrayValue(rttiInstance, object.get(), serializedArrayEntryPair.first, *newObject);
									}
								}
								break;
							}
						case SerializableFT_Plain:
							{
								SPtr<SerializedPlainData> serializedPlainData = std::static_pointer_cast<SerializedPlainData>(serializedTupleValue);
								if(serializedPlainData != nullptr)
								{
									Bitstream tempStream(serializedPlainData->Value, serializedPlainData->Size);
									if(iteratorField != nullptr)
									{
										iteratorField->ReadPlainTypeTupleFromStream(fieldValue, fieldTypeIndex, tempStream, false);
									}
									else
									{
										auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);
										curField->ArrayElemFromBuffer(rttiInstance, object.get(), serializedArrayEntryPair.first, tempStream);
									}
								}
							}
							break;
						default:
							break;
						}
					}

					if(iteratorField != nullptr && iterator != nullptr)
						iteratorField->SetIteratorValue(rttiInstance, object.get(), *mAllocator, *iterator, fieldValue);
				}
			}
			else
			{
				SPtr<SerializedTuple> serializedTuple;

				const bool isTuple = curGenericField->Schema.FieldTypes.Size() > 1;
				if(isTuple)
					serializedTuple = std::static_pointer_cast<SerializedTuple>(serializedEntry);

				void* fieldValue = nullptr;
				if(iteratorField != nullptr && iterator != nullptr)
					fieldValue = iteratorField->CreateEmptyFieldValue(*mAllocator);

				for(u32 fieldTypeIndex = 0; fieldTypeIndex < (u32)curGenericField->Schema.FieldTypes.Size(); ++fieldTypeIndex)
				{
					SPtr<ISerialized> serializedTupleValue;
					if(isTuple)
					{
						if(B3D_ENSURE(serializedTuple != nullptr && fieldTypeIndex > serializedTuple->Values.Size()))
							serializedTupleValue = serializedTuple->Values[fieldTypeIndex];
					}
					else
					{
						serializedTupleValue = serializedEntry;
					}

					// TODO - Can be unified with the array code above
					switch(curGenericField->Schema.Type)
					{
					case SerializableFT_ReflectablePtr:
						{
							SPtr<SerializedObject> referencedSerializedObject = std::static_pointer_cast<SerializedObject>(serializedTupleValue);

							SPtr<IReflectable> referencedObject;
							if(curGenericField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef))
								referencedObject = GetReflectableObject(referencedSerializedObject);
							else
								referencedObject = GetOrDeserializeReflectableObject(referencedSerializedObject);

							if(iteratorField != nullptr)
							{
								iteratorField->SetReflectablePointer(fieldValue, fieldTypeIndex, referencedObject);
							}
							else
							{
								auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);
								curField->SetValue(rttiInstance, object.get(), referencedObject);
							}
						}
						break;
					case SerializableFT_Reflectable:
						{
							SPtr<SerializedObject> referencedSerializedObject = std::static_pointer_cast<SerializedObject>(serializedTupleValue);
							RTTITypeBase* childRtti = nullptr;

							if(referencedSerializedObject != nullptr)
								childRtti = IReflectable::GetRTTITypeFromTypeId(referencedSerializedObject->GetRootTypeId());

							if(childRtti != nullptr)
							{
								SPtr<IReflectable> newObject = childRtti->NewRttiObject();
								DeserializeReflectableObject(newObject, referencedSerializedObject.get());

								if(iteratorField != nullptr)
								{
									iteratorField->SetReflectable(fieldValue, fieldTypeIndex, *newObject);
								}
								else
								{
									auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);
									curField->SetValue(rttiInstance, object.get(), *newObject);
								}
							}
							break;
						}
					case SerializableFT_Plain:
						{
							SPtr<SerializedPlainData> serializedPlainData = std::static_pointer_cast<SerializedPlainData>(serializedTupleValue);
							if(serializedPlainData != nullptr)
							{
								Bitstream tempStream(serializedPlainData->Value, serializedPlainData->Size);
								if(iteratorField != nullptr)
								{
									iteratorField->ReadPlainTypeTupleFromStream(fieldValue, fieldTypeIndex, tempStream, false);
								}
								else
								{
									auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);
									curField->FromBuffer(rttiInstance, object.get(), tempStream);
								}
							}
						}
						break;
					case SerializableFT_DataBlock:
						{
							auto* curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

							SPtr<SerializedDataBlock> serializedDataBlock = std::static_pointer_cast<SerializedDataBlock>(serializedTupleValue);
							if(serializedDataBlock != nullptr)
							{
								serializedDataBlock->Stream->Seek(serializedDataBlock->Offset);
								curField->SetValue(rttiInstance, object.get(), serializedDataBlock->Stream, serializedDataBlock->Size);
							}

							break;
						}
					}
				}

				if(iteratorField != nullptr && iterator != nullptr)
					iteratorField->SetIteratorValue(rttiInstance, object.get(), *mAllocator, *iterator, fieldValue);
			}
		}
	}

	while(!rttiInstances.empty())
	{
		RTTITypeBase* rttiInstance = rttiInstances.top();
		rttiInstance->OnDeserializationEnded(object.get(), mContext);
		mAllocator->Destruct(rttiInstance);

		rttiInstances.pop();
	}
}

SPtr<SerializedObject> IntermediateSerializer::SerializeReflectableObject(const IReflectable& object, SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAllocator& allocator)
{
	FrameStack<RTTITypeBase*> rttiInstances;
	RTTITypeBase* rtti = object.GetRtti();

	const auto cleanup = [&]()
	{
		while(!rttiInstances.empty())
		{
			RTTITypeBase* rttiInstance = rttiInstances.top();
			rttiInstance->OnSerializationEnded(const_cast<IReflectable*>(&object), context);
			allocator.Destruct(rttiInstance);

			rttiInstances.pop();
		}
	};

	bool replicableOnly = flags.IsSet(SerializedObjectEncodeFlag::ReplicableOnly);
	SPtr<SerializedObject> output = B3DMakeShared<SerializedObject>();

	// If an object has base classes, we need to iterate through all of them
	do
	{
		RTTITypeBase* rttiInstance = rtti->CloneInternal(allocator);
		rttiInstances.push(rttiInstance);

		rttiInstance->OnSerializationStarted(const_cast<IReflectable*>(&object), context);

		output->SubObjects.push_back(SerializedSubObject());
		SerializedSubObject& subObject = output->SubObjects.back();
		subObject.TypeId = rtti->GetRttiId();

		const u32 fieldCount = rtti->GetFieldCount();
		for(u32 fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
		{
			RTTIField* const field = rtti->GetField(fieldIndex);

			if(replicableOnly)
			{
				if(!field->Schema.Info.Flags.IsSet(RTTIFieldFlag::Replicate))
					continue;
			}

			SPtr<ISerialized> serializedEntry;
			if(field->Schema.IsIterator)
				serializedEntry = SerializeField(const_cast<IReflectable&>(object), *rttiInstance, static_cast<RTTIIteratorField&>(*field), flags, context, allocator);
			else
				serializedEntry = SerializeField(const_cast<IReflectable*>(&object), rttiInstance, field, (u32)-1, flags, context, &allocator);

			SerializedField entry;
			entry.FieldId = field->Schema.Id;
			entry.Value = serializedEntry;

			subObject.FieldEntries.insert(std::make_pair(field->Schema.Id, entry));
		}

		rtti = rtti->GetBaseClass();
	}
	while(rtti != nullptr); // Repeat until we reach the top of the inheritance hierarchy

	cleanup();

	return output;
}

SPtr<ISerialized> IntermediateSerializer::SerializeField(IReflectable* object, RTTITypeBase* rtti, RTTIField* field, u32 arrayIdx, SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAllocator* alloc)
{
	bool shallow = flags.IsSet(SerializedObjectEncodeFlag::Shallow);

	SPtr<ISerialized> output;
	if(field->Schema.IsArray)
	{
		const u32 arrayElementCount = field->GetArraySize(rtti, object);
		bool wholeArray = arrayIdx == (u32)-1;

		u32 arrayEndIdx;
		if(wholeArray)
		{
			arrayIdx = 0;
			arrayEndIdx = arrayElementCount;
		}
		else
			arrayEndIdx = arrayIdx + 1;

		SPtr<SerializedArray> serializedArray;
		if(wholeArray)
		{
			serializedArray = B3DMakeShared<SerializedArray>();
			serializedArray->ElementCount = arrayElementCount;

			output = serializedArray;
		}

		switch(field->Schema.Type)
		{
		case SerializableFT_ReflectablePtr:
			{
				auto curField = static_cast<RTTIReflectablePtrFieldBase*>(field);

				for(u32 arrIdx = arrayIdx; arrIdx < arrayEndIdx; arrIdx++)
				{
					SPtr<SerializedObject> serializedChildObject = nullptr;

					if(!shallow)
					{
						SPtr<IReflectable> childObject = curField->GetArrayValue(rtti, object, arrIdx);

						if(childObject)
							serializedChildObject = SerializeReflectableObject(*childObject, flags, context, *alloc);
					}

					if(wholeArray)
					{
						SerializedArrayEntry arrayEntry;
						arrayEntry.Value = serializedChildObject;
						arrayEntry.Index = arrIdx;

						serializedArray->Entries[arrIdx] = arrayEntry;
					}
					else
						output = serializedChildObject;
				}

				break;
			}
		case SerializableFT_Reflectable:
			{
				auto curField = static_cast<RTTIReflectableFieldBase*>(field);

				for(u32 arrIdx = 0; arrIdx < arrayElementCount; arrIdx++)
				{
					IReflectable& childObject = curField->GetArrayValue(rtti, object, arrIdx);

					const SPtr<SerializedObject> serializedChildObject = SerializeReflectableObject(childObject, flags, context, *alloc);

					if(wholeArray)
					{
						SerializedArrayEntry arrayEntry;
						arrayEntry.Value = serializedChildObject;
						arrayEntry.Index = arrIdx;

						serializedArray->Entries[arrIdx] = arrayEntry;
					}
					else
						output = serializedChildObject;
				}

				break;
			}
		case SerializableFT_Plain:
			{
				auto curField = static_cast<RTTIPlainFieldBase*>(field);

				for(u32 arrIdx = 0; arrIdx < arrayElementCount; arrIdx++)
				{
					u32 typeSize = 0;
					if(curField->Schema.HasDynamicSize)
						typeSize = curField->GetArrayElemDynamicSize(rtti, object, arrIdx, false).Bytes;
					else
						typeSize = curField->Schema.Size.Bytes;

					const auto serializedPlainData = B3DMakeShared<SerializedPlainData>();
					serializedPlainData->Value = (u8*)B3DAllocate(typeSize);
					serializedPlainData->OwnsMemory = true;
					serializedPlainData->Size = typeSize;

					Bitstream tempStream(serializedPlainData->Value, typeSize);
					curField->ArrayElemToStream(rtti, object, arrIdx, tempStream);

					if(wholeArray)
					{
						SerializedArrayEntry arrayEntry;
						arrayEntry.Value = serializedPlainData;
						arrayEntry.Index = arrIdx;

						serializedArray->Entries[arrIdx] = arrayEntry;
					}
					else
						output = serializedPlainData;
				}

				break;
			}
		default:
			B3D_EXCEPT(InternalErrorException, "Error encoding data. Encountered a type I don't know how to encode. Type: " + ToString(u32(field->Schema.Type)) + ", Is array: " + ToString(field->Schema.IsArray));
		}
	}
	else
	{
		switch(field->Schema.Type)
		{
		case SerializableFT_ReflectablePtr:
			{
				auto curField = static_cast<RTTIReflectablePtrFieldBase*>(field);

				if(!shallow)
				{
					SPtr<IReflectable> childObject = curField->GetValue(rtti, object);

					if(childObject)
						output = SerializeReflectableObject(*childObject, flags, context, *alloc);
				}

				break;
			}
		case SerializableFT_Reflectable:
			{
				auto curField = static_cast<RTTIReflectableFieldBase*>(field);
				IReflectable& childObject = curField->GetValue(rtti, object);

				output = SerializeReflectableObject(childObject, flags, context, *alloc);

				break;
			}
		case SerializableFT_Plain:
			{
				auto curField = static_cast<RTTIPlainFieldBase*>(field);

				u32 typeSize = 0;
				if(curField->Schema.HasDynamicSize)
					typeSize = curField->GetDynamicSize(rtti, object, false).Bytes;
				else
					typeSize = curField->Schema.Size.Bytes;

				const auto serializedField = B3DMakeShared<SerializedPlainData>();
				serializedField->Value = (u8*)B3DAllocate(typeSize);
				serializedField->OwnsMemory = true;
				serializedField->Size = typeSize;

				Bitstream tempStream(serializedField->Value, typeSize);
				curField->ToStream(rtti, object, tempStream);

				output = serializedField;

				break;
			}
		case SerializableFT_DataBlock:
			{
				auto curField = static_cast<RTTIManagedDataBlockFieldBase*>(field);

				u32 dataBlockSize = 0;
				SPtr<DataStream> blockStream = curField->GetValue(rtti, object, dataBlockSize);

				SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>(dataBlockSize);
				blockStream->Read(stream->Data(), dataBlockSize);

				SPtr<SerializedDataBlock> serializedDataBlock = B3DMakeShared<SerializedDataBlock>();
				serializedDataBlock->Stream = stream;
				serializedDataBlock->Offset = 0;

				serializedDataBlock->Size = dataBlockSize;
				output = serializedDataBlock;

				break;
			}
		default:
			B3D_EXCEPT(InternalErrorException, "Error encoding data. Encountered a type I don't know how to encode. Type: " + ToString(u32(field->Schema.Type)) + ", Is array: " + ToString(field->Schema.IsArray));
		}
	}

	return output;
}

SPtr<ISerialized> IntermediateSerializer::SerializeField(IReflectable& object, RTTITypeBase& rttiType, RTTIIteratorField& field, SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAllocator& allocator)
{
	SPtr<ISerialized> output;
	if(field.Schema.IsArray)
	{
		if(!B3D_ENSURE(field.Schema.IsIterator))
			return nullptr;

		const SPtr<IRTTIIterator> iterator = field.GetIterator(&rttiType, &object, allocator);

		SPtr<SerializedArray> serializedArray;
		if(iterator != nullptr)
		{
			serializedArray = B3DMakeShared<SerializedArray>();
			serializedArray->ElementCount = (u32)iterator->GetElementCount();

			output = serializedArray;

			for(u32 arrayIndex = 0; iterator->IsValid(); iterator->Increment(), arrayIndex++)
			{
				SerializedArrayEntry serializedArrayEntry;
				serializedArrayEntry.Index = arrayIndex;

				SPtr<SerializedTuple> serializedTuple;
				if(field.Schema.FieldTypes.Size() > 1)
					serializedTuple = B3DMakeShared<SerializedTuple>();

				for(u32 typeIndex = 0; typeIndex < (u32)field.Schema.FieldTypes.Size(); ++typeIndex)
				{
					SPtr<ISerialized> serialized = SerializeTupleElement(field, *iterator, typeIndex, flags, context, allocator);

					if(serializedTuple != nullptr)
						serializedTuple->Values.Add(serialized);
					else
						serializedArrayEntry.Value = serialized;
				}

				if(serializedTuple)
					serializedArrayEntry.Value = serializedTuple;

				serializedArray->Entries[arrayIndex] = std::move(serializedArrayEntry);
			}
		}
	}
	else
	{
		B3D_ENSURE(false); // TODO - Non-container fields not supported yet. Use the other SerializeField overload.
	}

	return output;
}

SPtr<ISerialized> IntermediateSerializer::SerializeTupleElement(RTTIIteratorField& field, IRTTIIterator& iterator, u32 tupleElementIndex, SerializedObjectEncodeFlags flags, SerializationContext* context, FrameAllocator& allocator)
{
	if(!B3D_ENSURE(iterator.IsValid()))
		return nullptr;

	if(!B3D_ENSURE(field.Schema.FieldTypes.Size() >= tupleElementIndex))
		return nullptr;

	const bool shallow = flags.IsSet(SerializedObjectEncodeFlag::Shallow);
	const RTTIFieldTypeSchema& typeSchema = field.Schema.FieldTypes[tupleElementIndex];

	const void* fieldValue = iterator.GetValue();

	switch(typeSchema.Type)
	{
	case SerializableFT_ReflectablePtr:
		{
			if(!shallow)
			{
				const SPtr<IReflectable> referencedObject = field.GetReflectablePointer(fieldValue, tupleElementIndex);

				if(referencedObject)
					return SerializeReflectableObject(*referencedObject, flags, context, allocator);
			}

			return nullptr;
		}
	case SerializableFT_Reflectable:
		{
			const IReflectable& inlineObject = field.GetReflectable(fieldValue, tupleElementIndex);
			return SerializeReflectableObject(inlineObject, flags, context, allocator);
		}
	case SerializableFT_Plain:
		{
			u32 typeSize = 0;
			if(typeSchema.HasDynamicSize)
				typeSize = field.GetPlainTypeSize(fieldValue, tupleElementIndex, false).Bytes;
			else
				typeSize = typeSchema.FixedSize.Bytes;

			const auto serializedPlainData = B3DMakeShared<SerializedPlainData>();
			serializedPlainData->Value = (u8*)B3DAllocate(typeSize);
			serializedPlainData->OwnsMemory = true;
			serializedPlainData->Size = typeSize;

			Bitstream tempStream(serializedPlainData->Value, typeSize);
			field.WritePlainTypeTupleToStream(fieldValue, tupleElementIndex, tempStream, false);

			return serializedPlainData;
		}
	default:
		B3D_LOG(Error, Serialization, "Error serializing data. Encountered a type I don't know how to encode. Type: {0}, Is array: {1}", typeSchema.Type, field.Schema.IsArray);
		return nullptr;
	}
}

SPtr<IReflectable> IntermediateSerializer::GetOrDeserializeReflectableObject(const SPtr<SerializedObject>& serializedObject)
{
	if(serializedObject == nullptr)
		return nullptr;

	auto found = mDeserializedObjectMap.find(serializedObject.get());
	if(found == mDeserializedObjectMap.end())
	{
		RTTITypeBase* objectRttiType = nullptr;

		if(serializedObject != nullptr)
			objectRttiType = IReflectable::GetRTTITypeFromTypeId(serializedObject->GetRootTypeId());

		if(objectRttiType == nullptr)
			return nullptr;

		SPtr<IReflectable> newObject = objectRttiType->NewRttiObject();
		found = mDeserializedObjectMap.insert(std::make_pair(serializedObject.get(), ObjectDeserializationData(newObject, serializedObject.get()))).first;
	}

	ObjectDeserializationData& objectDeserializationData = found->second;
	if(!objectDeserializationData.IsDeserialized)
	{
		if(objectDeserializationData.DeserializationInProgress)
		{
			B3D_LOG(Warning, Generic, "Detected a circular reference when decoding. "
									  "Referenced object's fields will be resolved in an undefined order "
									  "(i.e. one of the objects will not be fully deserialized when assigned "
									  "to its field). Use RTTI_Flag_WeakRef to get rid of this warning and tell "
									  "the system which of the objects is allowed to be deserialized after it "
									  "is assigned to its field.");
		}
		else
		{
			objectDeserializationData.DeserializationInProgress = true;
			DeserializeReflectableObject(objectDeserializationData.Object, objectDeserializationData.SerializedObject);
			objectDeserializationData.DeserializationInProgress = false;
			objectDeserializationData.IsDeserialized = true;
		}
	}

	return objectDeserializationData.Object;
}

SPtr<IReflectable> IntermediateSerializer::GetReflectableObject(const SPtr<SerializedObject>& serializedObject)
{
	auto found = mDeserializedObjectMap.find(serializedObject.get());
	if(found != mDeserializedObjectMap.end())
		return found->second.Object;

	return nullptr;
}

