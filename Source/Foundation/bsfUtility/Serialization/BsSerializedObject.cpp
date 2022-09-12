//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsSerializedObject.h"
#include "Serialization/BsIntermediateSerializer.h"
#include "RTTI/BsSerializedObjectRTTI.h"

namespace bs
{
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
		copy->subObjects = Vector<SerializedSubObject>(subObjects.size());

		UINT32 i = 0;
		for (auto& subObject : subObjects)
		{
			copy->subObjects[i].typeId = subObject.typeId;

			for (auto& entryPair : subObject.entries)
			{
				SerializedEntry entry = entryPair.second;

				if (entry.serialized != nullptr)
					entry.serialized = entry.serialized->Clone(cloneData);

				copy->subObjects[i].entries[entryPair.first] = entry;
			}

			i++;
		}

		return copy;
	}

	SPtr<SerializedInstance> SerializedField::Clone(bool cloneData)
	{
		SPtr<SerializedField> copy = bs_shared_ptr_new<SerializedField>();
		copy->size = size;

		if (cloneData)
		{
			copy->value = (UINT8*)bs_alloc(size);
			memcpy(copy->value, value, size);
			copy->ownsMemory = true;
		}
		else
		{
			copy->value = value;
			copy->ownsMemory = false;
		}

		return copy;
	}

	SPtr<SerializedInstance> SerializedDataBlock::Clone(bool cloneData)
	{
		SPtr<SerializedDataBlock> copy = bs_shared_ptr_new<SerializedDataBlock>();
		copy->size = size;

		if (cloneData)
		{
			if(stream->IsFile())
			{
				BS_LOG(Warning, Generic,
					"Cloning a file stream. Streaming is disabled and stream data will be loaded into memory.");
			}

			auto stream = bs_shared_ptr_new<MemoryDataStream>(size);
			stream->Read(stream->data(), size);

			copy->stream = stream;
			copy->offset = 0;
		}
		else
		{
			copy->stream = stream;
			copy->offset = offset;
		}

		return copy;
	}

	SPtr<SerializedInstance> SerializedArray::Clone(bool cloneData)
	{
		SPtr<SerializedArray> copy = bs_shared_ptr_new<SerializedArray>();
		copy->numElements = numElements;

		for (auto& entryPair : entries)
		{
			SerializedArrayEntry entry = entryPair.second;
			entry.serialized = entry.serialized->Clone(cloneData);

			copy->entries[entryPair.first] = entry;
		}

		return copy;
	}

	RTTITypeBase* SerializedInstance::getRTTIStatic()
	{
		return SerializedInstanceRTTI::Instance();
	}

	RTTITypeBase* SerializedInstance::getRTTI() const
	{
		return SerializedInstance::GetRTTIStatic();
	}

	RTTITypeBase* SerializedDataBlock::getRTTIStatic()
	{
		return SerializedDataBlockRTTI::Instance();
	}

	RTTITypeBase* SerializedDataBlock::getRTTI() const
	{
		return SerializedDataBlock::GetRTTIStatic();
	}

	RTTITypeBase* SerializedField::getRTTIStatic()
	{
		return SerializedFieldRTTI::Instance();
	}

	RTTITypeBase* SerializedField::getRTTI() const
	{
		return SerializedField::GetRTTIStatic();
	}

	UINT32 SerializedObject::GetRootTypeId() const
	{
		if(subObjects.size() > 0)
			return subObjects[0].typeId;

		return 0;
	}

	RTTITypeBase* SerializedObject::getRTTIStatic()
	{
		return SerializedObjectRTTI::Instance();
	}

	RTTITypeBase* SerializedObject::getRTTI() const
	{
		return SerializedObject::GetRTTIStatic();
	}

	RTTITypeBase* SerializedArray::getRTTIStatic()
	{
		return SerializedArrayRTTI::Instance();
	}

	RTTITypeBase* SerializedArray::getRTTI() const
	{
		return SerializedArray::GetRTTIStatic();
	}

	RTTITypeBase* SerializedSubObject::getRTTIStatic()
	{
		return SerializedSubObjectRTTI::Instance();
	}

	RTTITypeBase* SerializedSubObject::getRTTI() const
	{
		return SerializedSubObject::GetRTTIStatic();
	}

	RTTITypeBase* SerializedEntry::getRTTIStatic()
	{
		return SerializedEntryRTTI::Instance();
	}

	RTTITypeBase* SerializedEntry::getRTTI() const
	{
		return SerializedEntry::GetRTTIStatic();
	}

	RTTITypeBase* SerializedArrayEntry::getRTTIStatic()
	{
		return SerializedArrayEntryRTTI::Instance();
	}

	RTTITypeBase* SerializedArrayEntry::getRTTI() const
	{
		return SerializedArrayEntry::GetRTTIStatic();
	}
}
