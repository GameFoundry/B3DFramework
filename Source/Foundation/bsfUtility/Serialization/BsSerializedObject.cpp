//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsSerializedObject.h"
#include "Serialization/BsIntermediateSerializer.h"
#include "RTTI/BsSerializedObjectRTTI.h"

using namespace bs;

SPtr<SerializedObject> SerializedObject::Create(IReflectable& object, SerializedObjectEncodeFlags flags, SerializationContext* context)
{
	IntermediateSerializer is;
	return is.Encode(&object, flags, context);
}

SPtr<IReflectable> SerializedObject::Decode(SerializationContext* context) const
{
	IntermediateSerializer is;
	return is.Decode(this, context);
}

SPtr<ISerialized> SerializedObject::Clone(bool cloneData)
{
	SPtr<SerializedObject> copy = B3DMakeShared<SerializedObject>();
	copy->SubObjects = Vector<SerializedSubObject>(SubObjects.size());

	u32 i = 0;
	for(auto& subObject : SubObjects)
	{
		copy->SubObjects[i].TypeId = subObject.TypeId;

		for(auto& entryPair : subObject.FieldEntries)
		{
			SerializedField entry = entryPair.second;

			if(entry.Value != nullptr)
				entry.Value = entry.Value->Clone(cloneData);

			copy->SubObjects[i].FieldEntries[entryPair.first] = entry;
		}

		i++;
	}

	return copy;
}

SPtr<ISerialized> SerializedTuple::Clone(bool cloneData)
{
	SPtr<SerializedTuple> copy = B3DMakeShared<SerializedTuple>();

	for(const auto& entry : Values)
	{
		if(entry != nullptr)
			copy->Values.Add(entry->Clone(cloneData));
		else
			copy->Values.Add(nullptr);
	}

	return copy;
}

SPtr<ISerialized> SerializedPlainData::Clone(bool cloneData)
{
	SPtr<SerializedPlainData> copy = B3DMakeShared<SerializedPlainData>();
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

SPtr<ISerialized> SerializedDataBlock::Clone(bool cloneData)
{
	SPtr<SerializedDataBlock> copy = B3DMakeShared<SerializedDataBlock>();
	copy->Size = Size;

	if(cloneData)
	{
		if(Stream->IsFile())
		{
			B3D_LOG(Warning, Generic, "Cloning a file stream. Streaming is disabled and stream data will be loaded into memory.");
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

SPtr<ISerialized> SerializedArray::Clone(bool cloneData)
{
	SPtr<SerializedArray> copy = B3DMakeShared<SerializedArray>();
	copy->ElementCount = ElementCount;

	for(auto& entryPair : Entries)
	{
		SerializedArrayEntry arrayEntry = entryPair.second;

		if(arrayEntry.Value != nullptr)
			arrayEntry.Value = arrayEntry.Value->Clone(cloneData);

		copy->Entries[entryPair.first] = arrayEntry;
	}

	return copy;
}

RTTITypeBase* ISerialized::GetRttiStatic()
{
	return ISerializedRTTI::Instance();
}

RTTITypeBase* ISerialized::GetRtti() const
{
	return ISerialized::GetRttiStatic();
}

RTTITypeBase* SerializedDataBlock::GetRttiStatic()
{
	return SerializedDataBlockRTTI::Instance();
}

RTTITypeBase* SerializedDataBlock::GetRtti() const
{
	return SerializedDataBlock::GetRttiStatic();
}

RTTITypeBase* SerializedPlainData::GetRttiStatic()
{
	return SerializedPlainDataRTTI::Instance();
}

RTTITypeBase* SerializedPlainData::GetRtti() const
{
	return SerializedPlainData::GetRttiStatic();
}

u32 SerializedObject::GetRootTypeId() const
{
	if(SubObjects.size() > 0)
		return SubObjects[0].TypeId;

	return 0;
}

RTTITypeBase* SerializedObject::GetRttiStatic()
{
	return SerializedObjectRTTI::Instance();
}

RTTITypeBase* SerializedObject::GetRtti() const
{
	return SerializedObject::GetRttiStatic();
}

RTTITypeBase* SerializedArray::GetRttiStatic()
{
	return SerializedArrayRTTI::Instance();
}

RTTITypeBase* SerializedArray::GetRtti() const
{
	return SerializedArray::GetRttiStatic();
}

RTTITypeBase* SerializedSubObject::GetRttiStatic()
{
	return SerializedSubObjectRTTI::Instance();
}

RTTITypeBase* SerializedSubObject::GetRtti() const
{
	return SerializedSubObject::GetRttiStatic();
}

RTTITypeBase* SerializedField::GetRttiStatic()
{
	return SerializedFieldRTTI::Instance();
}

RTTITypeBase* SerializedField::GetRtti() const
{
	return SerializedField::GetRttiStatic();
}

RTTITypeBase* SerializedTuple::GetRttiStatic()
{
	return SerializedTupleRTTI::Instance();
}

RTTITypeBase* SerializedTuple::GetRtti() const
{
	return GetRttiStatic();
}

RTTITypeBase* SerializedArrayEntry::GetRttiStatic()
{
	return SerializedArrayEntryRTTI::Instance();
}

RTTITypeBase* SerializedArrayEntry::GetRtti() const
{
	return SerializedArrayEntry::GetRttiStatic();
}
