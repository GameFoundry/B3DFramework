//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsIntermediateSerializer.h"
#include "FileSystem/BsDataStream.h"
#include "Reflection/BsRTTIReflectablePtrField.h"
#include "Reflection/BsRTTIType.h"

using namespace bs;

IntermediateSerializer::IntermediateSerializer(FrameAllocator* allocator, RTTIOperationContext& context)
	: mAllocator(allocator), mContext(context)
{}

SPtr<IReflectable> IntermediateSerializer::Decode(const SerializedObject* serializedObject)
{
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

SPtr<SerializedObject> IntermediateSerializer::Encode(IReflectable* object, SerializedObjectEncodeFlags flags)
{
	mAllocator->MarkFrame();

	SPtr<SerializedObject> output = SerializeReflectableObject(*object, flags);
	mSerializedObjectMap.clear();

	mAllocator->Clear();

	return output;
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
		rttiInstance->NotifyOperationStarted(*object, RTTIOperationType::Deserialization, mContext);
		rttiInstances.push(rttiInstance);

		const u32 fieldCount = rtti->GetFieldCount();
		for(u32 fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
		{
			RTTIField* curGenericField = rtti->GetField(fieldIndex);

			auto iterFindFieldData = subObject.FieldEntries.find(curGenericField->Schema.Id);
			if(iterFindFieldData == subObject.FieldEntries.end())
				continue;

			const SPtr<ISerialized>& serializedFieldValue = iterFindFieldData->second.Value;
			if(curGenericField->Schema.IsIterator)
			{
				RTTIIteratorField* iteratorField = static_cast<RTTIIteratorField*>(curGenericField);
				SPtr<IRTTIIterator> iterator = iteratorField->GetIterator(rttiInstance, object.get(), *mAllocator);

				if(iterator != nullptr)
					iterator->Clear();

				const bool encodedAsArray = iteratorField->Schema.IsArray && iteratorField->IteratorSupportsSeekToIndex();
				const bool encodedAsMap = iteratorField->Schema.IsArray && iteratorField->IteratorSupportsSeekToKey();

				if(encodedAsArray)
				{
					SPtr<SerializedArray> serializedArray = std::static_pointer_cast<SerializedArray>(serializedFieldValue);
					const u64 elementCount = serializedArray != nullptr ? serializedArray->Entries.Size() : 0;

					for(u64 arrayIndex = 0; arrayIndex < elementCount; ++arrayIndex)
					{
						const SPtr<ISerialized>& serializedEntry = serializedArray->Entries[arrayIndex];
						DeserializeElement(*rttiInstance, object, *iteratorField, iterator, serializedEntry);
					}
				}
				else if(encodedAsMap)
				{
					SPtr<SerializedMap> serializedMap = std::static_pointer_cast<SerializedMap>(serializedFieldValue);
					if(serializedMap != nullptr)
					{
						for(auto it = serializedMap->Entries.begin(); it != serializedMap->Entries.end(); ++it)
						{
							DeserializeElement(*rttiInstance, object, *iteratorField, iterator, it->second);
						}
					}
				}
				else
				{
					DeserializeElement(*rttiInstance, object, *iteratorField, iterator, serializedFieldValue);
				}
			}
			else if(curGenericField->Schema.IsArray) // DEPRECATED
			{
				SPtr<SerializedArray> serializedArray = std::static_pointer_cast<SerializedArray>(serializedFieldValue);

				const u32 elementCount = (u32)serializedArray->Entries.Size();
				curGenericField->SetArraySize(rttiInstance, object.get(), elementCount);

				for(u32 elementIndex = 0; elementIndex < elementCount; ++elementIndex)
				{
					SPtr<ISerialized> serializedArrayEntry = serializedArray->Entries[elementIndex];
					SPtr<SerializedTuple> serializedTuple;

					const bool isTuple = curGenericField->Schema.FieldTypes.Size() > 1;
					if(isTuple)
						serializedTuple = std::static_pointer_cast<SerializedTuple>(serializedArrayEntry);

					for(u32 fieldTypeIndex = 0; fieldTypeIndex < (u32)curGenericField->Schema.FieldTypes.Size(); ++fieldTypeIndex)
					{
						RTTIFieldTypeSchema& tupleSchema = curGenericField->Schema.FieldTypes[fieldTypeIndex];

						SPtr<ISerialized> serializedTupleValue;
						if(isTuple)
						{
							if(B3D_ENSURE(serializedTuple != nullptr && fieldTypeIndex > serializedTuple->Values.Size()))
								serializedTupleValue = serializedTuple->Values[fieldTypeIndex];
						}
						else
						{
							serializedTupleValue = serializedArrayEntry;
						}

						switch(tupleSchema.Type)
						{
						case SerializableFT_ReflectablePtr:
							{
								SPtr<SerializedObject> referencedSerializedObject = std::static_pointer_cast<SerializedObject>(serializedTupleValue);

								SPtr<IReflectable> referencedObject;
								if(curGenericField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef))
									referencedObject = GetReflectableObject(referencedSerializedObject);
								else
									referencedObject = GetOrDeserializeReflectableObject(referencedSerializedObject);

								auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);
								curField->SetArrayValue(rttiInstance, object.get(), elementIndex, referencedObject);
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

									auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);
									curField->SetArrayValue(rttiInstance, object.get(), elementIndex, *newObject);
								}
								break;
							}
						case SerializableFT_Plain:
							{
								SPtr<SerializedPlainData> serializedPlainData = std::static_pointer_cast<SerializedPlainData>(serializedTupleValue);
								if(serializedPlainData != nullptr)
								{
									Bitstream tempStream(serializedPlainData->Value, serializedPlainData->Size);

									auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);
									curField->ArrayElemFromBuffer(rttiInstance, object.get(), elementIndex, tempStream);
								}
							}
							break;
						default:
							break;
						}
					}
				}
			}
			else // All except DataBlock case DEPRECATED
			{
				SPtr<SerializedTuple> serializedTuple;

				const bool isTuple = curGenericField->Schema.FieldTypes.Size() > 1;
				if(isTuple)
					serializedTuple = std::static_pointer_cast<SerializedTuple>(serializedFieldValue);

				for(u32 fieldTypeIndex = 0; fieldTypeIndex < (u32)curGenericField->Schema.FieldTypes.Size(); ++fieldTypeIndex)
				{
					RTTIFieldTypeSchema& tupleSchema = curGenericField->Schema.FieldTypes[fieldTypeIndex];

					SPtr<ISerialized> serializedTupleValue;
					if(isTuple)
					{
						if(B3D_ENSURE(serializedTuple != nullptr && fieldTypeIndex > serializedTuple->Values.Size()))
							serializedTupleValue = serializedTuple->Values[fieldTypeIndex];
					}
					else
					{
						serializedTupleValue = serializedFieldValue;
					}

					switch(tupleSchema.Type)
					{
					case SerializableFT_ReflectablePtr:
						{
							SPtr<SerializedObject> referencedSerializedObject = std::static_pointer_cast<SerializedObject>(serializedTupleValue);

							SPtr<IReflectable> referencedObject;
							if(curGenericField->Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef))
								referencedObject = GetReflectableObject(referencedSerializedObject);
							else
								referencedObject = GetOrDeserializeReflectableObject(referencedSerializedObject);

							auto* curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);
							curField->SetValue(rttiInstance, object.get(), referencedObject);
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

								auto* curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);
								curField->SetValue(rttiInstance, object.get(), *newObject);
							}
							break;
						}
					case SerializableFT_Plain:
						{
							SPtr<SerializedPlainData> serializedPlainData = std::static_pointer_cast<SerializedPlainData>(serializedTupleValue);
							if(serializedPlainData != nullptr)
							{
								Bitstream tempStream(serializedPlainData->Value, serializedPlainData->Size);

								auto* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);
								curField->FromBuffer(rttiInstance, object.get(), tempStream);
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
			}
		}
	}

	while(!rttiInstances.empty())
	{
		RTTITypeBase* rttiInstance = rttiInstances.top();
		rttiInstance->OnDeserializationEnded(object.get(), &mContext);
		rttiInstance->NotifyOperationEnded(*object, RTTIOperationType::Deserialization, mContext);
		mAllocator->Destruct(rttiInstance);

		rttiInstances.pop();
	}
}

void IntermediateSerializer::DeserializeElement(RTTITypeBase& rttiInstance, const SPtr<IReflectable>& object, RTTIIteratorField& field, const SPtr<IRTTIIterator>& iterator, const SPtr<ISerialized>& entry)
{
	if(!B3D_ENSURE(iterator != nullptr))
		return;

	void* fieldValue = field.CreateEmptyFieldValue(*mAllocator);
	DeserializeElement(field, fieldValue, entry);

	iterator->SeekToEnd(); // Ensures value is inserted at the end of the iterable container
	field.SetIteratorValue(&rttiInstance, object.get(), *mAllocator, *iterator, fieldValue);
	field.FreeFieldValue(fieldValue, *mAllocator);
}

void IntermediateSerializer::DeserializeElement(RTTIIteratorField& field, void* outFieldValue, const SPtr<ISerialized>& entry)
{
	SPtr<SerializedTuple> serializedTuple;

	const bool isTuple = field.Schema.FieldTypes.Size() > 1;
	if(isTuple)
		serializedTuple = std::static_pointer_cast<SerializedTuple>(entry);

	for(u32 fieldTypeIndex = 0; fieldTypeIndex < (u32)field.Schema.FieldTypes.Size(); ++fieldTypeIndex)
	{
		RTTIFieldTypeSchema& tupleSchema = field.Schema.FieldTypes[fieldTypeIndex];

		SPtr<ISerialized> serializedTupleValue;
		if(isTuple)
		{
			if(B3D_ENSURE(serializedTuple != nullptr && fieldTypeIndex < serializedTuple->Values.Size()))
				serializedTupleValue = serializedTuple->Values[fieldTypeIndex];
		}
		else
		{
			serializedTupleValue = entry;
		}

		DeserializeTupleElement(field, outFieldValue, fieldTypeIndex, serializedTupleValue);
	}
}

void IntermediateSerializer::DeserializeTupleElement(RTTIIteratorField& field, void* outFieldValue, u32 tupleElementIndex, const SPtr<ISerialized>& entry)
{
	if(!B3D_ENSURE(tupleElementIndex < field.Schema.FieldTypes.Size()))
		return;

	RTTIFieldTypeSchema& tupleElementSchema = field.Schema.FieldTypes[tupleElementIndex];
	switch(tupleElementSchema.Type)
	{
	case SerializableFT_ReflectablePtr:
		{
			SPtr<SerializedObject> referencedSerializedObject = std::static_pointer_cast<SerializedObject>(entry);

			SPtr<IReflectable> referencedObject;
			if(field.Schema.Info.Flags.IsSet(RTTIFieldFlag::WeakRef))
				referencedObject = GetReflectableObject(referencedSerializedObject);
			else
				referencedObject = GetOrDeserializeReflectableObject(referencedSerializedObject);

			field.SetReflectablePointer(outFieldValue, tupleElementIndex, referencedObject);
		}
		break;
	case SerializableFT_Reflectable:
		{
			SPtr<SerializedObject> referencedSerializedObject = std::static_pointer_cast<SerializedObject>(entry);
			RTTITypeBase* childRtti = nullptr;

			if(referencedSerializedObject != nullptr)
				childRtti = IReflectable::GetRTTITypeFromTypeId(referencedSerializedObject->GetRootTypeId());

			if(childRtti != nullptr)
			{
				SPtr<IReflectable> newObject = childRtti->NewRttiObject();
				DeserializeReflectableObject(newObject, referencedSerializedObject.get());

				field.SetReflectable(outFieldValue, tupleElementIndex, *newObject);
			}
			break;
		}
	case SerializableFT_Plain:
		{
			SPtr<SerializedPlainData> serializedPlainData = std::static_pointer_cast<SerializedPlainData>(entry);
			if(serializedPlainData != nullptr)
			{
				Bitstream tempStream(serializedPlainData->Value, serializedPlainData->Size);
				field.ReadPlainTypeTupleFromStream(outFieldValue, tupleElementIndex, tempStream, false);
			}
		}
		break;
	default:
		break;
	}
}

SPtr<SerializedObject> IntermediateSerializer::GetOrSerializeReflectableObject(const IReflectable& object, SerializedObjectEncodeFlags flags)
{
	auto found = mSerializedObjectMap.find(&object);
	if(found != mSerializedObjectMap.end())
		return found->second;

	SPtr<SerializedObject> serializedObject = SerializeReflectableObject(object, flags);
	mSerializedObjectMap[&object] = serializedObject;

	return serializedObject;
}

SPtr<SerializedObject> IntermediateSerializer::SerializeReflectableObject(const IReflectable& object, SerializedObjectEncodeFlags flags)
{
	FrameStack<RTTITypeBase*> rttiInstances;
	RTTITypeBase* rtti = object.GetRtti();

	const auto cleanup = [&]()
	{
		while(!rttiInstances.empty())
		{
			RTTITypeBase* rttiInstance = rttiInstances.top();
			rttiInstance->NotifyOperationEnded(const_cast<IReflectable&>(object), RTTIOperationType::Serialization, mContext);
			mAllocator->Destruct(rttiInstance);

			rttiInstances.pop();
		}
	};

	const bool replicableOnly = flags.IsSet(SerializedObjectEncodeFlag::ReplicableOnly);
	const bool isDeltaCopy = flags.IsSet(SerializedObjectEncodeFlag::IsDeltaCopy);

	SPtr<SerializedObject> output = B3DMakeShared<SerializedObject>();

	// If an object has base classes, we need to iterate through all of them
	do
	{
		RTTITypeBase* rttiInstance = rtti->CloneInternal(*mAllocator);
		rttiInstances.push(rttiInstance);

		rttiInstance->NotifyOperationStarted(const_cast<IReflectable&>(object), RTTIOperationType::Serialization, mContext);

		output->SubObjects.push_back(SerializedSubObject());
		SerializedSubObject& subObject = output->SubObjects.back();
		subObject.TypeId = rtti->GetRttiId();

		const u32 fieldCount = rtti->GetFieldCount();
		for(u32 fieldIndex = 0; fieldIndex < fieldCount; fieldIndex++)
		{
			RTTIField* const field = rtti->GetField(fieldIndex);

			if(replicableOnly && !field->Schema.Info.Flags.IsSet(RTTIFieldFlag::Replicate))
				continue;

			if(isDeltaCopy && field->Schema.Info.Flags.IsSet(RTTIFieldFlag::SkipInDeltaCopy))
				continue;

			SPtr<ISerialized> serializedEntry;
			if(field->Schema.IsIterator)
				serializedEntry = SerializeField(const_cast<IReflectable&>(object), *rttiInstance, static_cast<RTTIIteratorField&>(*field), flags);
			else
				serializedEntry = SerializeField(const_cast<IReflectable*>(&object), rttiInstance, field, (u32)-1, flags);

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

SPtr<ISerialized> IntermediateSerializer::SerializeField(IReflectable* object, RTTITypeBase* rtti, RTTIField* field, u32 arrayIdx, SerializedObjectEncodeFlags flags)
{
	bool shallow = flags.IsSet(SerializedObjectEncodeFlag::Shallow);

	SPtr<ISerialized> output;
	if(field->Schema.IsArray)
	{
		const u32 arrayElementCount = field->GetArraySize(rtti, object);
		bool wholeArray = arrayIdx == ~0u;

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
			serializedArray->Entries.Reserve(arrayElementCount);

			output = serializedArray;
		}

		switch(field->Schema.Type) // TODO - Not handling tuples
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
							serializedChildObject = GetOrSerializeReflectableObject(*childObject, flags);
					}

					if(wholeArray)
						serializedArray->Entries.Add(serializedChildObject);
					else
						output = serializedChildObject;
				}

				break;
			}
		case SerializableFT_Reflectable:
			{
				auto curField = static_cast<RTTIReflectableFieldBase*>(field);

				for(u32 arrIdx = arrayIdx; arrIdx < arrayEndIdx; arrIdx++)
				{
					IReflectable& childObject = curField->GetArrayValue(rtti, object, arrIdx);

					const SPtr<SerializedObject> serializedChildObject = SerializeReflectableObject(childObject, flags);

					if(wholeArray)
						serializedArray->Entries.Add(serializedChildObject);
					else
						output = serializedChildObject;
				}

				break;
			}
		case SerializableFT_Plain:
			{
				auto curField = static_cast<RTTIPlainFieldBase*>(field);

				for(u32 arrIdx = arrayIdx; arrIdx < arrayEndIdx; arrIdx++)
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
						serializedArray->Entries.Add(serializedPlainData);
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
		switch(field->Schema.Type) // TODO - Not handling tuples
		{
		case SerializableFT_ReflectablePtr:
			{
				auto curField = static_cast<RTTIReflectablePtrFieldBase*>(field);

				if(!shallow)
				{
					SPtr<IReflectable> childObject = curField->GetValue(rtti, object);

					if(childObject)
						output = GetOrSerializeReflectableObject(*childObject, flags);
				}

				break;
			}
		case SerializableFT_Reflectable:
			{
				auto curField = static_cast<RTTIReflectableFieldBase*>(field);
				IReflectable& childObject = curField->GetValue(rtti, object);

				output = SerializeReflectableObject(childObject, flags);

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

SPtr<ISerialized> IntermediateSerializer::SerializeField(IReflectable& object, RTTITypeBase& rttiInstance, RTTIIteratorField& field, SerializedObjectEncodeFlags flags)
{
	SPtr<ISerialized> output;
	if(field.Schema.IsIterator)
	{
		const SPtr<IRTTIIterator> iterator = field.GetIterator(&rttiInstance, &object, *mAllocator);

		if(iterator == nullptr)
			return nullptr;

		const bool encodeAsArray = field.Schema.IsArray && field.IteratorSupportsSeekToIndex();
		const bool encodeAsMap = field.Schema.IsArray && field.IteratorSupportsSeekToKey();

		SPtr<SerializedArray> serializedArray;
		SPtr<SerializedMap> serializedMap;
		if(encodeAsArray)
		{
			serializedArray = B3DMakeShared<SerializedArray>();
			serializedArray->Entries.Reserve(iterator->GetElementCount());

			output = serializedArray;
		}
		else if(encodeAsMap)
		{
			serializedMap = B3DMakeShared<SerializedMap>();
			output = serializedMap;
		}

		for(u32 arrayIndex = 0; iterator->IsValid(); iterator->Increment(), arrayIndex++)
		{
			SPtr<ISerialized> serializedEntry = SerializeElement(object, rttiInstance, field, *iterator, flags);

			if(encodeAsArray)
			{
				serializedArray->Entries.Add(std::move(serializedEntry));
			}
			else if(encodeAsMap)
			{
				if(const SPtr<SerializedTuple>& serializedTuple = B3DRTTICast<SerializedTuple>(serializedEntry))
				{
					if(B3D_ENSURE(!serializedTuple->Values.Empty()))
						serializedMap->Entries[serializedTuple->Values[0]] = serializedEntry;
				}
				else
				{
					serializedMap->Entries[serializedEntry] = serializedEntry;
				}
			}
			else
			{
				output = serializedEntry;
			}
		}
	}
	else
	{
		// TODO - Handle DataBlock field here
		B3D_ASSERT(false);
	}

	return output;
}

SPtr<ISerialized> IntermediateSerializer::SerializeElement(IReflectable& object, RTTITypeBase& rttiInstance, RTTIIteratorField& field, IRTTIIterator& iterator, SerializedObjectEncodeFlags flags)
{
	if(!iterator.IsValid())
		return nullptr;

	SPtr<ISerialized> serializedEntry;
	SPtr<SerializedTuple> serializedTuple;
	if(field.Schema.FieldTypes.Size() > 1)
	{
		serializedTuple = B3DMakeShared<SerializedTuple>();
		serializedEntry = serializedTuple;
	}

	for(u32 typeIndex = 0; typeIndex < (u32)field.Schema.FieldTypes.Size(); ++typeIndex)
	{
		SPtr<ISerialized> serializedTupleElement = SerializeTupleElement(object, rttiInstance, field, iterator, typeIndex, flags);

		if(serializedTuple != nullptr)
			serializedTuple->Values.Add(serializedTupleElement);
		else
			serializedEntry = serializedTupleElement;
	}

	if(serializedTuple)
		serializedEntry = serializedTuple;

	return serializedEntry;
}

SPtr<ISerialized> IntermediateSerializer::SerializeTupleElement(IReflectable& object, RTTITypeBase& rttiInstance, RTTIIteratorField& field, IRTTIIterator& iterator, u32 tupleElementIndex, SerializedObjectEncodeFlags flags)
{
	if(!B3D_ENSURE(iterator.IsValid()))
		return nullptr;

	if(!B3D_ENSURE(field.Schema.FieldTypes.Size() >= tupleElementIndex))
		return nullptr;

	const bool shallow = flags.IsSet(SerializedObjectEncodeFlag::Shallow);
	const RTTIFieldTypeSchema& typeSchema = field.Schema.FieldTypes[tupleElementIndex];

	const void* fieldValue = field.GetIteratorValue(&rttiInstance, &object, *mAllocator, iterator);

	switch(typeSchema.Type)
	{
	case SerializableFT_ReflectablePtr:
		{
			if(!shallow)
			{
				const SPtr<IReflectable> referencedObject = field.GetReflectablePointer(fieldValue, tupleElementIndex);

				if(referencedObject)
					return GetOrSerializeReflectableObject(*referencedObject, flags);
			}

			return nullptr;
		}
	case SerializableFT_Reflectable:
		{
			const IReflectable& inlineObject = field.GetReflectable(fieldValue, tupleElementIndex);
			return SerializeReflectableObject(inlineObject, flags);
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

