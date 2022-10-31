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

	class BS_UTILITY_EXPORT SerializedInstanceRTTI : public RTTIType<SerializedInstance, IReflectable, SerializedInstanceRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "SerializedInstance";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_SerializedInstance;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return nullptr;
		}
	};

	class BS_UTILITY_EXPORT SerializedFieldRTTI : public RTTIType<SerializedField, SerializedInstance, SerializedFieldRTTI>
	{
	private:
		SPtr<DataStream> GetData(SerializedField* obj, u32& size)
		{
			size = obj->Size;

			return B3DMakeShared<MemoryDataStream>(obj->Value, obj->Size);
		}

		void SetData(SerializedField* obj, const SPtr<DataStream>& value, u32 size)
		{
			obj->Value = (u8*)B3DAllocate(size);
			obj->Size = size;
			obj->OwnsMemory = true;

			value->Read(obj->Value, size);
		}

	public:
		SerializedFieldRTTI()
		{
			AddDataBlockField("data", 0, &SerializedFieldRTTI::GetData, &SerializedFieldRTTI::SetData);
		}

		const String& GetRttiName()
		{
			static String name = "SerializedField";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_SerializedField;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<SerializedField>();
		}
	};

	class BS_UTILITY_EXPORT SerializedDataBlockRTTI : public RTTIType<SerializedDataBlock, SerializedInstance, SerializedDataBlockRTTI>
	{
	private:
		SPtr<DataStream> GetData(SerializedDataBlock* obj, u32& size)
		{
			size = obj->Size;
			obj->Stream->Seek(obj->Offset);

			return obj->Stream;
		}

		void SetData(SerializedDataBlock* obj, const SPtr<DataStream>& value, u32 size)
		{
			SPtr<MemoryDataStream> memStream = B3DMakeShared<MemoryDataStream>(size);
			value->Read(memStream->Data(), size);

			obj->Stream = memStream;
			obj->Size = size;
			obj->Offset = 0;
		}

	public:
		SerializedDataBlockRTTI()
		{
			AddDataBlockField("data", 0, &SerializedDataBlockRTTI::GetData, &SerializedDataBlockRTTI::SetData);
		}

		const String& GetRttiName()
		{
			static String name = "SerializedDataBlock";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_SerializedDataBlock;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<SerializedDataBlock>();
		}
	};

	class BS_UTILITY_EXPORT SerializedObjectRTTI : public RTTIType<SerializedObject, SerializedInstance, SerializedObjectRTTI>
	{
	private:
		SerializedSubObject& GetEntry(SerializedObject* obj, u32 arrayIdx)
		{
			return obj->SubObjects[arrayIdx];
		}

		void SetEntry(SerializedObject* obj, u32 arrayIdx, SerializedSubObject& val)
		{
			obj->SubObjects[arrayIdx] = val;
		}

		u32 GetNumEntries(SerializedObject* obj)
		{
			return (u32)obj->SubObjects.size();
		}

		void SetNumEntries(SerializedObject* obj, u32 numEntries)
		{
			obj->SubObjects = Vector<SerializedSubObject>(numEntries);
		}

	public:
		SerializedObjectRTTI()
		{
			AddReflectableArrayField("entries", 1, &SerializedObjectRTTI::GetEntry, &SerializedObjectRTTI::GetNumEntries, &SerializedObjectRTTI::SetEntry, &SerializedObjectRTTI::SetNumEntries);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializedObject";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializedObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedObject>();
		}
	};

	class BS_UTILITY_EXPORT SerializedArrayRTTI : public RTTIType<SerializedArray, SerializedInstance, SerializedArrayRTTI>
	{
	private:
		u32& GetNumElements(SerializedArray* obj)
		{
			return obj->NumElements;
		}

		void SetNumElements(SerializedArray* obj, u32& val)
		{
			obj->NumElements = val;
		}

		SerializedArrayEntry& GetEntry(SerializedArray* obj, u32 arrayIdx)
		{
			return mSequentialEntries[arrayIdx];
		}

		void SetEntry(SerializedArray* obj, u32 arrayIdx, SerializedArrayEntry& val)
		{
			obj->Entries[val.Index] = val;
		}

		u32 GetNumEntries(SerializedArray* obj)
		{
			return (u32)mSequentialEntries.size();
		}

		void SetNumEntries(SerializedArray* obj, u32 numEntries)
		{
			obj->Entries = UnorderedMap<u32, SerializedArrayEntry>();
		}

	public:
		SerializedArrayRTTI()
		{
			AddPlainField("numElements", 0, &SerializedArrayRTTI::GetNumElements, &SerializedArrayRTTI::SetNumElements);
			AddReflectableArrayField("entries", 1, &SerializedArrayRTTI::GetEntry, &SerializedArrayRTTI::GetNumEntries, &SerializedArrayRTTI::SetEntry, &SerializedArrayRTTI::SetNumEntries);
		}

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			SerializedArray* serializedArray = static_cast<SerializedArray*>(obj);

			for(auto& entry : serializedArray->Entries)
				mSequentialEntries.push_back(entry.second);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializedArray";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializedArray;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedArray>();
		}

	private:
		Vector<SerializedArrayEntry> mSequentialEntries;
	};

	class BS_UTILITY_EXPORT SerializedSubObjectRTTI : public RTTIType<SerializedSubObject, IReflectable, SerializedSubObjectRTTI>
	{
	private:
		u32& GetTypeId(SerializedSubObject* obj)
		{
			return obj->TypeId;
		}

		void SetTypeId(SerializedSubObject* obj, u32& val)
		{
			obj->TypeId = val;
		}

		SerializedEntry& GetEntry(SerializedSubObject* obj, u32 arrayIdx)
		{
			return mSequentialEntries[arrayIdx];
		}

		void SetEntry(SerializedSubObject* obj, u32 arrayIdx, SerializedEntry& val)
		{
			obj->Entries[val.FieldId] = val;
		}

		u32 GetNumEntries(SerializedSubObject* obj)
		{
			return (u32)mSequentialEntries.size();
		}

		void SetNumEntries(SerializedSubObject* obj, u32 numEntries)
		{
			obj->Entries = UnorderedMap<u32, SerializedEntry>();
		}

	public:
		SerializedSubObjectRTTI()
		{
			AddPlainField("typeId", 0, &SerializedSubObjectRTTI::GetTypeId, &SerializedSubObjectRTTI::SetTypeId);
			AddReflectableArrayField("entries", 1, &SerializedSubObjectRTTI::GetEntry, &SerializedSubObjectRTTI::GetNumEntries, &SerializedSubObjectRTTI::SetEntry, &SerializedSubObjectRTTI::SetNumEntries);
		}

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			SerializedSubObject* serializableObject = static_cast<SerializedSubObject*>(obj);

			for(auto& entry : serializableObject->Entries)
				mSequentialEntries.push_back(entry.second);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializedSubObject";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializedSubObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedSubObject>();
		}

	private:
		Vector<SerializedEntry> mSequentialEntries;
	};

	class BS_UTILITY_EXPORT SerializedEntryRTTI : public RTTIType<SerializedEntry, IReflectable, SerializedEntryRTTI>
	{
	private:
		u32& GetFieldId(SerializedEntry* obj)
		{
			return obj->FieldId;
		}

		void SetFieldId(SerializedEntry* obj, u32& val)
		{
			obj->FieldId = val;
		}

		SPtr<SerializedInstance> GetSerialized(SerializedEntry* obj)
		{
			return obj->Serialized;
		}

		void SetSerialized(SerializedEntry* obj, SPtr<SerializedInstance> val)
		{
			obj->Serialized = val;
		}

	public:
		SerializedEntryRTTI()
		{
			AddPlainField("fieldId", 0, &SerializedEntryRTTI::GetFieldId, &SerializedEntryRTTI::SetFieldId);
			AddReflectablePtrField("serialized", 1, &SerializedEntryRTTI::GetSerialized, &SerializedEntryRTTI::SetSerialized);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializedEntry";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializedEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedEntry>();
		}
	};

	class BS_UTILITY_EXPORT SerializedArrayEntryRTTI : public RTTIType<SerializedArrayEntry, IReflectable, SerializedArrayEntryRTTI>
	{
	private:
		u32& GetArrayIdx(SerializedArrayEntry* obj)
		{
			return obj->Index;
		}

		void SetArrayIdx(SerializedArrayEntry* obj, u32& val)
		{
			obj->Index = val;
		}

		SPtr<SerializedInstance> GetSerialized(SerializedArrayEntry* obj)
		{
			return obj->Serialized;
		}

		void SetSerialized(SerializedArrayEntry* obj, SPtr<SerializedInstance> val)
		{
			obj->Serialized = val;
		}

	public:
		SerializedArrayEntryRTTI()
		{
			AddPlainField("index", 0, &SerializedArrayEntryRTTI::GetArrayIdx, &SerializedArrayEntryRTTI::SetArrayIdx);
			AddReflectablePtrField("serialized", 1, &SerializedArrayEntryRTTI::GetSerialized, &SerializedArrayEntryRTTI::SetSerialized);
		}

		const String& GetRttiName() override
		{
			static String name = "SerializedArrayEntry";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_SerializedArrayEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<SerializedArrayEntry>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
