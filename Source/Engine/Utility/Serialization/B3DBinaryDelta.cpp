//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DBinaryDelta.h"
#include "Serialization/B3DSerializedObject.h"
#include "Serialization/B3DBinarySerializer.h"
#include "Serialization/B3DBinaryCloner.h"
#include "Serialization/B3DIntermediateSerializer.h"
#include "Reflection/B3DRTTIType.h"
#include "Reflection/B3DRTTIObjectWrapper.h"
#include "FileSystem/B3DDataStream.h"
#include "RTTI/B3DBinaryDeltaRTTI.h"
#include "Utility/B3DUtility.h"

using namespace b3d;

SPtr<ISerialized> SerializedTupleDelta::Clone(bool cloneData)
{
	SPtr<SerializedTupleDelta> copy = B3DMakeShared<SerializedTupleDelta>();

	copy->Key = Key != nullptr ? Key->Clone(cloneData) : nullptr;

	for(auto entry : Values)
	{
		if(entry.Value != nullptr)
			entry.Value = entry.Value->Clone(cloneData);

		copy->Values.Add(entry);
	}

	return copy;
}

u64 SerializedTupleDelta::CalculateHash() const
{
	u64 hash = 0;

	if(Key != nullptr)
		B3DCombineHash(hash, Key->CalculateHash());

	for(auto& entry : Values)
	{
		B3DCombineHash(hash, entry.Index);

		if(entry.Value != nullptr)
			B3DCombineHash(hash, entry.Value->CalculateHash());
	}

	return hash;
}

bool SerializedTupleDelta::Equals(const SPtr<ISerialized>& other) const
{
	if(SPtr<SerializedTupleDelta> otherTuple = B3DRTTICast<SerializedTupleDelta>(other))
	{
		if(!::Equals(Key, otherTuple->Key))
			return false;

		if(Values.Size() != otherTuple->Values.Size())
			return false;

		for(auto it = Values.begin(); it != Values.end(); ++it)
		{
			auto found = std::find_if(otherTuple->Values.begin(), otherTuple->Values.end(), [index = it->Index](const SerializedTupleEntryDelta& entry)
			{
				return entry.Index == index;
			});

			if(found == otherTuple->Values.end())
				return false;

			if(!::Equals(it->Value, found->Value))
				return false;
		}

		return true;
	}

	return false;
}

RTTIType* SerializedTupleDelta::GetRttiStatic()
{
	return SerializedTupleDeltaRTTI::Instance();
}

RTTIType* SerializedTupleDelta::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* SerializedTupleEntryDelta::GetRttiStatic()
{
	return SerializedTupleEntryDeltaRTTI::Instance();
}

RTTIType* SerializedTupleEntryDelta::GetRtti() const
{
	return GetRttiStatic();
}

SPtr<ISerialized> SerializedArrayDelta::Clone(bool cloneData)
{
	SPtr<SerializedArrayDelta> copy = B3DMakeShared<SerializedArrayDelta>();
	copy->ElementCount = ElementCount;

	for(auto& entryPair : Entries)
	{
		SerializedArrayEntryDelta arrayEntry = entryPair.second;

		if(arrayEntry.Value != nullptr)
			arrayEntry.Value = arrayEntry.Value->Clone(cloneData);

		copy->Entries[entryPair.first] = arrayEntry;
	}

	return copy;
}

u64 SerializedArrayDelta::CalculateHash() const
{
	u64 hash = B3DHash(ElementCount);

	for(auto& entryPair : Entries)
	{
		SerializedArrayEntryDelta arrayEntry = entryPair.second;

		if(arrayEntry.Value != nullptr)
			B3DCombineHash(hash, arrayEntry.Value->CalculateHash());
	}

	return hash;
}

bool SerializedArrayDelta::Equals(const SPtr<ISerialized>& other) const
{
	if(SPtr<SerializedArrayDelta> otherArray = B3DRTTICast<SerializedArrayDelta>(other))
	{
		if(ElementCount != otherArray->ElementCount)
			return false;

		if(Entries.size() != otherArray->Entries.size())
			return false;

		for(auto it = Entries.begin(); it != Entries.end(); ++it)
		{
			auto found = otherArray->Entries.find(it->first);
			if(found == otherArray->Entries.end())
				return false;

			if(!::Equals(it->second.Value, found->second.Value))
				return false;
		}

		return true;
	}

	return false;
}

RTTIType* SerializedArrayDelta::GetRttiStatic()
{
	return SerializedArrayDeltaRTTI::Instance();
}

RTTIType* SerializedArrayDelta::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* SerializedArrayEntryDelta::GetRttiStatic()
{
	return SerializedArrayEntryDeltaRTTI::Instance();
}

RTTIType* SerializedArrayEntryDelta::GetRtti() const
{
	return GetRttiStatic();
}

SPtr<ISerialized> SerializedMapDelta::Clone(bool cloneData)
{
	SPtr<SerializedMapDelta> copy = B3DMakeShared<SerializedMapDelta>();

	for(auto& entryPair : Entries)
	{
		std::pair<SPtr<ISerialized>, SerializedMapEntryDelta> copiedEntry;
		if(entryPair.first != nullptr)
			copiedEntry.first = entryPair.first->Clone(cloneData);

		copiedEntry.second.IsRemoved = entryPair.second.IsRemoved;
		if(entryPair.second.Value != nullptr)
			copiedEntry.second.Value = entryPair.second.Value->Clone(cloneData);

		copy->Entries.insert(copiedEntry);
	}

	return copy;
}

u64 SerializedMapDelta::CalculateHash() const
{
	u64 hash = B3DHash(Entries.size());

	for(auto& entryPair : Entries)
	{
		if(entryPair.first != nullptr)
			B3DCombineHash(hash, entryPair.first->CalculateHash());

		B3DCombineHash(hash, entryPair.second.IsRemoved);
		if(entryPair.second.Value != nullptr)
			B3DCombineHash(hash, entryPair.second.Value->CalculateHash());
	}

	return hash;
}

bool SerializedMapDelta::Equals(const SPtr<ISerialized>& other) const
{
	if(SPtr<SerializedMapDelta> otherMap = B3DRTTICast<SerializedMapDelta>(other))
	{
		if(Entries.size() != otherMap->Entries.size())
			return false;

		for(auto it = Entries.begin(); it != Entries.end(); ++it)
		{
			auto found = otherMap->Entries.find(it->first);
			if(found == otherMap->Entries.end())
				return false;

			if(it->second.IsRemoved != found->second.IsRemoved)
				return false;

			if(!::Equals(it->second.Value, found->second.Value))
				return false;
		}

		return true;
	}

	return false;
}

RTTIType* SerializedMapDelta::GetRttiStatic()
{
	return SerializedMapDeltaRTTI::Instance();
}

RTTIType* SerializedMapDelta::GetRtti() const
{
	return GetRttiStatic();
}

RTTIType* SerializedMapEntryDelta::GetRttiStatic()
{
	return SerializedMapEntryDeltaRTTI::Instance();
}

RTTIType* SerializedMapEntryDelta::GetRtti() const
{
	return GetRttiStatic();
}

using namespace RTTIObjectWrapper;

typedef UnorderedMap<IReflectable*, SPtr<SerializedObject>> ObjectMap;

template <bool IsLHSIReflectable, bool IsRHSIReflectable>
SPtr<SerializedObject> GenerateObjectDelta(TOptional<Object<IsLHSIReflectable>> maybeLhs, Object<IsRHSIReflectable> rhs, ObjectMap& inOutObjectMap, RTTIOperationContext& context, bool replicableOnly);

/**
 * Compares two values and creates an object that contains differences between them (if any).
 *
 * @param fieldSchema			Schema describing the field whose values are we comparing.
 * @param maybeLhs				Optional value of the field on the left hand side. This is considered the original state of the field we're comparing against.
 *								Can be empty in which case we always generate a delta containing the right hand side value.
 * @param rhs					Value of the field on the right hand side. This is considered the new state of the field, and will be encoded in the delta
 *								if it differs from the left hand side value.
 * @param objectMap				Map that stores deltas for reflectable objects. As those objects may be references in multiple locations this map ensures we only
 *								generate one delta per object.
 * @param context				Context to pass along to the RTTI system.
 * @param replicableOnly		If true, we will generate a delta only for fields with the replicable flag.
 * @return						Object containing changes present in RHS compared to LHS. Will be empty if RHS and LHS are identical.
 */
template <bool IsLHSIReflectable, bool IsRHSIReflectable>
TOptional<SPtr<ISerialized>> GenerateValueDelta(const RTTIFieldSchema& fieldSchema, const TOptional<Value<IsLHSIReflectable>>& maybeLhs, const Value<IsRHSIReflectable> rhs, ObjectMap& objectMap, RTTIOperationContext& context, bool replicableOnly)
{
	SerializedObjectEncodeFlags flags = replicableOnly ? SerializedObjectEncodeFlag::ReplicableOnly : SerializedObjectEncodeFlags();

	SPtr<SerializedTupleDelta> serializedTupleDelta;
	TOptional<SPtr<ISerialized>> modification;

	const bool isTuple = fieldSchema.FieldDataTypes.Size() > 1;
	const bool isLhsMissing = !maybeLhs.has_value(); // Will be true if e.g. comparing an array or map entry that doesn't exist in LHS object
	for(u32 tupleElementIndex = 0; tupleElementIndex < (u32)fieldSchema.FieldDataTypes.Size(); ++tupleElementIndex)
	{
		const RTTIFieldDataTypeSchema& fieldTypeSchema = fieldSchema.FieldDataTypes[tupleElementIndex];

		const Value<IsRHSIReflectable>& rhsTupleElement = rhs.GetTupleElement(tupleElementIndex);
		TOptional<Value<IsLHSIReflectable>> maybeLhsTupleElement;
		if(!isLhsMissing)
			maybeLhsTupleElement = maybeLhs->GetTupleElement(tupleElementIndex);

		TOptional<SPtr<ISerialized>> tupleElementModification;

		switch(fieldTypeSchema.Type)
		{
		case RTTIFieldDataType::ReflectablePointer:
		case RTTIFieldDataType::Reflectable:
			{
				TOptional<Object<IsLHSIReflectable>> maybeLhsObject = isLhsMissing ? std::nullopt : std::make_optional(maybeLhsTupleElement->GetObject());
				Object<IsRHSIReflectable> rhsObject = rhsTupleElement.GetObject();

				bool isLHSEntryNull = isLhsMissing;
				bool isRHSEntryNull = false;
				if(fieldTypeSchema.Type == RTTIFieldDataType::ReflectablePointer)
				{
					isLHSEntryNull = isLHSEntryNull || maybeLhsObject->GetWrappedObject() == nullptr;
					isRHSEntryNull = rhsObject.GetWrappedObject() == nullptr;
				}

				if(!isLHSEntryNull)
				{
					if(!isRHSEntryNull)
					{
						auto found = objectMap.find(rhsObject.GetWrappedObject());
						if(found != objectMap.end())
							tupleElementModification = found->second;
						else
						{
							RTTIType* rttiType = nullptr;
							if(maybeLhsObject->GetTypeId() == rhsObject.GetTypeId())
								rttiType = IReflectable::GetRTTITypeFromTypeId(rhsObject.GetTypeId());

							SPtr<SerializedObject> objectDelta;
							if(rttiType != nullptr)
							{
								IDeltaHandler& handler = rttiType->GetDeltaHandler();
								objectDelta = handler.GenerateDeltaRecursive(maybeLhsObject->GetWrappedObject(), rhsObject.GetWrappedObject(), objectMap, context, replicableOnly);
							}

							if(objectDelta != nullptr)
							{
								objectMap[rhsObject.GetWrappedObject()] = objectDelta;
								tupleElementModification = objectDelta;
							}
						}
					}
					else
						tupleElementModification = nullptr;
				}
				else
				{
					if(!isRHSEntryNull)
						tupleElementModification = GenerateObjectDelta(TOptional<Object<IsLHSIReflectable>>(), rhsObject, objectMap, context, replicableOnly);
				}
			}
			break;
		case RTTIFieldDataType::Plain:
			{
				if(isLhsMissing || maybeLhsTupleElement->ComparePlain(rhsTupleElement))
					tupleElementModification = rhsTupleElement.Clone(flags, context);
			}
			break;
		case RTTIFieldDataType::DataBlock:
			{
				bool isDataBlockModified = isLhsMissing;
				if(!isLhsMissing)
				{
					u32 lhsFieldDataSize;
					u32 lhsFieldDataOffset;
					SPtr<DataStream> lhsFieldStream = maybeLhs->GetDataStream(lhsFieldDataSize, lhsFieldDataOffset);

					u32 rhsFieldDataSize;
					u32 rhsFieldDataOffset;
					SPtr<DataStream> rhsFieldStream = rhs.GetDataStream(rhsFieldDataSize, rhsFieldDataOffset);

					isDataBlockModified = lhsFieldDataSize != rhsFieldDataSize;
					if(!isDataBlockModified)
					{
						u8* lhsStreamData = nullptr;
						if(lhsFieldStream->IsFile())
						{
							lhsStreamData = (u8*)B3DStackAllocate(lhsFieldDataSize);
							lhsFieldStream->Seek(lhsFieldDataOffset);
							lhsFieldStream->Read(lhsStreamData, lhsFieldDataSize);
						}
						else
						{
							SPtr<MemoryDataStream> lhsMemoryStream = std::static_pointer_cast<MemoryDataStream>(lhsFieldStream);
							lhsStreamData = lhsMemoryStream->Cursor();
						}

						u8* rhsStreamData = nullptr;
						if(rhsFieldStream->IsFile())
						{
							rhsStreamData = (u8*)B3DStackAllocate(rhsFieldDataSize);
							rhsFieldStream->Seek(rhsFieldDataOffset);
							rhsFieldStream->Read(rhsStreamData, rhsFieldDataSize);
						}
						else
						{
							SPtr<MemoryDataStream> rhsMemoryStream = std::static_pointer_cast<MemoryDataStream>(rhsFieldStream);
							rhsStreamData = rhsMemoryStream->Cursor();
						}

						isDataBlockModified = memcmp(lhsStreamData, rhsStreamData, rhsFieldDataSize) != 0;

						if(rhsFieldStream->IsFile())
							B3DStackFree(rhsStreamData);

						if(lhsFieldStream->IsFile())
							B3DStackFree(lhsStreamData);
					}
				}

				if(isDataBlockModified)
					tupleElementModification = rhs.Clone(flags, context);
			}
			break;
		}

		if(tupleElementModification.has_value())
		{
			if(isTuple)
			{
				if(serializedTupleDelta == nullptr)
				{
					serializedTupleDelta = B3DMakeShared<SerializedTupleDelta>();

					const Value<IsRHSIReflectable>& rhsTupleKeyElement = rhs.GetTupleElement(0);
					serializedTupleDelta->Key = rhsTupleKeyElement.Clone(flags, context);

					modification = serializedTupleDelta;
				}

				SerializedTupleEntryDelta tupleDeltaEntry;
				tupleDeltaEntry.Index = tupleElementIndex;
				tupleDeltaEntry.Value = *tupleElementModification;

				serializedTupleDelta->Values.Add(std::move(tupleDeltaEntry));
			}
			else
				modification = tupleElementModification;
		}
	}

	return modification;
}

template <bool IsLHSIReflectable, bool IsRHSIReflectable>
SPtr<SerializedObject> GenerateObjectDelta(TOptional<Object<IsLHSIReflectable>> maybeLhs, Object<IsRHSIReflectable> rhs, ObjectMap& inOutObjectMap, RTTIOperationContext& context, bool replicableOnly)
{
	SubObjectIterator<IsRHSIReflectable> rhsSubObjectIterator = rhs.GetSubObjectIterator();

	SPtr<SerializedObject> output;
	while(rhsSubObjectIterator.MoveNext())
	{
		SubObject<IsRHSIReflectable> rhsSubObject = rhsSubObjectIterator.GetValue();

		RTTIType* rtti = IReflectable::GetRTTITypeFromTypeId(rhsSubObject.GetTypeId());
		if(rtti == nullptr)
			continue;

		TOptional<SubObject<IsLHSIReflectable>> maybeLhsSubObject;

		if(maybeLhs.has_value())
		{
			SubObjectIterator<IsLHSIReflectable> lhsSubObjectIterator = maybeLhs->GetSubObjectIterator();
			while(lhsSubObjectIterator.MoveNext())
			{
				SubObject<IsLHSIReflectable> lhsSubObjectCandidate = lhsSubObjectIterator.GetValue();
				if(lhsSubObjectCandidate.GetTypeId() == rhsSubObject.GetTypeId())
				{
					maybeLhsSubObject = lhsSubObjectCandidate;
					break;
				}
			}
		}

		rhs.NotifyBeginOperation(rhsSubObject, RTTIOperationType::DeltaGenerate, context);

		if(maybeLhsSubObject.has_value())
			maybeLhs->NotifyBeginOperation(*maybeLhsSubObject, RTTIOperationType::DeltaGenerate, context);

		FieldIterator<IsRHSIReflectable> rhsFieldIterator = rhsSubObject.GetFieldIterator();

		SerializedSubObject* subObjectDelta = nullptr;
		while(rhsFieldIterator.MoveNext())
		{
			Field<IsRHSIReflectable> rhsField = rhsFieldIterator.GetValue();

			RTTIField* const field = rtti->FindField(rhsField.GetId());
			if(field == nullptr)
				continue;

			if(replicableOnly && !field->Schema.Info.Flags.IsSet(RTTIFieldFlag::Replicate))
				continue;

			if(field->Schema.Info.Flags.IsSet(RTTIFieldFlag::SkipInDeltaCompare))
				continue;

			TOptional<Field<IsLHSIReflectable>> maybeLhsField;
			if(maybeLhsSubObject.has_value())
			{
				FieldIterator<IsLHSIReflectable> lhsFieldIterator = maybeLhsSubObject->GetFieldIterator();
				while(lhsFieldIterator.MoveNext())
				{
					Field<IsLHSIReflectable> lhsFieldCandidate = lhsFieldIterator.GetValue();
					if(lhsFieldCandidate.GetId() == rhsField.GetId())
					{
						maybeLhsField = lhsFieldCandidate;
						break;
					}
				}
			}

			SPtr<SerializedArrayDelta> serializedArrayDelta;
			SPtr<SerializedMapDelta> serializedMapDelta;
			SPtr<ISerialized> modification;
			bool hasModification = false;

			const bool isMap = field->Schema.FieldType == RTTIFieldType::Iterable && field->Schema.IsContainer && static_cast<RTTIIteratorField*>(field)->IteratorSupportsSeekToKey();
			const bool isArray = field->Schema.IsContainer;

			ValueIterator<IsRHSIReflectable> rhsValueIterator = rhsField.GetValueIterator();
			for(u32 elementIndex = 0; rhsValueIterator.MoveNext(); ++elementIndex)
			{
				Value<IsRHSIReflectable> rhsValue = rhsValueIterator.GetValue();
				TOptional<Value<IsLHSIReflectable>> maybeLHSValue;

				if(maybeLhsField.has_value())
				{
					ValueIterator<IsLHSIReflectable> lhsValueIterator = maybeLhsField->GetValueIterator();
					maybeLHSValue = lhsValueIterator.FindMatchingValue(rhsValueIterator);
				}

				TOptional<SPtr<ISerialized>> valueModification = GenerateValueDelta(field->Schema, maybeLHSValue, rhsValue, inOutObjectMap, context, replicableOnly);

				// If container, the modification above is just a single entry
				if(valueModification.has_value())
				{
					if(isMap)
					{
						if(serializedMapDelta == nullptr)
							serializedMapDelta = B3DMakeShared<SerializedMapDelta>();

						SPtr<ISerialized> entryKey;
						if(const auto& tuple = B3DRTTICast<SerializedTupleDelta>(*valueModification))
							entryKey = tuple->Key;
						else
							entryKey = *valueModification;

						SerializedMapEntryDelta mapEntry;
						mapEntry.IsRemoved = false;
						mapEntry.Value = *valueModification;

						serializedMapDelta->Entries[entryKey] = std::move(mapEntry);
						modification = serializedMapDelta;
						
					}
					else if(isArray)
					{
						if(serializedArrayDelta == nullptr)
						{
							serializedArrayDelta = B3DMakeShared<SerializedArrayDelta>();
							serializedArrayDelta->ElementCount = rhsValueIterator.GetElementCount();
						}

						SerializedArrayEntryDelta arrayEntry;
						arrayEntry.Index = elementIndex;
						arrayEntry.Value = *valueModification;

						serializedArrayDelta->Entries[elementIndex] = std::move(arrayEntry);
						modification = serializedArrayDelta;
					}
					else
					{
						modification = *valueModification;
					}

					hasModification = true;
				}
			} 

			// For maps we also need to iterate over LHS container, to find removed entries
			if(isMap && maybeLhsField.has_value())
			{
				ValueIterator<IsLHSIReflectable> lhsValueIterator = maybeLhsField->GetValueIterator();
				for(u32 elementIndex = 0; lhsValueIterator.MoveNext(); ++elementIndex)
				{
					Value<IsLHSIReflectable> lhsValue = lhsValueIterator.GetValue();

					if(!rhsValueIterator.FindMatchingValue(lhsValueIterator).has_value())
					{
						if(serializedMapDelta == nullptr)
							serializedMapDelta = B3DMakeShared<SerializedMapDelta>();

						// We use the delta generation function to generate the serialized key as a convenience
						TOptional<SPtr<ISerialized>> serializedLhsValue = GenerateValueDelta(field->Schema, TOptional<Value<IsLHSIReflectable>>(), lhsValue, inOutObjectMap, context, false);
						SPtr<ISerialized> entryKey;
						if(const auto& tuple = B3DRTTICast<SerializedTupleDelta>(*serializedLhsValue))
							entryKey = tuple->Key;
						else
							entryKey = *serializedLhsValue;

						SerializedMapEntryDelta mapEntry;
						mapEntry.IsRemoved = true;

						serializedMapDelta->Entries[entryKey] = std::move(mapEntry);
						modification = serializedMapDelta;

						hasModification = true;
					}
				} 
			}

			if(hasModification)
			{
				if(output == nullptr)
					output = B3DMakeShared<SerializedObject>();

				if(subObjectDelta == nullptr)
				{
					output->SubObjects.push_back(SerializedSubObject());
					subObjectDelta = &output->SubObjects.back();
					subObjectDelta->TypeId = rtti->GetRttiId();
				}

				SerializedField modificationEntry;
				modificationEntry.FieldId = field->Schema.Id;
				modificationEntry.Value = modification;
				subObjectDelta->FieldEntries[field->Schema.Id] = modificationEntry;
			}
		}

		if(maybeLhs.has_value())
			maybeLhs->NotifyEndOperation(RTTIOperationType::DeltaGenerate, context);

		rhs.NotifyEndOperation(RTTIOperationType::DeltaGenerate, context);
	}

	return output;
}

SPtr<SerializedObject> IDeltaHandler::GenerateDelta(const SPtr<IReflectable>& original, const SPtr<IReflectable>& modified, RTTIOperationContext& context, bool replicableOnly)
{
	ObjectMap objectMap;
	return GenerateDeltaRecursive(original.get(), modified.get(), objectMap, context, replicableOnly);
}

void IDeltaHandler::ApplyDelta(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& delta, RTTIOperationContext& context)
{
	FrameAllocator& allocator = GetFrameAllocator();
	FrameAllocatorScope frameAllocatorScope(&allocator);

	FrameVector<DeltaCommand> commands;

	DeltaObjectMap objectMap;
	GenerateDeltaApplyCommands(object, delta, allocator, objectMap, commands, context);

	IReflectable* destinationObject = nullptr;
	RTTIType* rttiInstance = nullptr;

	struct ObjectAndRTTIInstance
	{
		ObjectAndRTTIInstance(IReflectable* object = nullptr, RTTIType* rttiInstance = nullptr)
			: Object(object), RttiInstance(rttiInstance)
		{ }

		IReflectable* Object;
		RTTIType* RttiInstance;
	};

	FrameStack<ObjectAndRTTIInstance> objectStack;
	FrameVector<std::pair<RTTIType*, IReflectable*>> rttiInstances;

	struct IteratorAndValue
	{
		IteratorAndValue(const SPtr<IRTTIIterator>& iterator, void* fieldValue)
			: Iterator(iterator), FieldValue(fieldValue)
		{ }

		SPtr<IRTTIIterator> Iterator;
		void* FieldValue = nullptr;
	};

	FrameStack<IteratorAndValue> iteratorStack;
	SPtr<IRTTIIterator> currentIterator;
	void* currentIteratorFieldValue = nullptr;

	for(auto& command : commands)
	{
		const bool isArray = (command.Type & Diff_ArrayFlag) != 0;
		const bool isMap = (command.Type & Diff_MapFlag) != 0;
		DeltaCommandType type = (DeltaCommandType)(command.Type & 0xF);

		switch(type)
		{
		case Diff_ArraySize:
			{
				if(B3D_ENSURE(command.Field->Schema.FieldType == RTTIFieldType::Iterable))
				{
					auto& field = *static_cast<RTTIIteratorField*>(command.Field);
					SPtr<IRTTIIterator> iterator = field.GetIterator(rttiInstance, destinationObject, allocator);
					if(B3D_ENSURE(iterator != nullptr))
					{
						// Add or remove excess elements
						if(command.ArraySize > iterator->GetElementCount())
						{
							while(command.ArraySize > iterator->GetElementCount())
							{
								iterator->SeekToEnd(); // Ensures value is inserted at the end of the iterable container

								void* fieldValue = field.CreateEmptyFieldValue(allocator);
								field.SetIteratorValue(rttiInstance, destinationObject, allocator, *iterator, fieldValue);
								field.FreeFieldValue(fieldValue, allocator);
							}
						}
						else if(command.ArraySize < iterator->GetElementCount())
						{
							while(command.ArraySize < iterator->GetElementCount())
							{
								if(!B3D_ENSURE(field.IteratorSupportsSeekToIndex())) // ArraySize command shouldn't have been generated if this is not the case
									continue;

								// Erase last element
								iterator->SeekToIndex(iterator->GetElementCount() - 1);
								iterator->Erase();
							}
						}
					}
				}
			}
			break;
		case Diff_ObjectStart:
			{
				destinationObject = command.Object.get();
				objectStack.push(ObjectAndRTTIInstance(destinationObject, nullptr));

				FrameStack<RTTIType*> rttiTypes;
				RTTIType* curRtti = destinationObject->GetRtti();
				while(curRtti != nullptr)
				{
					rttiTypes.push(curRtti);
					curRtti = curRtti->GetBaseClass();
				}

				// Call base class first, followed by derived classes
				while(!rttiTypes.empty())
				{
					RTTIType* curRtti = rttiTypes.top();
					RTTIType* rttiInstance = curRtti->CloneInternal(allocator);

					rttiInstances.push_back(std::make_pair(rttiInstance, destinationObject));
					rttiInstance->NotifyOperationStarted(*destinationObject, RTTIOperationType::DeltaApply, context);

					rttiTypes.pop();
				}
			}
			break;
		case Diff_SubObjectStart:
			{
				// Find the instance
				rttiInstance = nullptr;
				for(auto iter = rttiInstances.rbegin(); iter != rttiInstances.rend(); ++iter)
				{
					if(iter->second != destinationObject)
						break;

					if(iter->first->GetRttiId() == command.RttiType->GetRttiId())
						rttiInstance = iter->first;
				}

				B3D_ASSERT(rttiInstance != nullptr);

				if(B3D_ENSURE(!objectStack.empty()))
					objectStack.top().RttiInstance = rttiInstance;
			}
			break;
		case Diff_ObjectEnd:
			{
				while(!rttiInstances.empty())
				{
					if(rttiInstances.back().second != destinationObject)
						break;

					RTTIType* rttiInstance = rttiInstances.back().first;

					rttiInstance->NotifyOperationEnded(*destinationObject, RTTIOperationType::DeltaApply, context);
					allocator.Destruct(rttiInstance);

					rttiInstances.erase(rttiInstances.end() - 1);
				}

				objectStack.pop();

				if(!objectStack.empty())
				{
					ObjectAndRTTIInstance objectAndRTTIInstance = objectStack.top();
					destinationObject = objectAndRTTIInstance.Object;
					rttiInstance = objectAndRTTIInstance.RttiInstance;
				}
				else
				{
					destinationObject = nullptr;
					rttiInstance = nullptr;
				}
			}
			break;
		case Diff_IterableEntryStart:
			{
				B3D_ASSERT(command.Field != nullptr);
				B3D_ASSERT(command.Field->Schema.FieldType == RTTIFieldType::Iterable);
				B3D_ASSERT(rttiInstance != nullptr);
				B3D_ASSERT(destinationObject != nullptr);

				if(currentIterator != nullptr)
					iteratorStack.push(IteratorAndValue(currentIterator, currentIteratorFieldValue));

				auto& field = *static_cast<RTTIIteratorField*>(command.Field);
				currentIterator = field.GetIterator(rttiInstance, destinationObject, allocator);
				currentIteratorFieldValue = nullptr;

				if(isMap)
				{
					B3D_ASSERT(command.MapKey != nullptr);

					if(currentIterator->SeekToKey(command.MapKey)) // Modifying existing value
						currentIteratorFieldValue = field.GetIteratorValueCopy(rttiInstance, destinationObject, allocator, *currentIterator);
					else // Inserting a new value
						currentIteratorFieldValue = field.CreateEmptyFieldValue(allocator);

					field.FreeFieldValue(command.MapKey, allocator);
					command.MapKey = nullptr;
				}
				else if(isArray)
				{
					B3D_ASSERT(command.ArrayIndex != ~0u);

					// This should have been guaranteed by the ArraySize command
					B3D_ASSERT(command.ArrayIndex < currentIterator->GetElementCount());

					if(currentIterator->SeekToIndex(command.ArrayIndex))
						currentIteratorFieldValue = field.GetIteratorValueCopy(rttiInstance, destinationObject, allocator, *currentIterator);
					else // Inserting a new value
					{
						currentIterator->SeekToEnd();
						currentIteratorFieldValue = field.CreateEmptyFieldValue(allocator);
					}
				}
				else
				{
					currentIteratorFieldValue = field.GetIteratorValueCopy(rttiInstance, destinationObject, allocator, *currentIterator);
				}
			}
			break;
		case Diff_IterableEntryEnd:
			{
				B3D_ASSERT(command.Field != nullptr);
				B3D_ASSERT(command.Field->Schema.FieldType == RTTIFieldType::Iterable);
				B3D_ASSERT(currentIterator != nullptr);
				B3D_ASSERT(currentIteratorFieldValue != nullptr);
				B3D_ASSERT(rttiInstance != nullptr);
				B3D_ASSERT(destinationObject != nullptr);

				auto& field = *static_cast<RTTIIteratorField*>(command.Field);
				field.SetIteratorValue(rttiInstance, destinationObject, allocator, *currentIterator, currentIteratorFieldValue);
				field.FreeFieldValue(currentIteratorFieldValue, allocator);

				currentIterator = nullptr;
				currentIteratorFieldValue = nullptr;

				if(!iteratorStack.empty())
				{
					currentIterator = iteratorStack.top().Iterator;
					currentIteratorFieldValue = iteratorStack.top().FieldValue;

					iteratorStack.pop();
				}
			}
			break;
		case Diff_RemoveMapEntry:
			{
				B3D_ASSERT(command.Field != nullptr);
				B3D_ASSERT(command.Field->Schema.FieldType == RTTIFieldType::Iterable);
				B3D_ASSERT(command.MapKey != nullptr);
				B3D_ASSERT(rttiInstance != nullptr);
				B3D_ASSERT(destinationObject != nullptr);
				B3D_ASSERT(isMap);

				auto& field = *static_cast<RTTIIteratorField*>(command.Field);
				const SPtr<IRTTIIterator> iterator = field.GetIterator(rttiInstance, destinationObject, allocator);

				if(B3D_ENSURE(iterator->SeekToKey(command.MapKey)))
					iterator->Erase();

				field.FreeFieldValue(command.MapKey, allocator);
				command.MapKey = nullptr;
			}
			break;
		default:
			break;
		}

		if(command.Field != nullptr)
		{
			switch(command.Field->Schema.FieldType)
			{
			case RTTIFieldType::Iterable:
			{
				auto& field = *static_cast<RTTIIteratorField*>(command.Field);

				switch(type)
				{
				case Diff_ReflectablePtr:
					{
						B3D_ASSERT(currentIterator != nullptr);
						B3D_ASSERT(currentIteratorFieldValue != nullptr);

						field.SetReflectablePointer(currentIteratorFieldValue, command.TupleElementIndex, command.Object);
					}
					break;
				case Diff_Reflectable:
					{
						B3D_ASSERT(currentIterator != nullptr);
						B3D_ASSERT(currentIteratorFieldValue != nullptr);
						B3D_ASSERT(command.Object != nullptr);

						field.SetReflectable(currentIteratorFieldValue, command.TupleElementIndex, *command.Object);
					}
					break;
				case Diff_Plain:
					{
						B3D_ASSERT(currentIterator != nullptr);
						B3D_ASSERT(currentIteratorFieldValue != nullptr);

						Bitstream tempStream(command.Value, command.Size);
						field.ReadPlainTypeTupleFromStream(currentIteratorFieldValue, command.TupleElementIndex, tempStream, false);
					}
					break;
				default:
					break;
				}
			}
			break;
			case RTTIFieldType::DataBlock:
			{
				auto* field = static_cast<RTTIDataBlockFieldBase*>(command.Field);
				field->SetValue(rttiInstance, destinationObject, command.StreamValue, command.Size);
			}
			break;
			}
		}
	}
}

void IDeltaHandler::GenerateDeltaApplyCommands(RTTIType* rtti, const SPtr<IReflectable>& object, const SPtr<SerializedObject>& delta, FrameAllocator& allocator, DeltaObjectMap& objectMap, FrameVector<DeltaCommand>& inOutDeltaCommands, RTTIOperationContext& context)
{
	IDeltaHandler& deltaHandler = rtti->GetDeltaHandler();
	deltaHandler.GenerateDeltaApplyCommands(object, delta, allocator, objectMap, inOutDeltaCommands, context);
}

SPtr<SerializedObject> BinaryDeltaHandler::GenerateDeltaRecursive(IReflectable* original, IReflectable* modified, ObjectMap& objectMap, RTTIOperationContext& context, bool replicableOnly)
{
	FrameAllocator& frameAllocator = GetFrameAllocator();

	if(original->GetTypeId() == TID_SerializedObject)
	{
		Object<false> lhsWrapper(static_cast<SerializedObject*>(original), &frameAllocator);

		if(modified->GetTypeId() == TID_SerializedObject)
		{
			Object<false> rhsWrapper(static_cast<SerializedObject*>(modified), &frameAllocator);
			return ::GenerateObjectDelta(std::make_optional(lhsWrapper), rhsWrapper, objectMap, context, replicableOnly);
		}

		Object<true> rhsWrapper(modified, modified->GetRtti(), &frameAllocator);
		return ::GenerateObjectDelta(std::make_optional(lhsWrapper), rhsWrapper, objectMap, context, replicableOnly);
	}
	else
	{
		Object<true> lhsWrapper(original, original->GetRtti(), &frameAllocator);

		if(modified->GetTypeId() == TID_SerializedObject)
		{
			Object<false> rhsWrapper(static_cast<SerializedObject*>(modified), &frameAllocator);
			return ::GenerateObjectDelta(std::make_optional(lhsWrapper), rhsWrapper, objectMap, context, replicableOnly);
		}

		Object<true> rhsWrapper(modified, modified->GetRtti(), &frameAllocator);
		return ::GenerateObjectDelta(std::make_optional(lhsWrapper), rhsWrapper, objectMap, context, replicableOnly);
	}
}

void BinaryDeltaHandler::GenerateDeltaApplyCommands(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& delta, FrameAllocator& allocator, DeltaObjectMap& objectMap, FrameVector<DeltaCommand>& inOutDeltaCommands, RTTIOperationContext& context)
{
	if(object == nullptr || delta == nullptr)
		return;

	// Generate a list of commands per sub-object
	FrameVector<FrameVector<DeltaCommand>> commandsPerSubObject;

	RTTIOperationContext rttiOperationContext;
	Stack<RTTIType*> rttiInstances;
	for(auto& subObject : delta->SubObjects)
	{
		RTTIType* rtti = IReflectable::GetRTTITypeFromTypeId(subObject.TypeId);
		if(rtti == nullptr)
			continue;

		if(!object->IsDerivedFrom(rtti))
			continue;

		RTTIType* rttiInstance = rtti->CloneInternal(allocator);
		rttiInstance->NotifyOperationStarted(*object, RTTIOperationType::DeltaRead, rttiOperationContext);
		rttiInstances.push(rttiInstance);

		FrameVector<DeltaCommand> subObjectCommands;

		DeltaCommand subObjectStartCommand;
		subObjectStartCommand.RttiType = rtti;
		subObjectStartCommand.Field = nullptr;
		subObjectStartCommand.Type = Diff_SubObjectStart;

		subObjectCommands.push_back(subObjectStartCommand);

		for(auto& diffEntry : subObject.FieldEntries)
		{
			RTTIField* genericField = rtti->FindField(diffEntry.first);
			if(genericField == nullptr)
				continue;

			SPtr<ISerialized> fieldDelta = diffEntry.second.Value;

			switch(genericField->Schema.FieldType)
			{
			case RTTIFieldType::Iterable:
			{
				auto* field = static_cast<RTTIIteratorField*>(genericField);

				if(const auto& serializedArrayDelta = B3DRTTICast<SerializedArrayDelta>(fieldDelta))
				{
					const u32 arrayDeltaElementCount = serializedArrayDelta->ElementCount;

					DeltaCommand arraySizeCommand;
					arraySizeCommand.Field = genericField;
					arraySizeCommand.Type = Diff_ArraySize | Diff_ArrayFlag;
					arraySizeCommand.ArraySize = arrayDeltaElementCount;

					subObjectCommands.push_back(arraySizeCommand);

					for(auto& arrayDeltaElement : serializedArrayDelta->Entries)
					{
						GenerateDeltaCommandForFieldEntry(rttiInstance, object, *field, arrayDeltaElement.second.Value, arrayDeltaElement.second.Index, nullptr, objectMap, subObjectCommands, context, allocator);
					}
				}
				else if(const auto& serializedMapDelta = B3DRTTICast<SerializedMapDelta>(fieldDelta))
				{
					for(auto& mapDeltaElement : serializedMapDelta->Entries)
					{
						void* deserializedMapKey = field->CreateEmptyFieldValue(allocator);

						RTTIOperationEngineContext rttiOperationContext;
						IntermediateSerializer intermediateSerializer(&allocator, rttiOperationContext);
						intermediateSerializer.DeserializeTupleElement(*field, deserializedMapKey, 0, mapDeltaElement.first);

						if(!mapDeltaElement.second.IsRemoved)
						{
							GenerateDeltaCommandForFieldEntry(rttiInstance, object, *field, mapDeltaElement.second.Value, ~0u, deserializedMapKey, objectMap, subObjectCommands, context, allocator);
						}
						else
						{
							DeltaCommand removeMapEntryCommand;
							removeMapEntryCommand.Field = genericField;
							removeMapEntryCommand.Type = Diff_RemoveMapEntry | Diff_MapFlag;
							removeMapEntryCommand.MapKey = deserializedMapKey;

							subObjectCommands.push_back(removeMapEntryCommand);
						}
					}
				}
				else
				{
					GenerateDeltaCommandForFieldEntry(rttiInstance, object, *field, fieldDelta, ~0u, nullptr, objectMap, subObjectCommands, context, allocator);
				}
			}
			break;
			case RTTIFieldType::DataBlock:
			{
				GenerateDeltaCommandForDataBlockField(*genericField, fieldDelta, subObjectCommands);
			}
			break;
			}
		}

		commandsPerSubObject.emplace_back(std::move(subObjectCommands));
	}

	DeltaCommand objectStartCommand;
	objectStartCommand.Field = nullptr;
	objectStartCommand.Type = Diff_ObjectStart;
	objectStartCommand.Object = object;

	inOutDeltaCommands.push_back(objectStartCommand);

	// Go in reverse because when deserializing we want to deserialize base first, and then derived types
	for(auto iter = commandsPerSubObject.rbegin(); iter != commandsPerSubObject.rend(); ++iter)
		inOutDeltaCommands.insert(inOutDeltaCommands.end(), iter->begin(), iter->end());

	DeltaCommand objectEndCommand;
	objectEndCommand.Field = nullptr;
	objectEndCommand.Type = Diff_ObjectEnd;
	objectEndCommand.Object = object;

	inOutDeltaCommands.push_back(objectEndCommand);

	while(!rttiInstances.empty())
	{
		RTTIType* rttiInstance = rttiInstances.top();
		rttiInstance->NotifyOperationEnded(*object, RTTIOperationType::DeltaRead, rttiOperationContext);
		allocator.Destruct(rttiInstance);

		rttiInstances.pop();
	}
}

void BinaryDeltaHandler::GenerateDeltaCommandForFieldEntry(RTTIType* rttiInstance, const SPtr<IReflectable>& object, RTTIIteratorField& field, const SPtr<ISerialized>& entryDelta, u32 arrayIndex, void* mapKey, DeltaObjectMap& inOutObjectMap, FrameVector<DeltaCommand>& outCommands, RTTIOperationContext& context, FrameAllocator& allocator)
{
	const SPtr<SerializedTupleDelta> serializedTupleDelta = B3DRTTICast<SerializedTupleDelta>(entryDelta);
	if(!B3D_ENSURE(field.Schema.FieldDataTypes.Size() == 1 || serializedTupleDelta != nullptr))
		return;

	SPtr<IRTTIIterator> iterator = field.GetIterator(rttiInstance, object.get(), allocator);
	if(iterator == nullptr)
		return;

	DeltaCommand iteratorStartCommand;
	iteratorStartCommand.Field = &field;
	iteratorStartCommand.Type = Diff_IterableEntryStart;

	const void* fieldValue = nullptr;
	if(arrayIndex != ~0u)
	{
		iteratorStartCommand.Type |= Diff_ArrayFlag;
		iteratorStartCommand.ArrayIndex = arrayIndex;

		if(!B3D_ENSURE(field.IteratorSupportsSeekToIndex()))
			return;

		if(iterator->SeekToIndex(arrayIndex))
			fieldValue = field.GetIteratorValue(rttiInstance, object.get(), allocator, *iterator);
	}
	else if(mapKey != nullptr)
	{
		iteratorStartCommand.Type |= Diff_MapFlag;
		iteratorStartCommand.MapKey = mapKey;

		if(!B3D_ENSURE(field.IteratorSupportsSeekToKey()))
			return;

		if(iterator->SeekToKey(mapKey))
			fieldValue = field.GetIteratorValue(rttiInstance, object.get(), allocator, *iterator);
	}
	else
	{
		fieldValue = field.GetIteratorValue(rttiInstance, object.get(), allocator, *iterator);
	}

	outCommands.push_back(iteratorStartCommand);

	for(u32 tupleElementIndex = 0; tupleElementIndex < field.Schema.FieldDataTypes.Size(); ++tupleElementIndex)
	{
		SPtr<ISerialized> serializedEntryDelta;
		if(serializedTupleDelta != nullptr)
		{
			auto found = std::find_if(serializedTupleDelta->Values.begin(), serializedTupleDelta->Values.end(),
				[tupleElementIndex](const SerializedTupleEntryDelta& entry)
				{
					return entry.Index == tupleElementIndex;
				});

			if(found == serializedTupleDelta->Values.end())
				continue;

			serializedEntryDelta = found->Value;
		}
		else
			serializedEntryDelta = entryDelta;

		DeltaCommand command;
		command.Field = &field;
		command.TupleElementIndex = tupleElementIndex;
		command.Type = 0;

		// Retrieves the deserialized object from the object map, or if it doesn't exist creates and new instance of the object.
		auto fnGetOrDeserializeObject = [&inOutObjectMap](const SPtr<SerializedObject>& serializedObject) -> SPtr<IReflectable>
		{
			if(serializedObject == nullptr)
				return nullptr;

			RTTIType* childRtti = IReflectable::GetRTTITypeFromTypeId(serializedObject->GetRootTypeId());
			if(childRtti != nullptr)
			{
				auto foundObject = inOutObjectMap.find(serializedObject);
				if(foundObject == inOutObjectMap.end())
				{
					SPtr<IReflectable> newObject = childRtti->NewRttiObject();
					foundObject = inOutObjectMap.insert(std::make_pair(serializedObject, newObject)).first;
				}

				return foundObject->second;
			}

			return nullptr;
		};

		switch(field.Schema.FieldDataTypes[tupleElementIndex].Type)
		{
		case RTTIFieldDataType::ReflectablePointer:
			{
				SPtr<SerializedObject> serializedObjectDelta = std::static_pointer_cast<SerializedObject>(serializedEntryDelta);

				command.Type |= Diff_ReflectablePtr;

				if(serializedObjectDelta == nullptr)
					command.Object = nullptr;
				else
				{
					SPtr<IReflectable> childObject = fieldValue != nullptr ? field.GetReflectablePointer(fieldValue, tupleElementIndex) : nullptr;
					if(childObject == nullptr)
						command.Object = fnGetOrDeserializeObject(serializedObjectDelta);
					else
						command.Object = childObject;

					if(childObject != nullptr)
						IDeltaHandler::GenerateDeltaApplyCommands(childObject->GetRtti(), childObject, serializedObjectDelta, allocator, inOutObjectMap, outCommands, context);
				}

				outCommands.push_back(command);
			}
			break;
		case RTTIFieldDataType::Reflectable:
			{
				SPtr<SerializedObject> serializedObjectDelta = std::static_pointer_cast<SerializedObject>(serializedEntryDelta);

				SPtr<IReflectable> clonedObject;
				if(fieldValue != nullptr)
				{
					const IReflectable& childObject = field.GetReflectable(fieldValue, tupleElementIndex);
					clonedObject = BinaryCloner::Clone(const_cast<IReflectable*>(&childObject), true);
				}
				else
					clonedObject = fnGetOrDeserializeObject(serializedObjectDelta);

				if(B3D_ENSURE(clonedObject != nullptr))
					IDeltaHandler::GenerateDeltaApplyCommands(clonedObject->GetRtti(), clonedObject, serializedObjectDelta, allocator, inOutObjectMap, outCommands, context);

				command.Type |= Diff_Reflectable;
				command.Object = clonedObject;

				outCommands.push_back(command);
			}
			break;
		case RTTIFieldDataType::Plain:
			{
				SPtr<SerializedPlainData> serializedPlainData = std::static_pointer_cast<SerializedPlainData>(serializedEntryDelta);

				if(serializedPlainData->Size > 0)
				{
					command.Type |= Diff_Plain;
					command.Value = serializedPlainData->Value;
					command.Size = serializedPlainData->Size;

					outCommands.push_back(command);
				}
			}
			break;
		default:
			B3D_ENSURE(false);
			return;
		}
	}

	DeltaCommand iteratorEndCommand;
	iteratorEndCommand.Field = &field;
	iteratorEndCommand.Type = Diff_IterableEntryEnd;

	if(arrayIndex != ~0u)
	{
		iteratorEndCommand.Type |= Diff_ArrayFlag;
		iteratorEndCommand.ArrayIndex = arrayIndex;
	}
	else if(mapKey != nullptr)
	{
		iteratorEndCommand.Type |= Diff_MapFlag;
		iteratorEndCommand.MapKey = mapKey;
	}

	outCommands.push_back(iteratorEndCommand);
}

void BinaryDeltaHandler::GenerateDeltaCommandForDataBlockField(RTTIField& field, const SPtr<ISerialized>& entryDelta, FrameVector<DeltaCommand>& outCommands)
{
	if(!B3D_ENSURE(field.Schema.FieldDataTypes.Size() == 1))
		return;

	if(!B3D_ENSURE(field.Schema.FieldDataTypes[0].Type == RTTIFieldDataType::DataBlock))
		return;

	SPtr<SerializedDataBlock> serializedDataBlock = std::static_pointer_cast<SerializedDataBlock>(entryDelta);
	if(!B3D_ENSURE(serializedDataBlock != nullptr))
		return;

	DeltaCommand command;
	command.Field = &field;
	command.Type = Diff_DataBlock;
	command.StreamValue = serializedDataBlock->Stream;
	command.Value = nullptr;
	command.Size = serializedDataBlock->Size;

	outCommands.push_back(command);
}
