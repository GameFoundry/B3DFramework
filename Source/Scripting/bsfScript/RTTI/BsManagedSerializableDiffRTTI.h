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

	class BS_SCR_BE_EXPORT ModifiedFieldRTTI : public RTTIType<ManagedSerializableDiff::ModifiedField, IReflectable, ModifiedFieldRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfo> GetParentType(ManagedSerializableDiff::ModifiedField* obj)
		{
			return obj->ParentType;
		}

		void SetParentType(ManagedSerializableDiff::ModifiedField* obj, SPtr<ManagedSerializableTypeInfo> val)
		{
			obj->ParentType = val;
		}

		SPtr<ManagedSerializableMemberInfo> GetFieldType(ManagedSerializableDiff::ModifiedField* obj)
		{
			return obj->FieldType;
		}

		void SetFieldType(ManagedSerializableDiff::ModifiedField* obj, SPtr<ManagedSerializableMemberInfo> val)
		{
			obj->FieldType = val;
		}

		SPtr<ManagedSerializableDiff::Modification> GetModification(ManagedSerializableDiff::ModifiedField* obj)
		{
			return obj->Modification;
		}

		void SetModification(ManagedSerializableDiff::ModifiedField* obj, SPtr<ManagedSerializableDiff::Modification> val)
		{
			obj->Modification = val;
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

		u32 GetRttiId()
		{
			return TID_ScriptModifiedField;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedSerializableDiff::ModifiedField>();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedArrayEntryRTTI : public RTTIType<ManagedSerializableDiff::ModifiedArrayEntry, IReflectable, ModifiedArrayEntryRTTI>
	{
	private:
		u32& GetIdx(ManagedSerializableDiff::ModifiedArrayEntry* obj)
		{
			return obj->Idx;
		}

		void SetIdx(ManagedSerializableDiff::ModifiedArrayEntry* obj, u32& val)
		{
			obj->Idx = val;
		}

		SPtr<ManagedSerializableDiff::Modification> GetModification(ManagedSerializableDiff::ModifiedArrayEntry* obj)
		{
			return obj->Modification;
		}

		void SetModification(ManagedSerializableDiff::ModifiedArrayEntry* obj, SPtr<ManagedSerializableDiff::Modification> val)
		{
			obj->Modification = val;
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

		u32 GetRttiId() override
		{
			return TID_ScriptModifiedArrayEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableDiff::ModifiedArrayEntry>();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedDictionaryEntryRTTI : public RTTIType<ManagedSerializableDiff::ModifiedDictionaryEntry, IReflectable, ModifiedDictionaryEntryRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldData> GetKey(ManagedSerializableDiff::ModifiedDictionaryEntry* obj)
		{
			return obj->Key;
		}

		void SetKey(ManagedSerializableDiff::ModifiedDictionaryEntry* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->Key = val;
		}

		SPtr<ManagedSerializableDiff::Modification> GetModification(ManagedSerializableDiff::ModifiedDictionaryEntry* obj)
		{
			return obj->Modification;
		}

		void SetModification(ManagedSerializableDiff::ModifiedDictionaryEntry* obj, SPtr<ManagedSerializableDiff::Modification> val)
		{
			obj->Modification = val;
		}

	public:
		ModifiedDictionaryEntryRTTI()
		{
			AddReflectablePtrField("key", 0, &ModifiedDictionaryEntryRTTI::GetKey, &ModifiedDictionaryEntryRTTI::SetKey);
			AddReflectablePtrField("modification", 1, &ModifiedDictionaryEntryRTTI::GetModification, &ModifiedDictionaryEntryRTTI::SetModification);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptModifiedDictionaryEntry";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ScriptModifiedDictionaryEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableDiff::ModifiedDictionaryEntry>();
		}
	};

	class BS_SCR_BE_EXPORT ModificationRTTI : public RTTIType<ManagedSerializableDiff::Modification, IReflectable, ModificationRTTI>
	{
	public:
		ModificationRTTI()
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptModification";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ScriptModification;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return nullptr;
		}
	};

	class BS_SCR_BE_EXPORT ModifiedObjectRTTI : public RTTIType<ManagedSerializableDiff::ModifiedObject, ManagedSerializableDiff::Modification, ModifiedObjectRTTI>
	{
	private:
		ManagedSerializableDiff::ModifiedField& GetFieldEntry(ManagedSerializableDiff::ModifiedObject* obj, u32 arrayIdx)
		{
			return obj->Entries[arrayIdx];
		}

		void SetFieldEntry(ManagedSerializableDiff::ModifiedObject* obj, u32 arrayIdx, ManagedSerializableDiff::ModifiedField& val)
		{
			obj->Entries[arrayIdx] = val;
		}

		u32 GetNumFieldEntries(ManagedSerializableDiff::ModifiedObject* obj)
		{
			return (u32)obj->Entries.size();
		}

		void SetNumFieldEntries(ManagedSerializableDiff::ModifiedObject* obj, u32 numEntries)
		{
			obj->Entries = Vector<ManagedSerializableDiff::ModifiedField>(numEntries);
		}

	public:
		ModifiedObjectRTTI()
		{
			AddReflectableArrayField("entries", 0, &ModifiedObjectRTTI::GetFieldEntry, &ModifiedObjectRTTI::GetNumFieldEntries, &ModifiedObjectRTTI::SetFieldEntry, &ModifiedObjectRTTI::SetNumFieldEntries);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptModifiedObject";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ScriptModifiedObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDiff::ModifiedObject::Create();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedArrayRTTI : public RTTIType<ManagedSerializableDiff::ModifiedArray, ManagedSerializableDiff::Modification, ModifiedArrayRTTI>
	{
	private:
		Vector<u32>& GetOrigSizes(ManagedSerializableDiff::ModifiedArray* obj)
		{
			return obj->OrigSizes;
		}

		void SetOrigSizes(ManagedSerializableDiff::ModifiedArray* obj, Vector<u32>& val)
		{
			obj->OrigSizes = val;
		}

		Vector<u32>& GetNewSizes(ManagedSerializableDiff::ModifiedArray* obj)
		{
			return obj->NewSizes;
		}

		void SetNewSizes(ManagedSerializableDiff::ModifiedArray* obj, Vector<u32>& val)
		{
			obj->NewSizes = val;
		}

		ManagedSerializableDiff::ModifiedArrayEntry& GetFieldEntry(ManagedSerializableDiff::ModifiedArray* obj, u32 arrayIdx)
		{
			return obj->Entries[arrayIdx];
		}

		void SetFieldEntry(ManagedSerializableDiff::ModifiedArray* obj, u32 arrayIdx, ManagedSerializableDiff::ModifiedArrayEntry& val)
		{
			obj->Entries[arrayIdx] = val;
		}

		u32 GetNumFieldEntries(ManagedSerializableDiff::ModifiedArray* obj)
		{
			return (u32)obj->Entries.size();
		}

		void SetNumFieldEntries(ManagedSerializableDiff::ModifiedArray* obj, u32 numEntries)
		{
			obj->Entries = Vector<ManagedSerializableDiff::ModifiedArrayEntry>(numEntries);
		}

	public:
		ModifiedArrayRTTI()
		{
			AddPlainField("origSizes", 0, &ModifiedArrayRTTI::GetOrigSizes, &ModifiedArrayRTTI::SetOrigSizes);
			AddPlainField("newSizes", 1, &ModifiedArrayRTTI::GetNewSizes, &ModifiedArrayRTTI::SetNewSizes);
			AddReflectableArrayField("entries", 2, &ModifiedArrayRTTI::GetFieldEntry, &ModifiedArrayRTTI::GetNumFieldEntries, &ModifiedArrayRTTI::SetFieldEntry, &ModifiedArrayRTTI::SetNumFieldEntries);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptModifiedArray";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ScriptModifiedArray;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDiff::ModifiedArray::Create();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedDictionaryRTTI : public RTTIType<ManagedSerializableDiff::ModifiedDictionary, ManagedSerializableDiff::Modification, ModifiedDictionaryRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldData> GetRemovedEntry(ManagedSerializableDiff::ModifiedDictionary* obj, u32 arrayIdx)
		{
			return obj->Removed[arrayIdx];
		}

		void SetRemovedEntry(ManagedSerializableDiff::ModifiedDictionary* obj, u32 arrayIdx, SPtr<ManagedSerializableFieldData> val)
		{
			obj->Removed[arrayIdx] = val;
		}

		u32 GetNumRemovedEntries(ManagedSerializableDiff::ModifiedDictionary* obj)
		{
			return (u32)obj->Removed.size();
		}

		void SetNumRemovedEntries(ManagedSerializableDiff::ModifiedDictionary* obj, u32 numEntries)
		{
			obj->Removed = Vector<SPtr<ManagedSerializableFieldData>>(numEntries);
		}

		ManagedSerializableDiff::ModifiedDictionaryEntry& GetFieldEntry(ManagedSerializableDiff::ModifiedDictionary* obj, u32 arrayIdx)
		{
			return obj->Entries[arrayIdx];
		}

		void SetFieldEntry(ManagedSerializableDiff::ModifiedDictionary* obj, u32 arrayIdx, ManagedSerializableDiff::ModifiedDictionaryEntry& val)
		{
			obj->Entries[arrayIdx] = val;
		}

		u32 GetNumFieldEntries(ManagedSerializableDiff::ModifiedDictionary* obj)
		{
			return (u32)obj->Entries.size();
		}

		void SetNumFieldEntries(ManagedSerializableDiff::ModifiedDictionary* obj, u32 numEntries)
		{
			obj->Entries = Vector<ManagedSerializableDiff::ModifiedDictionaryEntry>(numEntries);
		}

	public:
		ModifiedDictionaryRTTI()
		{
			AddReflectablePtrArrayField("removed", 0, &ModifiedDictionaryRTTI::GetRemovedEntry, &ModifiedDictionaryRTTI::GetNumRemovedEntries, &ModifiedDictionaryRTTI::SetRemovedEntry, &ModifiedDictionaryRTTI::SetNumRemovedEntries);
			AddReflectableArrayField("entries", 1, &ModifiedDictionaryRTTI::GetFieldEntry, &ModifiedDictionaryRTTI::GetNumFieldEntries, &ModifiedDictionaryRTTI::SetFieldEntry, &ModifiedDictionaryRTTI::SetNumFieldEntries);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptModifiedDictionary";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ScriptModifiedDictionary;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDiff::ModifiedDictionary::Create();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedEntryRTTI : public RTTIType<ManagedSerializableDiff::ModifiedEntry, ManagedSerializableDiff::Modification, ModifiedEntryRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldData> GetValue(ManagedSerializableDiff::ModifiedEntry* obj)
		{
			return obj->Value;
		}

		void SetValue(ManagedSerializableDiff::ModifiedEntry* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->Value = val;
		}

	public:
		ModifiedEntryRTTI()
		{
			AddReflectablePtrField("value", 0, &ModifiedEntryRTTI::GetValue, &ModifiedEntryRTTI::SetValue);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptModifiedEntry";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ScriptModifiedEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDiff::ModifiedEntry::Create(nullptr);
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableDiffRTTI : public RTTIType<ManagedSerializableDiff, IReflectable, ManagedSerializableDiffRTTI>
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
			AddReflectablePtrField("mModificationRoot", 0, &ManagedSerializableDiffRTTI::GetModificationRoot, &ManagedSerializableDiffRTTI::SetModificationRoot);
		}

		const String& GetRttiName() override
		{
			static String name = "ScriptSerializableDiff";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ScriptSerializableDiff;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableDiff>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
