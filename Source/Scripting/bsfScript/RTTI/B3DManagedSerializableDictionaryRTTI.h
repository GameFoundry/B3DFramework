//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "Serialization/B3DManagedSerializableDictionary.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableDictionaryKeyValueRTTI : public TRTTIType<ManagedSerializableDictionaryKeyValue, IReflectable, ManagedSerializableDictionaryKeyValueRTTI>
	{
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Key, 0)
			B3D_RTTI_MEMBER(Value, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "ManagedSerializableDictionaryKeyValue";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScriptSerializableDictionaryKeyValue;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedSerializableDictionaryKeyValue>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableDictionaryRTTI : public TRTTIType<ManagedSerializableDictionary, IReflectable, ManagedSerializableDictionaryRTTI>
	{
		Vector<ManagedSerializableDictionaryKeyValue> mDictionaryEntries;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mDictionaryTypeInfo, 0)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mDictionaryEntries, 1)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ManagedSerializableDictionary& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				auto enumerator = object.GetEnumerator();
				while(enumerator.MoveNext())
					mDictionaryEntries.push_back(ManagedSerializableDictionaryKeyValue(enumerator.GetKey(), enumerator.GetValue()));
			}
		}

		void OnOperationEnded(ManagedSerializableDictionary& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				for(const auto& entry : mDictionaryEntries)
					object.SetFieldData(entry.Key, entry.Value);
			}
		}

		const String& GetRttiName()
		{
			static String name = "ScriptSerializableDictionary";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScriptSerializableDictionary;
		}

		TShared<IReflectable> NewRttiObject()
		{
			return ManagedSerializableDictionary::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
