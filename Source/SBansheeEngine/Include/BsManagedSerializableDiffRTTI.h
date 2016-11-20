//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsRTTIType.h"
#include "BsManagedSerializableDiff.h"

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
		SPtr<ManagedSerializableTypeInfo> getParentType(ManagedSerializableDiff::ModifiedField* obj)
		{
			return obj->parentType;
		}

		void setParentType(ManagedSerializableDiff::ModifiedField* obj, SPtr<ManagedSerializableTypeInfo> val)
		{
			obj->parentType = val;
		}

		SPtr<ManagedSerializableMemberInfo> getFieldType(ManagedSerializableDiff::ModifiedField* obj)
		{
			return obj->fieldType;
		}

		void setFieldType(ManagedSerializableDiff::ModifiedField* obj, SPtr<ManagedSerializableMemberInfo> val)
		{
			obj->fieldType = val;
		}

		SPtr<ManagedSerializableDiff::Modification> getModification(ManagedSerializableDiff::ModifiedField* obj)
		{
			return obj->modification;
		}

		void setModification(ManagedSerializableDiff::ModifiedField* obj, SPtr<ManagedSerializableDiff::Modification> val)
		{
			obj->modification = val;
		}
	public:
		ModifiedFieldRTTI()
		{
			addReflectablePtrField("parentType", 0, &ModifiedFieldRTTI::getParentType, &ModifiedFieldRTTI::setParentType);
			addReflectablePtrField("fieldType", 1, &ModifiedFieldRTTI::getFieldType, &ModifiedFieldRTTI::setFieldType);
			addReflectablePtrField("modification", 2, &ModifiedFieldRTTI::getModification, &ModifiedFieldRTTI::setModification);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptModifiedField";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptModifiedField;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableDiff::ModifiedField>();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedArrayEntryRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedArrayEntry, IReflectable, ModifiedArrayEntryRTTI >
	{
	private:
		UINT32& getIdx(ManagedSerializableDiff::ModifiedArrayEntry* obj)
		{
			return obj->idx;
		}

		void setIdx(ManagedSerializableDiff::ModifiedArrayEntry* obj, UINT32& val)
		{
			obj->idx = val;
		}

		SPtr<ManagedSerializableDiff::Modification> getModification(ManagedSerializableDiff::ModifiedArrayEntry* obj)
		{
			return obj->modification;
		}

		void setModification(ManagedSerializableDiff::ModifiedArrayEntry* obj, SPtr<ManagedSerializableDiff::Modification> val)
		{
			obj->modification = val;
		}
	public:
		ModifiedArrayEntryRTTI()
		{
			addPlainField("idx", 0, &ModifiedArrayEntryRTTI::getIdx, &ModifiedArrayEntryRTTI::setIdx);
			addReflectablePtrField("modification", 1, &ModifiedArrayEntryRTTI::getModification, &ModifiedArrayEntryRTTI::setModification);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptModifiedArrayEntry";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptModifiedArrayEntry;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableDiff::ModifiedArrayEntry>();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedDictionaryEntryRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedDictionaryEntry, IReflectable, ModifiedDictionaryEntryRTTI >
	{
	private:
		SPtr<ManagedSerializableFieldData> getKey(ManagedSerializableDiff::ModifiedDictionaryEntry* obj)
		{
			return obj->key;
		}

		void setKey(ManagedSerializableDiff::ModifiedDictionaryEntry* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->key = val;
		}

		SPtr<ManagedSerializableDiff::Modification> getModification(ManagedSerializableDiff::ModifiedDictionaryEntry* obj)
		{
			return obj->modification;
		}

		void setModification(ManagedSerializableDiff::ModifiedDictionaryEntry* obj, SPtr<ManagedSerializableDiff::Modification> val)
		{
			obj->modification = val;
		}
	public:
		ModifiedDictionaryEntryRTTI()
		{
			addReflectablePtrField("key", 0, &ModifiedDictionaryEntryRTTI::getKey, &ModifiedDictionaryEntryRTTI::setKey);
			addReflectablePtrField("modification", 1, &ModifiedDictionaryEntryRTTI::getModification, &ModifiedDictionaryEntryRTTI::setModification);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptModifiedDictionaryEntry";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptModifiedDictionaryEntry;
		}

		SPtr<IReflectable> newRTTIObject() override
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

		const String& getRTTIName() override
		{
			static String name = "ScriptModification";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptModification;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return nullptr;
		}
	};

	class BS_SCR_BE_EXPORT ModifiedObjectRTTI : 
		public RTTIType < ManagedSerializableDiff::ModifiedObject, ManagedSerializableDiff::Modification, ModifiedObjectRTTI >
	{
	private:
		ManagedSerializableDiff::ModifiedField& getFieldEntry(ManagedSerializableDiff::ModifiedObject* obj, UINT32 arrayIdx)
		{
			return obj->entries[arrayIdx];
		}

		void setFieldEntry(ManagedSerializableDiff::ModifiedObject* obj, UINT32 arrayIdx, ManagedSerializableDiff::ModifiedField& val)
		{
			obj->entries[arrayIdx] = val;
		}

		UINT32 getNumFieldEntries(ManagedSerializableDiff::ModifiedObject* obj)
		{
			return (UINT32)obj->entries.size();
		}

		void setNumFieldEntries(ManagedSerializableDiff::ModifiedObject* obj, UINT32 numEntries)
		{
			obj->entries = Vector<ManagedSerializableDiff::ModifiedField>(numEntries);
		}

	public:
		ModifiedObjectRTTI()
		{
			addReflectableArrayField("entries", 0, &ModifiedObjectRTTI::getFieldEntry, &ModifiedObjectRTTI::getNumFieldEntries,
				&ModifiedObjectRTTI::setFieldEntry, &ModifiedObjectRTTI::setNumFieldEntries);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptModifiedObject";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptModifiedObject;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ManagedSerializableDiff::ModifiedObject::create();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedArrayRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedArray, ManagedSerializableDiff::Modification, ModifiedArrayRTTI >
	{
	private:
		Vector<UINT32>& getOrigSizes(ManagedSerializableDiff::ModifiedArray* obj)
		{
			return obj->origSizes;
		}

		void setOrigSizes(ManagedSerializableDiff::ModifiedArray* obj, Vector<UINT32>& val)
		{
			obj->origSizes = val;
		}

		Vector<UINT32>& getNewSizes(ManagedSerializableDiff::ModifiedArray* obj)
		{
			return obj->newSizes;
		}

		void setNewSizes(ManagedSerializableDiff::ModifiedArray* obj, Vector<UINT32>& val)
		{
			obj->newSizes = val;
		}

		ManagedSerializableDiff::ModifiedArrayEntry& getFieldEntry(ManagedSerializableDiff::ModifiedArray* obj, UINT32 arrayIdx)
		{
			return obj->entries[arrayIdx];
		}

		void setFieldEntry(ManagedSerializableDiff::ModifiedArray* obj, UINT32 arrayIdx, ManagedSerializableDiff::ModifiedArrayEntry& val)
		{
			obj->entries[arrayIdx] = val;
		}

		UINT32 getNumFieldEntries(ManagedSerializableDiff::ModifiedArray* obj)
		{
			return (UINT32)obj->entries.size();
		}

		void setNumFieldEntries(ManagedSerializableDiff::ModifiedArray* obj, UINT32 numEntries)
		{
			obj->entries = Vector<ManagedSerializableDiff::ModifiedArrayEntry>(numEntries);
		}

	public:
		ModifiedArrayRTTI()
		{
			addPlainField("origSizes", 0, &ModifiedArrayRTTI::getOrigSizes, &ModifiedArrayRTTI::setOrigSizes);
			addPlainField("newSizes", 1, &ModifiedArrayRTTI::getNewSizes, &ModifiedArrayRTTI::setNewSizes);
			addReflectableArrayField("entries", 2, &ModifiedArrayRTTI::getFieldEntry, &ModifiedArrayRTTI::getNumFieldEntries,
				&ModifiedArrayRTTI::setFieldEntry, &ModifiedArrayRTTI::setNumFieldEntries);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptModifiedArray";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptModifiedArray;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ManagedSerializableDiff::ModifiedArray::create();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedDictionaryRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedDictionary, ManagedSerializableDiff::Modification, ModifiedDictionaryRTTI >
	{
	private:
		SPtr<ManagedSerializableFieldData> getRemovedEntry(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 arrayIdx)
		{
			return obj->removed[arrayIdx];
		}

		void setRemovedEntry(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 arrayIdx, SPtr<ManagedSerializableFieldData> val)
		{
			obj->removed[arrayIdx] = val;
		}

		UINT32 getNumRemovedEntries(ManagedSerializableDiff::ModifiedDictionary* obj)
		{
			return (UINT32)obj->removed.size();
		}

		void setNumRemovedEntries(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 numEntries)
		{
			obj->removed = Vector<SPtr<ManagedSerializableFieldData>>(numEntries);
		}

		ManagedSerializableDiff::ModifiedDictionaryEntry& getFieldEntry(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 arrayIdx)
		{
			return obj->entries[arrayIdx];
		}

		void setFieldEntry(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 arrayIdx, ManagedSerializableDiff::ModifiedDictionaryEntry& val)
		{
			obj->entries[arrayIdx] = val;
		}

		UINT32 getNumFieldEntries(ManagedSerializableDiff::ModifiedDictionary* obj)
		{
			return (UINT32)obj->entries.size();
		}

		void setNumFieldEntries(ManagedSerializableDiff::ModifiedDictionary* obj, UINT32 numEntries)
		{
			obj->entries = Vector<ManagedSerializableDiff::ModifiedDictionaryEntry>(numEntries);
		}

	public:
		ModifiedDictionaryRTTI()
		{
			addReflectablePtrArrayField("removed", 0, &ModifiedDictionaryRTTI::getRemovedEntry, &ModifiedDictionaryRTTI::getNumRemovedEntries,
				&ModifiedDictionaryRTTI::setRemovedEntry, &ModifiedDictionaryRTTI::setNumRemovedEntries);
			addReflectableArrayField("entries", 1, &ModifiedDictionaryRTTI::getFieldEntry, &ModifiedDictionaryRTTI::getNumFieldEntries,
				&ModifiedDictionaryRTTI::setFieldEntry, &ModifiedDictionaryRTTI::setNumFieldEntries);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptModifiedDictionary";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptModifiedDictionary;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ManagedSerializableDiff::ModifiedDictionary::create();
		}
	};

	class BS_SCR_BE_EXPORT ModifiedEntryRTTI :
		public RTTIType < ManagedSerializableDiff::ModifiedEntry, ManagedSerializableDiff::Modification, ModifiedEntryRTTI >
	{
	private:
		SPtr<ManagedSerializableFieldData> getValue(ManagedSerializableDiff::ModifiedEntry* obj)
		{
			return obj->value;
		}

		void setValue(ManagedSerializableDiff::ModifiedEntry* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->value = val;
		}

	public:
		ModifiedEntryRTTI()
		{
			addReflectablePtrField("value", 0, &ModifiedEntryRTTI::getValue, &ModifiedEntryRTTI::setValue);
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptModifiedEntry";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptModifiedEntry;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ManagedSerializableDiff::ModifiedEntry::create(nullptr);
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableDiffRTTI : public RTTIType <ManagedSerializableDiff, IReflectable, ManagedSerializableDiffRTTI>
	{
	private:
		SPtr<ManagedSerializableDiff::ModifiedObject> getModificationRoot(ManagedSerializableDiff* obj)
		{
			return obj->mModificationRoot;
		}

		void setModificationRoot(ManagedSerializableDiff* obj, SPtr<ManagedSerializableDiff::ModifiedObject> val)
		{
			obj->mModificationRoot = val;
		}

	public:
		ManagedSerializableDiffRTTI()
		{
			addReflectablePtrField("mModificationRoot", 0, &ManagedSerializableDiffRTTI::getModificationRoot, 
				&ManagedSerializableDiffRTTI::setModificationRoot);

		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableDiff";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptSerializableDiff;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableDiff>();
		}
	};

	/** @} */
	/** @endcond */
}