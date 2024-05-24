//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Serialization/BsManagedSerializableDictionary.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-SEngine
	 *  @{
	 */

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableDictionaryKeyValueRTTI : public RTTIType<ManagedSerializableDictionaryKeyValue, IReflectable, ManagedSerializableDictionaryKeyValueRTTI>
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

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedSerializableDictionaryKeyValue>();
		}
	};

	class B3D_SCRIPT_INTEROP_EXPORT ManagedSerializableDictionaryRTTI : public RTTIType<ManagedSerializableDictionary, IReflectable, ManagedSerializableDictionaryRTTI>
	{
		Vector<ManagedSerializableDictionaryKeyValue> mSequentialData;

		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(mDictionaryTypeInfo, 0)
			B3D_RTTI_GENERATED_MEMBER_CONTAINER(mSequentialData, 1)
		B3D_RTTI_END_MEMBERS

	public:
		void OnOperationStarted(ManagedSerializableDictionary& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::ReadBit))
			{
				auto enumerator = object.GetEnumerator();
				while(enumerator.MoveNext())
					mSequentialData.push_back(ManagedSerializableDictionaryKeyValue(enumerator.GetKey(), enumerator.GetValue()));
			}
		}

		void OnOperationEnded(ManagedSerializableDictionary& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit))
			{
				for(const auto& entry : mSequentialData)
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

		SPtr<IReflectable> NewRttiObject()
		{
			return ManagedSerializableDictionary::CreateEmpty();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
