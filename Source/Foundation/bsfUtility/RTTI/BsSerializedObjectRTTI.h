//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Reflection/BsRTTIType.h"
#include "Reflection/BsRTTIPlain.h"
#include "Serialization/BsSerializedObject.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Utility
	 *  @{
	 */

	class BS_UTILITY_EXPORT SerializedInstanceRTTI : public RTTIType <SerializedInstance, IReflectable, SerializedInstanceRTTI>
	{
	public:
		const String& getRTTIName() override
		{
			static String name = "SerializedInstance";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializedInstance;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return nullptr;
		}
	};

	class BS_UTILITY_EXPORT SerializedFieldRTTI : public RTTIType <SerializedField, SerializedInstance, SerializedFieldRTTI>
	{
	private:
		SPtr<DataStream> getData(SerializedField* obj, UINT32& size)
		{
			size = obj->size;

			return bs_shared_ptr_new<MemoryDataStream>(obj->value, obj->size);
		}

		void setData(SerializedField* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			obj->value = (UINT8*)bs_alloc(size);
			obj->size = size;
			obj->ownsMemory = true;

			value->read(obj->value, size);
		}

	public:
		SerializedFieldRTTI()
		{
			addDataBlockField("data", 0, &SerializedFieldRTTI::getData, &SerializedFieldRTTI::setData);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializedField";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializedField;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedField>();
		}
	};

	class BS_UTILITY_EXPORT SerializedDataBlockRTTI : public RTTIType <SerializedDataBlock, SerializedInstance, SerializedDataBlockRTTI>
	{
	private:
		SPtr<DataStream> getData(SerializedDataBlock* obj, UINT32& size)
		{
			size = obj->size;
			obj->stream->seek(obj->offset);

			return obj->stream;
		}

		void setData(SerializedDataBlock* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			SPtr<MemoryDataStream> memStream = bs_shared_ptr_new<MemoryDataStream>(size);
			value->read(memStream->data(), size);

			obj->stream = memStream;
			obj->size = size;
			obj->offset = 0;
		}
	public:
		SerializedDataBlockRTTI()
		{
			addDataBlockField("data", 0, &SerializedDataBlockRTTI::getData, &SerializedDataBlockRTTI::setData);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializedDataBlock";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializedDataBlock;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedDataBlock>();
		}
	};

	class BS_UTILITY_EXPORT SerializedObjectRTTI : public RTTIType <SerializedObject, SerializedInstance, SerializedObjectRTTI>
	{
	private:
		SerializedSubObject& getEntry(SerializedObject* obj, UINT32 arrayIdx)
		{
			return obj->subObjects[arrayIdx];
		}

		void setEntry(SerializedObject* obj, UINT32 arrayIdx, SerializedSubObject& val)
		{
			obj->subObjects[arrayIdx] = val;
		}

		UINT32 getNumEntries(SerializedObject* obj)
		{
			return (UINT32)obj->subObjects.size();
		}

		void setNumEntries(SerializedObject* obj, UINT32 numEntries)
		{
			obj->subObjects = Vector<SerializedSubObject>(numEntries);
		}
	public:
		SerializedObjectRTTI()
		{
			addReflectableArrayField("entries", 1, &SerializedObjectRTTI::getEntry, &SerializedObjectRTTI::getNumEntries,
				&SerializedObjectRTTI::setEntry, &SerializedObjectRTTI::setNumEntries);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializedObject";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializedObject;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedObject>();
		}
	};

	class BS_UTILITY_EXPORT SerializedArrayRTTI : public RTTIType <SerializedArray, SerializedInstance, SerializedArrayRTTI>
	{
	private:
		UINT32& getNumElements(SerializedArray* obj)
		{
			return obj->numElements;
		}

		void setNumElements(SerializedArray* obj, UINT32& val)
		{
			obj->numElements = val;
		}

		SerializedArrayEntry& getEntry(SerializedArray* obj, UINT32 arrayIdx)
		{
			return mSequentialEntries[arrayIdx];
		}

		void setEntry(SerializedArray* obj, UINT32 arrayIdx, SerializedArrayEntry& val)
		{
			obj->entries[val.index] = val;
		}

		UINT32 getNumEntries(SerializedArray* obj)
		{
			return (UINT32)mSequentialEntries.size();
		}

		void setNumEntries(SerializedArray* obj, UINT32 numEntries)
		{
			obj->entries = UnorderedMap<UINT32, SerializedArrayEntry>();
		}
	public:
		SerializedArrayRTTI()
		{
			addPlainField("numElements", 0, &SerializedArrayRTTI::getNumElements, &SerializedArrayRTTI::setNumElements);
			addReflectableArrayField("entries", 1, &SerializedArrayRTTI::getEntry, &SerializedArrayRTTI::getNumEntries,
				&SerializedArrayRTTI::setEntry, &SerializedArrayRTTI::setNumEntries);
		}

		void onSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			SerializedArray* serializedArray = static_cast<SerializedArray*>(obj);

			for (auto& entry : serializedArray->entries)
				mSequentialEntries.push_back(entry.second);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializedArray";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializedArray;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedArray>();
		}

	private:
		Vector<SerializedArrayEntry> mSequentialEntries;
	};

	class BS_UTILITY_EXPORT SerializedSubObjectRTTI : public RTTIType <SerializedSubObject, IReflectable, SerializedSubObjectRTTI>
	{
	private:
		UINT32& getTypeId(SerializedSubObject* obj)
		{
			return obj->typeId;
		}

		void setTypeId(SerializedSubObject* obj, UINT32& val)
		{
			obj->typeId = val;
		}

		SerializedEntry& getEntry(SerializedSubObject* obj, UINT32 arrayIdx)
		{
			return mSequentialEntries[arrayIdx];
		}

		void setEntry(SerializedSubObject* obj, UINT32 arrayIdx, SerializedEntry& val)
		{
			obj->entries[val.fieldId] = val;
		}

		UINT32 getNumEntries(SerializedSubObject* obj)
		{
			return (UINT32)mSequentialEntries.size();
		}

		void setNumEntries(SerializedSubObject* obj, UINT32 numEntries)
		{
			obj->entries = UnorderedMap<UINT32, SerializedEntry>();
		}
	public:
		SerializedSubObjectRTTI()
		{
			addPlainField("typeId", 0, &SerializedSubObjectRTTI::getTypeId, &SerializedSubObjectRTTI::setTypeId);
			addReflectableArrayField("entries", 1, &SerializedSubObjectRTTI::getEntry, &SerializedSubObjectRTTI::getNumEntries,
				&SerializedSubObjectRTTI::setEntry, &SerializedSubObjectRTTI::setNumEntries);
		}

		void onSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			SerializedSubObject* serializableObject = static_cast<SerializedSubObject*>(obj);

			for (auto& entry : serializableObject->entries)
				mSequentialEntries.push_back(entry.second);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializedSubObject";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializedSubObject;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedSubObject>();
		}

	private:
		Vector<SerializedEntry> mSequentialEntries;
	};

	class BS_UTILITY_EXPORT SerializedEntryRTTI : public RTTIType <SerializedEntry, IReflectable, SerializedEntryRTTI>
	{
	private:
		UINT32& getFieldId(SerializedEntry* obj)
		{
			return obj->fieldId;
		}

		void setFieldId(SerializedEntry* obj, UINT32& val)
		{
			obj->fieldId = val;
		}

		SPtr<SerializedInstance> getSerialized(SerializedEntry* obj)
		{
			return obj->serialized;
		}

		void setSerialized(SerializedEntry* obj, SPtr<SerializedInstance> val)
		{
			obj->serialized = val;
		}

	public:
		SerializedEntryRTTI()
		{
			addPlainField("fieldId", 0, &SerializedEntryRTTI::getFieldId, &SerializedEntryRTTI::setFieldId);
			addReflectablePtrField("serialized", 1, &SerializedEntryRTTI::getSerialized, &SerializedEntryRTTI::setSerialized);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializedEntry";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializedEntry;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedEntry>();
		}
	};

	class BS_UTILITY_EXPORT SerializedArrayEntryRTTI : public RTTIType <SerializedArrayEntry, IReflectable, SerializedArrayEntryRTTI>
	{
	private:
		UINT32& getArrayIdx(SerializedArrayEntry* obj)
		{
			return obj->index;
		}

		void setArrayIdx(SerializedArrayEntry* obj, UINT32& val)
		{
			obj->index = val;
		}

		SPtr<SerializedInstance> getSerialized(SerializedArrayEntry* obj)
		{
			return obj->serialized;
		}

		void setSerialized(SerializedArrayEntry* obj, SPtr<SerializedInstance> val)
		{
			obj->serialized = val;
		}

	public:
		SerializedArrayEntryRTTI()
		{
			addPlainField("index", 0, &SerializedArrayEntryRTTI::getArrayIdx, &SerializedArrayEntryRTTI::setArrayIdx);
			addReflectablePtrField("serialized", 1, &SerializedArrayEntryRTTI::getSerialized, &SerializedArrayEntryRTTI::setSerialized);
		}

		const String& getRTTIName() override
		{
			static String name = "SerializedArrayEntry";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_SerializedArrayEntry;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedArrayEntry>();
		}
	};

	/** @} */
	/** @endcond */
}
