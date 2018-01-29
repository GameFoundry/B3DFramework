//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Serialization/BsSerializedObject.h"
#include "Private/RTTI/BsSerializedObjectRTTI.h"

namespace bs
{
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
				LOGWRN("Cloning a file stream. Streaming is disabled and stream data will be loaded into memory.");

			UINT8* data = (UINT8*)bs_alloc(size);
			stream->read(data, size);

			copy->stream = bs_shared_ptr_new<MemoryDataStream>(data, size);
			copy->offset = 0;
		}
		else
		{
			copy->stream = stream;
			copy->offset = offset;
		}

		return copy;
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