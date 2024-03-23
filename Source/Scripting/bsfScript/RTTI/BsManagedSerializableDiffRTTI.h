//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStdRTTI.h"
#include "Serialization/BsManagedSerializableDelta.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedFieldRTTI : public RTTIType<ManagedSerializableDelta::ModifiedField, IReflectable, ModifiedFieldRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfo> GetParentType(ManagedSerializableDelta::ModifiedField* obj)
		{
			return obj->ParentType;
		}

		void SetParentType(ManagedSerializableDelta::ModifiedField* obj, SPtr<ManagedSerializableTypeInfo> val)
		{
			obj->ParentType = val;
		}

		SPtr<ManagedSerializableMemberInfo> GetFieldType(ManagedSerializableDelta::ModifiedField* obj)
		{
			return obj->FieldType;
		}

		void SetFieldType(ManagedSerializableDelta::ModifiedField* obj, SPtr<ManagedSerializableMemberInfo> val)
		{
			obj->FieldType = val;
		}

		SPtr<ManagedSerializableDelta::Modification> GetModification(ManagedSerializableDelta::ModifiedField* obj)
		{
			return obj->Modification;
		}

		void SetModification(ManagedSerializableDelta::ModifiedField* obj, SPtr<ManagedSerializableDelta::Modification> val)
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

		u32 GetRttiId() const override
		{
			return TID_ScriptModifiedField;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedSerializableDelta::ModifiedField>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedArrayEntryRTTI : public RTTIType<ManagedSerializableDelta::ModifiedArrayEntry, IReflectable, ModifiedArrayEntryRTTI>
	{
	private:
		u32& GetIdx(ManagedSerializableDelta::ModifiedArrayEntry* obj)
		{
			return obj->Idx;
		}

		void SetIdx(ManagedSerializableDelta::ModifiedArrayEntry* obj, u32& val)
		{
			obj->Idx = val;
		}

		SPtr<ManagedSerializableDelta::Modification> GetModification(ManagedSerializableDelta::ModifiedArrayEntry* obj)
		{
			return obj->Modification;
		}

		void SetModification(ManagedSerializableDelta::ModifiedArrayEntry* obj, SPtr<ManagedSerializableDelta::Modification> val)
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

		u32 GetRttiId() const override
		{
			return TID_ScriptModifiedArrayEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableDelta::ModifiedArrayEntry>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedDictionaryEntryRTTI : public RTTIType<ManagedSerializableDelta::ModifiedDictionaryEntry, IReflectable, ModifiedDictionaryEntryRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldData> GetKey(ManagedSerializableDelta::ModifiedDictionaryEntry* obj)
		{
			return obj->Key;
		}

		void SetKey(ManagedSerializableDelta::ModifiedDictionaryEntry* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->Key = val;
		}

		SPtr<ManagedSerializableDelta::Modification> GetModification(ManagedSerializableDelta::ModifiedDictionaryEntry* obj)
		{
			return obj->Modification;
		}

		void SetModification(ManagedSerializableDelta::ModifiedDictionaryEntry* obj, SPtr<ManagedSerializableDelta::Modification> val)
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

		u32 GetRttiId() const override
		{
			return TID_ScriptModifiedDictionaryEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableDelta::ModifiedDictionaryEntry>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ModificationRTTI : public RTTIType<ManagedSerializableDelta::Modification, IReflectable, ModificationRTTI>
	{
	public:
		ModificationRTTI()
		{}

		const String& GetRttiName() override
		{
			static String name = "ScriptModification";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScriptModification;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return nullptr;
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedObjectRTTI : public RTTIType<ManagedSerializableDelta::ModifiedObject, ManagedSerializableDelta::Modification, ModifiedObjectRTTI>
	{
	private:
		ManagedSerializableDelta::ModifiedField& GetFieldEntry(ManagedSerializableDelta::ModifiedObject* obj, u32 arrayIdx)
		{
			return obj->Entries[arrayIdx];
		}

		void SetFieldEntry(ManagedSerializableDelta::ModifiedObject* obj, u32 arrayIdx, ManagedSerializableDelta::ModifiedField& val)
		{
			obj->Entries[arrayIdx] = val;
		}

		u32 GetNumFieldEntries(ManagedSerializableDelta::ModifiedObject* obj)
		{
			return (u32)obj->Entries.size();
		}

		void SetNumFieldEntries(ManagedSerializableDelta::ModifiedObject* obj, u32 numEntries)
		{
			obj->Entries = Vector<ManagedSerializableDelta::ModifiedField>(numEntries);
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

		u32 GetRttiId() const override
		{
			return TID_ScriptModifiedObject;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDelta::ModifiedObject::Create();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedArrayRTTI : public RTTIType<ManagedSerializableDelta::ModifiedArray, ManagedSerializableDelta::Modification, ModifiedArrayRTTI>
	{
	private:
		Vector<u32>& GetOrigSizes(ManagedSerializableDelta::ModifiedArray* obj)
		{
			return obj->OrigSizes;
		}

		void SetOrigSizes(ManagedSerializableDelta::ModifiedArray* obj, Vector<u32>& val)
		{
			obj->OrigSizes = val;
		}

		Vector<u32>& GetNewSizes(ManagedSerializableDelta::ModifiedArray* obj)
		{
			return obj->NewSizes;
		}

		void SetNewSizes(ManagedSerializableDelta::ModifiedArray* obj, Vector<u32>& val)
		{
			obj->NewSizes = val;
		}

		ManagedSerializableDelta::ModifiedArrayEntry& GetFieldEntry(ManagedSerializableDelta::ModifiedArray* obj, u32 arrayIdx)
		{
			return obj->Entries[arrayIdx];
		}

		void SetFieldEntry(ManagedSerializableDelta::ModifiedArray* obj, u32 arrayIdx, ManagedSerializableDelta::ModifiedArrayEntry& val)
		{
			obj->Entries[arrayIdx] = val;
		}

		u32 GetNumFieldEntries(ManagedSerializableDelta::ModifiedArray* obj)
		{
			return (u32)obj->Entries.size();
		}

		void SetNumFieldEntries(ManagedSerializableDelta::ModifiedArray* obj, u32 numEntries)
		{
			obj->Entries = Vector<ManagedSerializableDelta::ModifiedArrayEntry>(numEntries);
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

		u32 GetRttiId() const override
		{
			return TID_ScriptModifiedArray;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDelta::ModifiedArray::Create();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedDictionaryRTTI : public RTTIType<ManagedSerializableDelta::ModifiedDictionary, ManagedSerializableDelta::Modification, ModifiedDictionaryRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldData> GetRemovedEntry(ManagedSerializableDelta::ModifiedDictionary* obj, u32 arrayIdx)
		{
			return obj->Removed[arrayIdx];
		}

		void SetRemovedEntry(ManagedSerializableDelta::ModifiedDictionary* obj, u32 arrayIdx, SPtr<ManagedSerializableFieldData> val)
		{
			obj->Removed[arrayIdx] = val;
		}

		u32 GetNumRemovedEntries(ManagedSerializableDelta::ModifiedDictionary* obj)
		{
			return (u32)obj->Removed.size();
		}

		void SetNumRemovedEntries(ManagedSerializableDelta::ModifiedDictionary* obj, u32 numEntries)
		{
			obj->Removed = Vector<SPtr<ManagedSerializableFieldData>>(numEntries);
		}

		ManagedSerializableDelta::ModifiedDictionaryEntry& GetFieldEntry(ManagedSerializableDelta::ModifiedDictionary* obj, u32 arrayIdx)
		{
			return obj->Entries[arrayIdx];
		}

		void SetFieldEntry(ManagedSerializableDelta::ModifiedDictionary* obj, u32 arrayIdx, ManagedSerializableDelta::ModifiedDictionaryEntry& val)
		{
			obj->Entries[arrayIdx] = val;
		}

		u32 GetNumFieldEntries(ManagedSerializableDelta::ModifiedDictionary* obj)
		{
			return (u32)obj->Entries.size();
		}

		void SetNumFieldEntries(ManagedSerializableDelta::ModifiedDictionary* obj, u32 numEntries)
		{
			obj->Entries = Vector<ManagedSerializableDelta::ModifiedDictionaryEntry>(numEntries);
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

		u32 GetRttiId() const override
		{
			return TID_ScriptModifiedDictionary;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDelta::ModifiedDictionary::Create();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedEntryRTTI : public RTTIType<ManagedSerializableDelta::ModifiedEntry, ManagedSerializableDelta::Modification, ModifiedEntryRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldData> GetValue(ManagedSerializableDelta::ModifiedEntry* obj)
		{
			return obj->Value;
		}

		void SetValue(ManagedSerializableDelta::ModifiedEntry* obj, SPtr<ManagedSerializableFieldData> val)
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

		u32 GetRttiId() const override
		{
			return TID_ScriptModifiedEntry;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return ManagedSerializableDelta::ModifiedEntry::Create(nullptr);
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableDeltaRTTI : public RTTIType<ManagedSerializableDelta, IReflectable, ManagedSerializableDeltaRTTI>
	{
	private:
		SPtr<ManagedSerializableDelta::ModifiedObject> GetModificationRoot(ManagedSerializableDelta* obj)
		{
			return obj->mModificationRoot;
		}

		void SetModificationRoot(ManagedSerializableDelta* obj, SPtr<ManagedSerializableDelta::ModifiedObject> val)
		{
			obj->mModificationRoot = val;
		}

	public:
		ManagedSerializableDeltaRTTI()
		{
			AddReflectablePtrField("mModificationRoot", 0, &ManagedSerializableDeltaRTTI::GetModificationRoot, &ManagedSerializableDeltaRTTI::SetModificationRoot);
		}

		const String& GetRttiName() override
		{
			static String name = "ManagedSerializableDelta";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ManagedSerializableDelta;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ManagedSerializableDelta>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
