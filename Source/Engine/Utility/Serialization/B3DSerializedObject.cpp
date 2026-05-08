//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DSerializedObject.h"
#include "Serialization/B3DIntermediateSerializer.h"
#include "RTTI/B3DSerializedObjectRTTI.h"

#include "ThirdParty/CityHash/city.h"

using namespace b3d;

namespace b3d
{
	bool Equals(const TShared<ISerialized>& lhs, const TShared<ISerialized>& rhs)
	{
		if(lhs == nullptr)
			return rhs == nullptr;

		if(rhs == nullptr) // lhs isn't null, so not equal
			return false;

		return lhs->Equals(rhs);
	}
} // namespace b3d

TShared<SerializedObject> SerializedObject::Create(IReflectable& object, SerializedObjectEncodeFlags flags)
{
	RTTIOperationContext rttiOperationContext;
	IntermediateSerializer intermediateSerializer(&GetFrameAllocator(), rttiOperationContext);
	return intermediateSerializer.Encode(&object, flags);
}

TShared<IReflectable> SerializedObject::Decode(RTTIOperationContext& context) const
{
	IntermediateSerializer intermediateSerializer(&GetFrameAllocator(), context);
	return intermediateSerializer.Decode(this);
}

u32 SerializedObject::GetRootTypeId() const
{
	if(SubObjects.size() > 0)
		return SubObjects[0].TypeId;

	return 0;
}

TShared<ISerialized> SerializedObject::Clone(bool cloneData)
{
	TShared<SerializedObject> copy = B3DMakeShared<SerializedObject>();
	copy->SubObjects = Vector<SerializedSubObject>(SubObjects.size());

	u32 subObjectIndex = 0;
	for(auto& subObject : SubObjects)
	{
		copy->SubObjects[subObjectIndex].TypeId = subObject.TypeId;

		for(auto& entryPair : subObject.FieldEntries)
		{
			SerializedField entry = entryPair.second;

			if(entry.Value != nullptr)
				entry.Value = entry.Value->Clone(cloneData);

			copy->SubObjects[subObjectIndex].FieldEntries[entryPair.first] = entry;
		}

		subObjectIndex++;
	}

	return copy;
}

u64 SerializedObject::CalculateHash() const
{
	u64 hash = 0;

	for(auto& subObject : SubObjects)
	{
		B3DCombineHash(hash, subObject.TypeId);

		for(auto& entryPair : subObject.FieldEntries)
		{
			SerializedField entry = entryPair.second;

			if(entry.Value != nullptr)
				B3DCombineHash(hash, entry.Value->CalculateHash());
		}
	}

	return hash;
}

bool SerializedObject::Equals(const TShared<ISerialized>& other) const
{
	if(TShared<SerializedObject> otherObject = B3DRTTICast<SerializedObject>(other))
	{
		if(SubObjects.size() != otherObject->SubObjects.size())
			return false;

		const u32 subObjectCount = (u32)SubObjects.size();
		for(u32 subObjectIndex = 0; subObjectIndex < subObjectCount; ++subObjectIndex)
		{
			const SerializedSubObject& mySubObject = SubObjects[subObjectIndex];
			const SerializedSubObject& otherSubObject = otherObject->SubObjects[subObjectIndex];

			if(mySubObject.TypeId != otherSubObject.TypeId)
				return false;

			if(mySubObject.FieldEntries.size() != otherSubObject.FieldEntries.size())
				return false;

			for(auto myFieldIterator = mySubObject.FieldEntries.begin(); myFieldIterator != mySubObject.FieldEntries.end(); ++myFieldIterator)
			{
				auto foundOtherField = otherSubObject.FieldEntries.find(myFieldIterator->first);
				if(foundOtherField == otherSubObject.FieldEntries.end())
					return false;
				
				if(!::Equals(myFieldIterator->second.Value, foundOtherField->second.Value))
					return false;
			}
		}

		return true;
	}

	return false;
}

TShared<ISerialized> SerializedTuple::Clone(bool cloneData)
{
	TShared<SerializedTuple> copy = B3DMakeShared<SerializedTuple>();

	for(const auto& entry : Values)
	{
		if(entry != nullptr)
			copy->Values.Add(entry->Clone(cloneData));
		else
			copy->Values.Add(nullptr);
	}

	return copy;
}

u64 SerializedTuple::CalculateHash() const
{
	u64 hash = 0;

	for(auto& entry : Values)
	{
		if(entry != nullptr)
			B3DCombineHash(hash, entry->CalculateHash());
	}

	return hash;
}

bool SerializedTuple::Equals(const TShared<ISerialized>& other) const
{
	if(TShared<SerializedTuple> otherTuple = B3DRTTICast<SerializedTuple>(other))
	{
		if(Values.Size() != otherTuple->Values.Size())
			return false;

		const u32 valueCount = (u32)Values.Size();
		for(u32 valueIndex = 0; valueIndex < valueCount; ++valueIndex)
		{
			if(!::Equals(Values[valueIndex], otherTuple->Values[valueIndex]))
				return false;
		}

		return true;
	}

	return false;
}

TShared<ISerialized> SerializedPlainData::Clone(bool cloneData)
{
	TShared<SerializedPlainData> copy = B3DMakeShared<SerializedPlainData>();
	copy->Size = Size;

	if(cloneData)
	{
		copy->Value = (u8*)B3DAllocate(Size);
		memcpy(copy->Value, Value, Size);
		copy->OwnsMemory = true;
	}
	else
	{
		copy->Value = Value;
		copy->OwnsMemory = false;
	}

	return copy;
}

u64 SerializedPlainData::CalculateHash() const
{
	return CityHash64(reinterpret_cast<char*>(Value), Size);
}

bool SerializedPlainData::Equals(const TShared<ISerialized>& other) const
{
	if(TShared<SerializedPlainData> otherPlainData = B3DRTTICast<SerializedPlainData>(other))
	{
		if(Size != otherPlainData->Size)
			return false;

		return memcmp(Value, otherPlainData->Value, Size) == 0;
	}

	return false;
}

TShared<ISerialized> SerializedDataBlock::Clone(bool cloneData)
{
	TShared<SerializedDataBlock> copy = B3DMakeShared<SerializedDataBlock>();
	copy->Size = Size;

	if(cloneData)
	{
		if(Stream->IsFile())
		{
			B3D_LOG(Warning, LogGeneric, "Cloning a file stream. Streaming is disabled and stream data will be loaded into memory.");
		}

		auto stream = B3DMakeShared<MemoryDataStream>(Size);
		stream->Read(stream->Data(), Size);

		copy->Stream = stream;
		copy->Offset = 0;
	}
	else
	{
		copy->Stream = Stream;
		copy->Offset = Offset;
	}

	return copy;
}

u64 SerializedDataBlock::CalculateHash() const
{
	u64 hash = B3DHash(Offset);

	if(Stream != nullptr)
	{
		TShared<DataStream> copy = Stream->Clone(false);
		copy->Seek(Offset);

		u64 remainingSize = Size;
		while(remainingSize > 0)
		{
			static constexpr u32 kBufferSize = 1024;

			u8* readBuffer[kBufferSize];

			const u64 amountToRead = Math::Min(kBufferSize, remainingSize);
			const u64 amountRead = copy->Read(readBuffer, amountToRead);

			B3DCombineHash(hash, CityHash64((const char*)readBuffer, amountRead));

			B3D_ASSERT(amountRead <= remainingSize);
			remainingSize -= amountRead;
		}
	}

	return hash;
}

bool SerializedDataBlock::Equals(const TShared<ISerialized>& other) const
{
	if(TShared<SerializedDataBlock> otherDataBlock = B3DRTTICast<SerializedDataBlock>(other))
	{
		if(Offset != otherDataBlock->Offset)
			return false;

		if(Size != otherDataBlock->Size)
			return false;

		if(Stream == nullptr)
			return otherDataBlock->Stream == nullptr;

		if(otherDataBlock->Stream == nullptr)
			return false;

		TShared<DataStream> myStreamCopy = Stream->Clone(false);
		myStreamCopy->Seek(Offset);

		TShared<DataStream> otherStreamCopy = otherDataBlock->Stream->Clone(false);
		otherStreamCopy->Seek(Offset);

		u64 remainingSize = Size;
		while(remainingSize > 0)
		{
			static constexpr u32 kBufferSize = 1024;

			u8* myReadBuffer[kBufferSize];
			u8* otherReadBuffer[kBufferSize];

			const u64 amountToRead = Math::Min(kBufferSize, remainingSize);

			const u64 myAmountRead = myStreamCopy->Read(myReadBuffer, amountToRead);
			const u64 otherAmountRead = otherStreamCopy->Read(otherReadBuffer, amountToRead);

			if(myAmountRead != otherAmountRead)
				return false;

			if(memcmp(myReadBuffer, otherReadBuffer, myAmountRead) != 0)
				return false;

			B3D_ASSERT(myAmountRead <= remainingSize);
			remainingSize -= myAmountRead;
		}

		return true;
	}

	return false;
}

TShared<ISerialized> SerializedArray::Clone(bool cloneData)
{
	TShared<SerializedArray> copy = B3DMakeShared<SerializedArray>();

	for(const auto& entry : Entries)
	{
		if(entry != nullptr)
			copy->Entries.Add(entry->Clone(cloneData));
		else
			copy->Entries.Add(nullptr);
	}

	return copy;
}

u64 SerializedArray::CalculateHash() const
{
	u64 hash = 0;
	B3DCombineHash(hash, Entries.Size());

	for(const auto& entry : Entries)
	{
		if(entry != nullptr)
			B3DCombineHash(hash, entry->CalculateHash());
	}

	return hash;
}

bool SerializedArray::Equals(const TShared<ISerialized>& other) const
{
	if(TShared<SerializedArray> otherArray = B3DRTTICast<SerializedArray>(other))
	{
		if(Entries.size() != otherArray->Entries.size())
			return false;

		const u64 entryCount = Entries.size();
		for(u64 entryIndex = 0; entryIndex < entryCount; ++entryIndex)
		{
			if(!::Equals(Entries[entryIndex], otherArray->Entries[entryIndex]))
				return false;
		}

		return true;
	}

	return false;
}

TShared<ISerialized> SerializedMap::Clone(bool cloneData)
{
	TShared<SerializedMap> copy = B3DMakeShared<SerializedMap>();

	for(auto& entryPair : Entries)
	{
		std::pair<TShared<ISerialized>, TShared<ISerialized>> copiedEntry;
		if(entryPair.first != nullptr)
			copiedEntry.first = entryPair.first->Clone(cloneData);

		if(entryPair.second != nullptr)
			copiedEntry.second = entryPair.second->Clone(cloneData);

		copy->Entries.insert(copiedEntry);
	}

	return copy;
}

u64 SerializedMap::CalculateHash() const
{
	u64 hash = B3DHash(Entries.size());

	for(auto& entryPair : Entries)
	{
		if(entryPair.first != nullptr)
			B3DCombineHash(hash, entryPair.first->CalculateHash());

		if(entryPair.second != nullptr)
			B3DCombineHash(hash, entryPair.second->CalculateHash());
	}

	return hash;
}

bool SerializedMap::Equals(const TShared<ISerialized>& other) const
{
	if(TShared<SerializedMap> otherMap = B3DRTTICast<SerializedMap>(other))
	{
		if(Entries.size() != otherMap->Entries.size())
			return false;

		for(auto myEntryIterator = Entries.begin(); myEntryIterator != Entries.end(); ++myEntryIterator)
		{
			auto foundOtherEntry = otherMap->Entries.find(myEntryIterator->first);
			if(foundOtherEntry == otherMap->Entries.end())
				return false;

			if(!::Equals(myEntryIterator->second, foundOtherEntry->second))
				return false;
		}

		return true;
	}

	return false;
}

RTTIType* ISerialized::GetRttiStatic()
{
	return ISerializedRTTI::Instance();
}

RTTIType* ISerialized::GetRtti() const
{
	return ISerialized::GetRttiStatic();
}

RTTIType* SerializedDataBlock::GetRttiStatic()
{
	return SerializedDataBlockRTTI::Instance();
}

RTTIType* SerializedDataBlock::GetRtti() const
{
	return SerializedDataBlock::GetRttiStatic();
}

RTTIType* SerializedPlainData::GetRttiStatic()
{
	return SerializedPlainDataRTTI::Instance();
}

RTTIType* SerializedPlainData::GetRtti() const
{
	return SerializedPlainData::GetRttiStatic();
}

RTTIType* SerializedObject::GetRttiStatic()
{
	return SerializedObjectRTTI::Instance();
}

RTTIType* SerializedObject::GetRtti() const
{
	return SerializedObject::GetRttiStatic();
}

RTTIType* SerializedArray::GetRttiStatic()
{
	return SerializedArrayRTTI::Instance();
}

RTTIType* SerializedArray::GetRtti() const
{
	return SerializedArray::GetRttiStatic();
}

RTTIType* SerializedMap::GetRttiStatic()
{
	return SerializedMapRTTI::Instance();
}

RTTIType* SerializedMap::GetRtti() const
{
	return SerializedMap::GetRttiStatic();
}

RTTIType* SerializedSubObject::GetRttiStatic()
{
	return SerializedSubObjectRTTI::Instance();
}

RTTIType* SerializedSubObject::GetRtti() const
{
	return SerializedSubObject::GetRttiStatic();
}

RTTIType* SerializedField::GetRttiStatic()
{
	return SerializedFieldRTTI::Instance();
}

RTTIType* SerializedField::GetRtti() const
{
	return SerializedField::GetRttiStatic();
}

RTTIType* SerializedTuple::GetRttiStatic()
{
	return SerializedTupleRTTI::Instance();
}

RTTIType* SerializedTuple::GetRtti() const
{
	return GetRttiStatic();
}
