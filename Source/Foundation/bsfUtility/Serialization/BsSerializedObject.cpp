//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsSerializedObject.h"
#include "Serialization/BsIntermediateSerializer.h"
#include "RTTI/BsSerializedObjectRTTI.h"

namespace bs
{
	SPtr<SerializedObject> SerializedObject::create(IReflectable& obj, SerializedObjectEncodeFlags flags, SerializationContext* context)
	{
		IntermediateSerializer is;
		return is.encode(&obj, flags, context);
	}

	SPtr<IReflectable> SerializedObject::decode(SerializationContext* context) const
	{
		IntermediateSerializer is;
		return is.decode(this, context);
	}

	SPtr<SerializedInstance> SerializedObject::clone(bool cloneData)
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
					entry.serialized = entry.serialized->clone(cloneData);

				copy->subObjects[i].entries[entryPair.first] = entry;
			}

			i++;
		}

		return copy;
	}

	SPtr<SerializedInstance> SerializedField::clone(bool cloneData)
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

	SPtr<SerializedInstance> SerializedDataBlock::clone(bool cloneData)
	{
		SPtr<SerializedDataBlock> copy = bs_shared_ptr_new<SerializedDataBlock>();
		copy->size = size;

		if (cloneData)
		{
			if(stream->isFile())
			{
				BS_LOG(Warning, Generic,
					"Cloning a file stream. Streaming is disabled and stream data will be loaded into memory.");
			}

			auto stream = bs_shared_ptr_new<MemoryDataStream>(size);
			stream->read(stream->data(), size);

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

	SPtr<SerializedInstance> SerializedArray::clone(bool cloneData)
	{
		SPtr<SerializedArray> copy = bs_shared_ptr_new<SerializedArray>();
		copy->numElements = numElements;

		for (auto& entryPair : entries)
		{
			SerializedArrayEntry entry = entryPair.second;
			entry.serialized = entry.serialized->clone(cloneData);

			copy->entries[entryPair.first] = entry;
		}

		return copy;
	}

	RTTITypeBase* SerializedInstance::getRTTIStatic()
	{
		return SerializedInstanceRTTI::instance();
	}

	RTTITypeBase* SerializedInstance::getRTTI() const
	{
		return SerializedInstance::getRTTIStatic();
	}

	RTTITypeBase* SerializedDataBlock::getRTTIStatic()
	{
		return SerializedDataBlockRTTI::instance();
	}

	RTTITypeBase* SerializedDataBlock::getRTTI() const
	{
		return SerializedDataBlock::getRTTIStatic();
	}

	RTTITypeBase* SerializedField::getRTTIStatic()
	{
		return SerializedFieldRTTI::instance();
	}

	RTTITypeBase* SerializedField::getRTTI() const
	{
		return SerializedField::getRTTIStatic();
	}

	UINT32 SerializedObject::getRootTypeId() const
	{
		if(subObjects.size() > 0)
			return subObjects[0].typeId;

		return 0;
	}

	RTTITypeBase* SerializedObject::getRTTIStatic()
	{
		return SerializedObjectRTTI::instance();
	}

	RTTITypeBase* SerializedObject::getRTTI() const
	{
		return SerializedObject::getRTTIStatic();
	}

	RTTITypeBase* SerializedArray::getRTTIStatic()
	{
		return SerializedArrayRTTI::instance();
	}

	RTTITypeBase* SerializedArray::getRTTI() const
	{
		return SerializedArray::getRTTIStatic();
	}

	RTTITypeBase* SerializedSubObject::getRTTIStatic()
	{
		return SerializedSubObjectRTTI::instance();
	}

	RTTITypeBase* SerializedSubObject::getRTTI() const
	{
		return SerializedSubObject::getRTTIStatic();
	}

	RTTITypeBase* SerializedEntry::getRTTIStatic()
	{
		return SerializedEntryRTTI::instance();
	}

	RTTITypeBase* SerializedEntry::getRTTI() const
	{
		return SerializedEntry::getRTTIStatic();
	}

	RTTITypeBase* SerializedArrayEntry::getRTTIStatic()
	{
		return SerializedArrayEntryRTTI::instance();
	}

	RTTITypeBase* SerializedArrayEntry::getRTTI() const
	{
		return SerializedArrayEntry::getRTTIStatic();
	}
}
