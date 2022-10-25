//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsSerializedObject.h"
#include "Serialization/BsIntermediateSerializer.h"
#include "RTTI/BsSerializedObjectRTTI.h"

using namespace bs;

SPtr<SerializedObject> SerializedObject::Create(IReflectable& obj, SerializedObjectEncodeFlags flags, SerializationContext* context)
{
	IntermediateSerializer is;
	return is.Encode(&obj, flags, context);
}

SPtr<IReflectable> SerializedObject::Decode(SerializationContext* context) const
{
	IntermediateSerializer is;
	return is.Decode(this, context);
}

SPtr<SerializedInstance> SerializedObject::Clone(bool cloneData)
{
	SPtr<SerializedObject> copy = bs_shared_ptr_new<SerializedObject>();
	copy->SubObjects = Vector<SerializedSubObject>(SubObjects.size());

	u32 i = 0;
	for(auto& subObject : SubObjects)
	{
		copy->SubObjects[i].TypeId = subObject.TypeId;

		for(auto& entryPair : subObject.Entries)
		{
			SerializedEntry entry = entryPair.second;

			if(entry.Serialized != nullptr)
				entry.Serialized = entry.Serialized->Clone(cloneData);

			copy->SubObjects[i].Entries[entryPair.first] = entry;
		}

		i++;
	}

	return copy;
}

SPtr<SerializedInstance> SerializedField::Clone(bool cloneData)
{
	SPtr<SerializedField> copy = bs_shared_ptr_new<SerializedField>();
	copy->Size = Size;

	if(cloneData)
	{
		copy->Value = (u8*)bs_alloc(Size);
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

SPtr<SerializedInstance> SerializedDataBlock::Clone(bool cloneData)
{
	SPtr<SerializedDataBlock> copy = bs_shared_ptr_new<SerializedDataBlock>();
	copy->Size = Size;

	if(cloneData)
	{
		if(Stream->IsFile())
		{
			BS_LOG(Warning, Generic, "Cloning a file stream. Streaming is disabled and stream data will be loaded into memory.");
		}

		auto stream = bs_shared_ptr_new<MemoryDataStream>(Size);
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

SPtr<SerializedInstance> SerializedArray::Clone(bool cloneData)
{
	SPtr<SerializedArray> copy = bs_shared_ptr_new<SerializedArray>();
	copy->NumElements = NumElements;

	for(auto& entryPair : Entries)
	{
		SerializedArrayEntry entry = entryPair.second;
		entry.Serialized = entry.Serialized->Clone(cloneData);

		copy->Entries[entryPair.first] = entry;
	}

	return copy;
}

RTTITypeBase* SerializedInstance::GetRttiStatic()
{
	return SerializedInstanceRTTI::Instance();
}

RTTITypeBase* SerializedInstance::GetRtti() const
{
	return SerializedInstance::GetRttiStatic();
}

RTTITypeBase* SerializedDataBlock::GetRttiStatic()
{
	return SerializedDataBlockRTTI::Instance();
}

RTTITypeBase* SerializedDataBlock::GetRtti() const
{
	return SerializedDataBlock::GetRttiStatic();
}

RTTITypeBase* SerializedField::GetRttiStatic()
{
	return SerializedFieldRTTI::Instance();
}

RTTITypeBase* SerializedField::GetRtti() const
{
	return SerializedField::GetRttiStatic();
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

RTTITypeBase* SerializedEntry::GetRttiStatic()
{
	return SerializedEntryRTTI::Instance();
}

RTTITypeBase* SerializedEntry::GetRtti() const
{
	return SerializedEntry::GetRttiStatic();
}

RTTITypeBase* SerializedArrayEntry::GetRttiStatic()
{
	return SerializedArrayEntryRTTI::Instance();
}

RTTITypeBase* SerializedArrayEntry::GetRtti() const
{
	return SerializedArrayEntry::GetRttiStatic();
}
