//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStdRTTI.h"
#include "Serialization/BsManagedSerializableDiff.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class BS_SCR_BE_EXPORT ModifiedFieldRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedField, IReflectable, ModifiedFieldRTTI >
	{
	private:
		SPtr<ManagedSerializableTypeInfo> GetParentType(ManagedSerializableDiff::ModifiedField* obj)
		{
			return obj->parentType;
		}

		void SetParentType(ManagedSerializableDiff::ModifiedField* obj, SPtr<ManagedSerializableTypeInfo> val)
		{
			obj->parentType = val;
		}

		SPtr<ManagedSerializableMemberInfo> GetFieldType(ManagedSerializableDiff::ModifiedField* obj)
		{
			return obj->fieldType;
		}

		void SetFieldType(ManagedSerializableDiff::ModifiedField* obj, SPtr<ManagedSerializableMemberInfo> val)
		{
			obj->fieldType = val;
		}

		SPtr<ManagedSerializableDiff::Modification> GetModification(ManagedSerializableDiff::ModifiedField* obj)
		{
			return obj->modification;
		}

		void SetModification(ManagedSerializableDiff::ModifiedField* obj, SPtr<ManagedSerializableDiff::Modification> val)
		{
			obj->modification = val;
		}
	public:
		ModifiedFieldRTTI()
		{
			AddReflectablePtrField("parentType", 0, &ModifiedFieldRTTI::GetParentType, &ModifiedFieldRTTI::SetParentType);
			AddReflectablePtrField("fieldType", 1, &ModifiedFieldRTTI::GetFieldType, &ModifiedFieldRTTI::SetFieldType);
			AddReflectablePtrField("modification", 2, &ModifiedFieldRTTI::GetModification, &ModifiedFieldRTTI::SetModification);
		}

		const String& GetRttiName() 
		{
			static String name = "ScriptModifiedField";
			return name;
		}

		UINT32 GetRttiId() 
		{
			return TID_ScriptModifiedField;
		}

		SPtr<IReflectable> NewRttiObject() 
		{
			return bs_shared_ptr_new<ManagedSerializableDiff::ModifiedField>();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedArrayEntryRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedArrayEntry, IReflectable, ModifiedArrayEntryRTTI >
	{
	private:
		UINT32& GetIdx(ManagedSerializableDiff::ModifiedArrayEntry* obj)
		{
			return obj->idx;
		}

		void SetIdx(ManagedSerializableDiff::ModifiedArrayEntry* obj, UINT32& val)
		{
			obj->idx = val;
		}

		SPtr<ManagedSerializableDiff::Modification> GetModification(ManagedSerializableDiff::ModifiedArrayEntry* obj)
		{
			return obj->modification;
		}

		void SetModification(ManagedSerializableDiff::ModifiedArrayEntry* obj, SPtr<ManagedSerializableDiff::Modification> val)
		{
			obj->modification = val;
		}
	public:
		ModifiedArrayEntryRTTI()
		{
			AddPlainField("idx", 0, &ModifiedArrayEntryRTTI::GetIdx, &ModifiedArrayEntryRTTI::SetIdx);
			AddReflectablePtrField("modification", 1, &ModifiedArrayEntryRTTI::GetModification, &ModifiedArrayEntryRTTI::SetModification);
		}

		const String& GetRttiName() 
		{
			static String name = "ScriptModifiedArrayEntry";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_ScriptModifiedArrayEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableDiff::ModifiedArrayEntry>();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedDictionaryEntryRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedDictionaryEntry, IReflectable, ModifiedDictionaryEntryRTTI >
	{
	private:
		SPtr<ManagedSerializableFieldData> GetKey(ManagedSerializableDiff::ModifiedDictionaryEntry* obj)
		{
			return obj->key;
		}

		void SetKey(ManagedSerializableDiff::ModifiedDictionaryEntry* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->key = val;
		}

		SPtr<ManagedSerializableDiff::Modification> GetModification(ManagedSerializableDiff::ModifiedDictionaryEntry* obj)
		{
			return obj->modification;
		}

		void SetModification(ManagedSerializableDiff::ModifiedDictionaryEntry* obj, SPtr<ManagedSerializableDiff::Modification> val)
		{
			obj->modification = val;
		}
	public:
		ModifiedDictionaryEntryRTTI()
		{
			AddReflectablePtrField("key", 0, &ModifiedDictionaryEntryRTTI::getKey, &ModifiedDictionaryEntryRTTI::setKey);
			AddReflectablePtrField("modification", 1, &ModifiedDictionaryEntryRTTI::getModification, &ModifiedDictionaryEntryRTTI::setModification);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptModifiedDictionaryEntry";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_ScriptModifiedDictionaryEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableDiff::ModifiedDictionaryEntry>();
		}
	};

	class BS_SCR_BE_EXPORT ModificationRTTI :
		public RTTIType < ManagedSerializableDiff::Modification, IReflectable, ModificationRTTI >
	{
	public:
		ModificationRTTI()
		{ }

		const String& GetRttiName() override
		{
			static String name = "ScriptModification";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_ScriptModification;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return nullptr;
		}
	};

	class BS_SCR_BE_EXPORT ModifiedObjectRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedObject, ManagedSerializableDiff::Modification, ModifiedObjectRTTI >
	{
	private:
		ManagedSerializableDiff::ModifiedField& GetFieldEntry(ManagedSerializableDiff::ModifiedObject* obj, UINT32 arrayIdx)
		{
			return obj->entries[arrayIdx];
		}

		void SetFieldEntry(ManagedSerializableDiff::ModifiedObject* obj, UINT32 arrayIdx, ManagedSerializableDiff::ModifiedField& val)
		{
			obj->entries[arrayIdx] = val;
		}

		UINT32 GetNumFieldEntries(ManagedSerializableDiff::ModifiedObject* obj)
		{
			return (UINT32)obj->entries.size();
		}

		void SetNumFieldEntries(ManagedSerializableDiff::ModifiedObject* obj, UINT32 numEntries)
		{
			obj->entries = Vector<ManagedSerializableDiff::ModifiedField>(numEntries);
		}

	public:
		ModifiedObjectRTTI()
		{
			addReflectableArrayField("entries", 0, &ModifiedObjectRTTI::GetFieldEntry, &ModifiedObjectRTTI::GetNumFieldEntries,
				&ModifiedObjectRTTI::SetFieldEntry, &ModifiedObjectRTTI::SetNumFieldEntries);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptModifiedObject";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_ScriptModifiedObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDiff::ModifiedObject::Create();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedArrayRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedArray, ManagedSerializableDiff::Modification, ModifiedArrayRTTI >
	{
	private:
		Vector<UINT32>& GetOrigSizes(ManagedSerializableDiff::ModifiedArray* obj)
		{
			return obj->origSizes;
		}

		void SetOrigSizes(ManagedSerializableDiff::ModifiedArray* obj, Vector<UINT32>& val)
		{
			obj->origSizes = val;
		}

		Vector<UINT32>& GetNewSizes(ManagedSerializableDiff::ModifiedArray* obj)
		{
			return obj->newSizes;
		}

		void SetNewSizes(ManagedSerializableDiff::ModifiedArray* obj, Vector<UINT32>& val)
		{
			obj->newSizes = val;
		}

		ManagedSerializableDiff::ModifiedArrayEntry& GetFieldEntry(ManagedSerializableDiff::ModifiedArray* obj, UINT32 arrayIdx)
		{
			return obj->entries[arrayIdx];
		}

		void SetFieldEntry(ManagedSerializableDiff::ModifiedArray* obj, UINT32 arrayIdx, ManagedSerializableDiff::ModifiedArrayEntry& val)
		{
			obj->entries[arrayIdx] = val;
		}

		UINT32 GetNumFieldEntries(ManagedSerializableDiff::ModifiedArray* obj)
		{
			return (UINT32)obj->entries.size();
		}

		void SetNumFieldEntries(ManagedSerializableDiff::ModifiedArray* obj, UINT32 numEntries)
		{
			obj->entries = Vector<ManagedSerializableDiff::ModifiedArrayEntry>(numEntries);
		}

	public:
		ModifiedArrayRTTI()
		{
			AddPlainField("origSizes", 0, &ModifiedArrayRTTI::getOrigSizes, &ModifiedArrayRTTI::setOrigSizes);
			AddPlainField("newSizes", 1, &ModifiedArrayRTTI::getNewSizes, &ModifiedArrayRTTI::setNewSizes);
			addReflectableArrayField("entries", 2, &ModifiedArrayRTTI::GetFieldEntry, &ModifiedArrayRTTI::GetNumFieldEntries,
				&ModifiedArrayRTTI::SetFieldEntry, &ModifiedArrayRTTI::SetNumFieldEntries);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptModifiedArray";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_ScriptModifiedArray;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDiff::ModifiedArray::Create();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedDictionaryRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedDictionary, ManagedSerializableDiff::Modification, ModifiedDictionaryRTTI >
	{
	private:
		SPtr<ManagedSerializableFieldData> GetRemovedEntry(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 arrayIdx)
		{
			return obj->removed[arrayIdx];
		}

		void SetRemovedEntry(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 arrayIdx, SPtr<ManagedSerializableFieldData> val)
		{
			obj->removed[arrayIdx] = val;
		}

		UINT32 GetNumRemovedEntries(ManagedSerializableDiff::ModifiedDictionary* obj)
		{
			return (UINT32)obj->removed.size();
		}

		void SetNumRemovedEntries(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 numEntries)
		{
			obj->removed = Vector<SPtr<ManagedSerializableFieldData>>(numEntries);
		}

		ManagedSerializableDiff::ModifiedDictionaryEntry& GetFieldEntry(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 arrayIdx)
		{
			return obj->entries[arrayIdx];
		}

		void SetFieldEntry(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 arrayIdx, ManagedSerializableDiff::ModifiedDictionaryEntry& val)
		{
			obj->entries[arrayIdx] = val;
		}

		UINT32 GetNumFieldEntries(ManagedSerializableDiff::ModifiedDictionary* obj)
		{
			return (UINT32)obj->entries.size();
		}

		void SetNumFieldEntries(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 numEntries)
		{
			obj->entries = Vector<ManagedSerializableDiff::ModifiedDictionaryEntry>(numEntries);
		}

	public:
		ModifiedDictionaryRTTI()
		{
			addReflectablePtrArrayField("removed", 0, &ModifiedDictionaryRTTI::getRemovedEntry, &ModifiedDictionaryRTTI::getNumRemovedEntries,
				&ModifiedDictionaryRTTI::setRemovedEntry, &ModifiedDictionaryRTTI::setNumRemovedEntries);
			addReflectableArrayField("entries", 1, &ModifiedDictionaryRTTI::GetFieldEntry, &ModifiedDictionaryRTTI::GetNumFieldEntries,
				&ModifiedDictionaryRTTI::SetFieldEntry, &ModifiedDictionaryRTTI::SetNumFieldEntries);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptModifiedDictionary";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_ScriptModifiedDictionary;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDiff::ModifiedDictionary::Create();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedEntryRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedEntry, ManagedSerializableDiff::Modification, ModifiedEntryRTTI >
	{
	private:
		SPtr<ManagedSerializableFieldData> GetValue(ManagedSerializableDiff::ModifiedEntry* obj)
		{
			return obj->value;
		}

		void SetValue(ManagedSerializableDiff::ModifiedEntry* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->value = val;
		}

	public:
		ModifiedEntryRTTI()
		{
			AddReflectablePtrField("value", 0, &ModifiedEntryRTTI::getValue, &ModifiedEntryRTTI::setValue);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptModifiedEntry";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_ScriptModifiedEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDiff::ModifiedEntry::Create(nullptr);
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableDiffRTTI : public RTTIType <ManagedSerializableDiff, IReflectable, ManagedSerializableDiffRTTI>
	{
	private:
		SPtr<ManagedSerializableDiff::ModifiedObject> GetModificationRoot(ManagedSerializableDiff* obj)
		{
			return obj->mModificationRoot;
		}

		void SetModificationRoot(ManagedSerializableDiff* obj, SPtr<ManagedSerializableDiff::ModifiedObject> val)
		{
			obj->mModificationRoot = val;
		}

	public:
		ManagedSerializableDiffRTTI()
		{
			AddReflectablePtrField("mModificationRoot", 0, &ManagedSerializableDiffRTTI::getModificationRoot,
				&ManagedSerializableDiffRTTI::setModificationRoot);

		}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableDiff";
			return name;
		}

		UINT32 GetRttiId() override
		{
			return TID_ScriptSerializableDiff;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableDiff>();
		}
	};

	/** @} */
	/** @endcond */
}
