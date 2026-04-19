//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DIntermediateSerializer.h"
#include "FileSystem/B3DDataStream.h"
#include "Reflection/B3DRTTIType.h"

using namespace b3d;

IntermediateSerializer::IntermediateSerializer(FrameAllocator* allocator, RTTIOperationContext& context)
	: mAllocator(allocator), mContext(context)
{}

SPtr<IReflectable> IntermediateSerializer::Decode(const SerializedObject* serializedObject)
{
	mAllocator->MarkFrame();
	mDeserializedObjectMap.clear();

	SPtr<IReflectable> output;
	RTTIType* type = IReflectable::GetRTTITypeFromTypeId(serializedObject->GetRootTypeId());
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
	const u32 subObjectCount = (u32)serializableObject->SubObjects.size();
	if(subObjectCount == 0)
		return;

	FrameStack<RTTIType*> rttiInstances;
	for(i32 subObjectIndex = (i32)subObjectCount - 1; subObjectIndex >= 0; subObjectIndex--)
	{
		const SerializedSubObject& subObject = serializableObject->SubObjects[subObjectIndex];

		RTTIType* rtti = IReflectable::GetRTTITypeFromTypeId(subObject.TypeId);
		if(rtti == nullptr)
			continue;

		RTTIType* rttiInstance = rtti->CloneInternal(*mAllocator);
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
			switch(curGenericField->Schema.FieldType)
			{
			case RTTIFieldType::Iterable:
			{
				RTTIIteratorField* iteratorField = static_cast<RTTIIteratorField*>(curGenericField);
				SPtr<IRTTIIterator> iterator = iteratorField->GetIterator(rttiInstance, object.get(), *mAllocator);

				if(iterator != nullptr)
					iterator->Clear();

				const bool encodedAsArray = iteratorField->Schema.IsContainer && iteratorField->IteratorSupportsSeekToIndex();
				const bool encodedAsMap = iteratorField->Schema.IsContainer && iteratorField->IteratorSupportsSeekToKey();

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
			break;
			case RTTIFieldType::DataBlock:
			{
				auto* curField = static_cast<RTTIDataBlockFieldBase*>(curGenericField);

				SPtr<SerializedDataBlock> serializedDataBlock = std::static_pointer_cast<SerializedDataBlock>(serializedFieldValue);
				if(serializedDataBlock != nullptr)
				{
					serializedDataBlock->Stream->Seek(serializedDataBlock->Offset);
					curField->SetValue(rttiInstance, object.get(), serializedDataBlock->Stream, serializedDataBlock->Size);
				}
				
			}
			break;
			default:
				B3D_ENSURE(false);
			break;
			}
		}
	}

	while(!rttiInstances.empty())
	{
		RTTIType* rttiInstance = rttiInstances.top();
		rttiInstance->NotifyOperationEnded(*object, RTTIOperationType::Deserialization, mContext);
		mAllocator->Destruct(rttiInstance);

		rttiInstances.pop();
	}
}

void IntermediateSerializer::DeserializeElement(RTTIType& rttiInstance, const SPtr<IReflectable>& object, RTTIIteratorField& field, const SPtr<IRTTIIterator>& iterator, const SPtr<ISerialized>& entry)
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

	const bool isTuple = field.Schema.FieldDataTypes.Size() > 1;
	if(isTuple)
		serializedTuple = std::static_pointer_cast<SerializedTuple>(entry);

	for(u32 fieldTypeIndex = 0; fieldTypeIndex < (u32)field.Schema.FieldDataTypes.Size(); ++fieldTypeIndex)
	{
		RTTIFieldDataTypeSchema& tupleSchema = field.Schema.FieldDataTypes[fieldTypeIndex];

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
	if(!B3D_ENSURE(tupleElementIndex < field.Schema.FieldDataTypes.Size()))
		return;

	RTTIFieldDataTypeSchema& tupleElementSchema = field.Schema.FieldDataTypes[tupleElementIndex];
	switch(tupleElementSchema.Type)
	{
	case RTTIFieldDataType::ReflectablePointer:
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
	case RTTIFieldDataType::Reflectable:
		{
			SPtr<SerializedObject> referencedSerializedObject = std::static_pointer_cast<SerializedObject>(entry);
			RTTIType* childRtti = nullptr;

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
	case RTTIFieldDataType::Plain:
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
	FrameStack<RTTIType*> rttiInstances;
	RTTIType* rtti = object.GetRtti();

	const auto cleanup = [&]()
	{
		while(!rttiInstances.empty())
		{
			RTTIType* rttiInstance = rttiInstances.top();
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
		RTTIType* rttiInstance = rtti->CloneInternal(*mAllocator);
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
			switch(field->Schema.FieldType)
			{
			case RTTIFieldType::Iterable:
				serializedEntry = SerializeIterableField(const_cast<IReflectable&>(object), *rttiInstance, static_cast<RTTIIteratorField&>(*field), flags);
				break;
			case RTTIFieldType::DataBlock:
				serializedEntry = SerializeDataBlockField(const_cast<IReflectable*>(&object), rttiInstance, field, flags);
				break;
			default:
				B3D_ENSURE(false);
				continue;
			}

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

SPtr<ISerialized> IntermediateSerializer::SerializeDataBlockField(IReflectable* object, RTTIType* rtti, RTTIField* field, SerializedObjectEncodeFlags flags)
{
	if(!B3D_ENSURE(field->Schema.FieldType == RTTIFieldType::DataBlock))
		return nullptr;

	auto curField = static_cast<RTTIDataBlockFieldBase*>(field);

	u32 dataBlockSize = 0;
	SPtr<DataStream> blockStream = curField->GetValue(rtti, object, dataBlockSize);

	SPtr<MemoryDataStream> stream = B3DMakeShared<MemoryDataStream>(dataBlockSize);
	blockStream->Read(stream->Data(), dataBlockSize);

	SPtr<SerializedDataBlock> serializedDataBlock = B3DMakeShared<SerializedDataBlock>();
	serializedDataBlock->Stream = stream;
	serializedDataBlock->Offset = 0;

	serializedDataBlock->Size = dataBlockSize;

	return serializedDataBlock;
}

SPtr<ISerialized> IntermediateSerializer::SerializeIterableField(IReflectable& object, RTTIType& rttiInstance, RTTIIteratorField& field, SerializedObjectEncodeFlags flags)
{
	if(!B3D_ENSURE(field.Schema.FieldType == RTTIFieldType::Iterable))
		return nullptr;

	SPtr<ISerialized> output;
	const SPtr<IRTTIIterator> iterator = field.GetIterator(&rttiInstance, &object, *mAllocator);

	if(iterator == nullptr)
		return nullptr;

	const bool encodeAsArray = field.Schema.IsContainer && field.IteratorSupportsSeekToIndex();
	const bool encodeAsMap = field.Schema.IsContainer && field.IteratorSupportsSeekToKey();

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

	return output;
}

SPtr<ISerialized> IntermediateSerializer::SerializeElement(IReflectable& object, RTTIType& rttiInstance, RTTIIteratorField& field, IRTTIIterator& iterator, SerializedObjectEncodeFlags flags)
{
	if(!iterator.IsValid())
		return nullptr;

	SPtr<ISerialized> serializedEntry;
	SPtr<SerializedTuple> serializedTuple;
	if(field.Schema.FieldDataTypes.Size() > 1)
	{
		serializedTuple = B3DMakeShared<SerializedTuple>();
		serializedEntry = serializedTuple;
	}

	for(u32 typeIndex = 0; typeIndex < (u32)field.Schema.FieldDataTypes.Size(); ++typeIndex)
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

SPtr<ISerialized> IntermediateSerializer::SerializeTupleElement(IReflectable& object, RTTIType& rttiInstance, RTTIIteratorField& field, IRTTIIterator& iterator, u32 tupleElementIndex, SerializedObjectEncodeFlags flags)
{
	if(!B3D_ENSURE(iterator.IsValid()))
		return nullptr;

	if(!B3D_ENSURE(field.Schema.FieldDataTypes.Size() >= tupleElementIndex))
		return nullptr;

	const bool shallow = flags.IsSet(SerializedObjectEncodeFlag::Shallow);
	const RTTIFieldDataTypeSchema& typeSchema = field.Schema.FieldDataTypes[tupleElementIndex];

	const void* fieldValue = field.GetIteratorValue(&rttiInstance, &object, *mAllocator, iterator);

	switch(typeSchema.Type)
	{
	case RTTIFieldDataType::ReflectablePointer:
		{
			if(!shallow)
			{
				const SPtr<IReflectable> referencedObject = field.GetReflectablePointer(fieldValue, tupleElementIndex);

				if(referencedObject)
					return GetOrSerializeReflectableObject(*referencedObject, flags);
			}

			return nullptr;
		}
	case RTTIFieldDataType::Reflectable:
		{
			const IReflectable& inlineObject = field.GetReflectable(fieldValue, tupleElementIndex);
			return SerializeReflectableObject(inlineObject, flags);
		}
	case RTTIFieldDataType::Plain:
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
		B3D_LOG(Error, LogSerialization, "Error serializing data. Encountered a type I don't know how to encode. Type: {0}, Is array: {1}", (u32)typeSchema.Type, field.Schema.IsContainer);
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
		RTTIType* objectRttiType = nullptr;

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
			B3D_LOG(Warning, LogGeneric, "Detected a circular reference when decoding. "
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

