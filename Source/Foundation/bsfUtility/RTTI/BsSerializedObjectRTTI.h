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
		const String& GetRTTIName() override
		{
			static String name = "SerializedInstance";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_SerializedInstance;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return nullptr;
		}
	};

	class BS_UTILITY_EXPORT SerializedFieldRTTI : public RTTIType <SerializedField, SerializedInstance, SerializedFieldRTTI>
	{
	private:
		SPtr<DataStream> GetData(SerializedField* obj, UINT32& size)
		{
			size = obj->size;

			return bs_shared_ptr_new<MemoryDataStream>(obj->value, obj->size);
		}

		void SetData(SerializedField* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			obj->value = (UINT8*)bs_alloc(size);
			obj->size = size;
			obj->ownsMemory = true;

			value->Read(obj->value, size);
		}

	public:
		SerializedFieldRTTI()
		{
			addDataBlockField("data", 0, &SerializedFieldRTTI::getData, &SerializedFieldRTTI::setData);
		}

		const String& GetRTTIName() override
		{
			static String name = "SerializedField";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_SerializedField;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedField>();
		}
	};

	class BS_UTILITY_EXPORT SerializedDataBlockRTTI : public RTTIType <SerializedDataBlock, SerializedInstance, SerializedDataBlockRTTI>
	{
	private:
		SPtr<DataStream> GetData(SerializedDataBlock* obj, UINT32& size)
		{
			size = obj->size;
			obj->stream->Seek(obj->offset);

			return obj->stream;
		}

		void SetData(SerializedDataBlock* obj, const SPtr<DataStream>& value, UINT32 size)
		{
			SPtr<MemoryDataStream> memStream = bs_shared_ptr_new<MemoryDataStream>(size);
			value->Read(memStream->data(), size);

			obj->stream = memStream;
			obj->size = size;
			obj->offset = 0;
		}
	public:
		SerializedDataBlockRTTI()
		{
			addDataBlockField("data", 0, &SerializedDataBlockRTTI::getData, &SerializedDataBlockRTTI::setData);
		}

		const String& GetRTTIName() override
		{
			static String name = "SerializedDataBlock";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_SerializedDataBlock;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedDataBlock>();
		}
	};

	class BS_UTILITY_EXPORT SerializedObjectRTTI : public RTTIType <SerializedObject, SerializedInstance, SerializedObjectRTTI>
	{
	private:
		SerializedSubObject& GetEntry(SerializedObject* obj, UINT32 arrayIdx)
		{
			return obj->subObjects[arrayIdx];
		}

		void SetEntry(SerializedObject* obj, UINT32 arrayIdx, SerializedSubObject& val)
		{
			obj->subObjects[arrayIdx] = val;
		}

		UINT32 GetNumEntries(SerializedObject* obj)
		{
			return (UINT32)obj->subObjects.Size();
		}

		void SetNumEntries(SerializedObject* obj, UINT32 numEntries)
		{
			obj->subObjects = Vector<SerializedSubObject>(numEntries);
		}
	public:
		SerializedObjectRTTI()
		{
			addReflectableArrayField("entries", 1, &SerializedObjectRTTI::getEntry, &SerializedObjectRTTI::getNumEntries,
				&SerializedObjectRTTI::setEntry, &SerializedObjectRTTI::setNumEntries);
		}

		const String& GetRTTIName() override
		{
			static String name = "SerializedObject";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_SerializedObject;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedObject>();
		}
	};

	class BS_UTILITY_EXPORT SerializedArrayRTTI : public RTTIType <SerializedArray, SerializedInstance, SerializedArrayRTTI>
	{
	private:
		UINT32& GetNumElements(SerializedArray* obj)
		{
			return obj->numElements;
		}

		void SetNumElements(SerializedArray* obj, UINT32& val)
		{
			obj->numElements = val;
		}

		SerializedArrayEntry& GetEntry(SerializedArray* obj, UINT32 arrayIdx)
		{
			return mSequentialEntries[arrayIdx];
		}

		void SetEntry(SerializedArray* obj, UINT32 arrayIdx, SerializedArrayEntry& val)
		{
			obj->entries[val.index] = val;
		}

		UINT32 GetNumEntries(SerializedArray* obj)
		{
			return (UINT32)mSequentialEntries.Size();
		}

		void SetNumEntries(SerializedArray* obj, UINT32 numEntries)
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

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			SerializedArray* serializedArray = static_cast<SerializedArray*>(obj);

			for (auto& entry : serializedArray->entries)
				mSequentialEntries.push_back(entry.second);
		}

		const String& GetRTTIName() override
		{
			static String name = "SerializedArray";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_SerializedArray;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedArray>();
		}

	private:
		Vector<SerializedArrayEntry> mSequentialEntries;
	};

	class BS_UTILITY_EXPORT SerializedSubObjectRTTI : public RTTIType <SerializedSubObject, IReflectable, SerializedSubObjectRTTI>
	{
	private:
		UINT32& GetTypeId(SerializedSubObject* obj)
		{
			return obj->typeId;
		}

		void SetTypeId(SerializedSubObject* obj, UINT32& val)
		{
			obj->typeId = val;
		}

		SerializedEntry& GetEntry(SerializedSubObject* obj, UINT32 arrayIdx)
		{
			return mSequentialEntries[arrayIdx];
		}

		void SetEntry(SerializedSubObject* obj, UINT32 arrayIdx, SerializedEntry& val)
		{
			obj->entries[val.fieldId] = val;
		}

		UINT32 GetNumEntries(SerializedSubObject* obj)
		{
			return (UINT32)mSequentialEntries.Size();
		}

		void SetNumEntries(SerializedSubObject* obj, UINT32 numEntries)
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

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			SerializedSubObject* serializableObject = static_cast<SerializedSubObject*>(obj);

			for (auto& entry : serializableObject->entries)
				mSequentialEntries.push_back(entry.second);
		}

		const String& GetRTTIName() override
		{
			static String name = "SerializedSubObject";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_SerializedSubObject;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedSubObject>();
		}

	private:
		Vector<SerializedEntry> mSequentialEntries;
	};

	class BS_UTILITY_EXPORT SerializedEntryRTTI : public RTTIType <SerializedEntry, IReflectable, SerializedEntryRTTI>
	{
	private:
		UINT32& GetFieldId(SerializedEntry* obj)
		{
			return obj->fieldId;
		}

		void SetFieldId(SerializedEntry* obj, UINT32& val)
		{
			obj->fieldId = val;
		}

		SPtr<SerializedInstance> GetSerialized(SerializedEntry* obj)
		{
			return obj->serialized;
		}

		void SetSerialized(SerializedEntry* obj, SPtr<SerializedInstance> val)
		{
			obj->serialized = val;
		}

	public:
		SerializedEntryRTTI()
		{
			addPlainField("fieldId", 0, &SerializedEntryRTTI::getFieldId, &SerializedEntryRTTI::setFieldId);
			addReflectablePtrField("serialized", 1, &SerializedEntryRTTI::getSerialized, &SerializedEntryRTTI::setSerialized);
		}

		const String& GetRTTIName() override
		{
			static String name = "SerializedEntry";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_SerializedEntry;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedEntry>();
		}
	};

	class BS_UTILITY_EXPORT SerializedArrayEntryRTTI : public RTTIType <SerializedArrayEntry, IReflectable, SerializedArrayEntryRTTI>
	{
	private:
		UINT32& GetArrayIdx(SerializedArrayEntry* obj)
		{
			return obj->index;
		}

		void SetArrayIdx(SerializedArrayEntry* obj, UINT32& val)
		{
			obj->index = val;
		}

		SPtr<SerializedInstance> GetSerialized(SerializedArrayEntry* obj)
		{
			return obj->serialized;
		}

		void SetSerialized(SerializedArrayEntry* obj, SPtr<SerializedInstance> val)
		{
			obj->serialized = val;
		}

	public:
		SerializedArrayEntryRTTI()
		{
			addPlainField("index", 0, &SerializedArrayEntryRTTI::getArrayIdx, &SerializedArrayEntryRTTI::setArrayIdx);
			addReflectablePtrField("serialized", 1, &SerializedArrayEntryRTTI::getSerialized, &SerializedArrayEntryRTTI::setSerialized);
		}

		const String& GetRTTIName() override
		{
			static String name = "SerializedArrayEntry";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_SerializedArrayEntry;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<SerializedArrayEntry>();
		}
	};

	/** @} */
	/** @endcond */
}
