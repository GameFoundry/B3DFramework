//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStdRTTI.h"
#include "Serialization/B3DManagedSerializableDelta.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedFieldRTTI : public TRTTIType<ManagedSerializableDelta::ModifiedField, IReflectable, ModifiedFieldRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(ParentType, 0)
			B3D_RTTI_MEMBER(FieldType, 1)
			B3D_RTTI_MEMBER(Modification, 2)
		B3D_RTTI_END_MEMBERS

	public:
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

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedArrayEntryRTTI : public TRTTIType<ManagedSerializableDelta::ModifiedArrayEntry, IReflectable, ModifiedArrayEntryRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Idx, 0)
			B3D_RTTI_MEMBER(Modification, 1)
		B3D_RTTI_END_MEMBERS

	public:
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

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedDictionaryEntryRTTI : public TRTTIType<ManagedSerializableDelta::ModifiedDictionaryEntry, IReflectable, ModifiedDictionaryEntryRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Key, 0)
			B3D_RTTI_MEMBER(Modification, 1)
		B3D_RTTI_END_MEMBERS

	public:
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

	class B3D_SCRIPT_INTEROP_EXPORT ModificationRTTI : public TRTTIType<ManagedSerializableDelta::Modification, IReflectable, ModificationRTTI>
	{
	public:
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

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedObjectRTTI : public TRTTIType<ManagedSerializableDelta::ModifiedObject, ManagedSerializableDelta::Modification, ModifiedObjectRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(Entries, 0)
		B3D_RTTI_END_MEMBERS

	public:
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

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedArrayRTTI : public TRTTIType<ManagedSerializableDelta::ModifiedArray, ManagedSerializableDelta::Modification, ModifiedArrayRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(OrigSizes, 0)
			B3D_RTTI_MEMBER(NewSizes, 1)
			B3D_RTTI_MEMBER_CONTAINER(Entries, 2)
		B3D_RTTI_END_MEMBERS

	public:
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

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedDictionaryRTTI : public TRTTIType<ManagedSerializableDelta::ModifiedDictionary, ManagedSerializableDelta::Modification, ModifiedDictionaryRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(Removed, 0)
			B3D_RTTI_MEMBER_CONTAINER(Entries, 1)
		B3D_RTTI_END_MEMBERS

	public:
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

	class B3D_SCRIPT_INTEROP_EXPORT ModifiedEntryRTTI : public TRTTIType<ManagedSerializableDelta::ModifiedEntry, ManagedSerializableDelta::Modification, ModifiedEntryRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Value, 0)
		B3D_RTTI_END_MEMBERS

	public:
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

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableDeltaRTTI : public TRTTIType<ManagedSerializableDelta, IReflectable, ManagedSerializableDeltaRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mModificationRoot, 0)
		B3D_RTTI_END_MEMBERS

	public:
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
} // namespace b3d
