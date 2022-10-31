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

	class BS_SCR_BE_EXPORT ManagedSerializableDictionaryKeyValueRTTI : public RTTIType<ManagedSerializableDictionaryKeyValue, IReflectable, ManagedSerializableDictionaryKeyValueRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldData> GetKey(ManagedSerializableDictionaryKeyValue* obj)
		{
			return obj->Key;
		}

		void SetKey(ManagedSerializableDictionaryKeyValue* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->Key = val;
		}

		SPtr<ManagedSerializableFieldData> GetValue(ManagedSerializableDictionaryKeyValue* obj)
		{
			return obj->Value;
		}

		void SetValue(ManagedSerializableDictionaryKeyValue* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->Value = val;
		}

	public:
		ManagedSerializableDictionaryKeyValueRTTI()
		{
			AddReflectablePtrField("key", 0, &ManagedSerializableDictionaryKeyValueRTTI::GetKey, &ManagedSerializableDictionaryKeyValueRTTI::SetKey);
			AddReflectablePtrField("value", 1, &ManagedSerializableDictionaryKeyValueRTTI::GetValue, &ManagedSerializableDictionaryKeyValueRTTI::SetValue);
		}

		const String& GetRttiName()
		{
			static String name = "ManagedSerializableDictionaryKeyValue";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_ScriptSerializableDictionaryKeyValue;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<ManagedSerializableDictionaryKeyValue>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableDictionaryRTTI : public RTTIType<ManagedSerializableDictionary, IReflectable, ManagedSerializableDictionaryRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfoDictionary> GetTypeInfo(ManagedSerializableDictionary* obj) { return obj->mDictionaryTypeInfo; }

		void SetTypeInfo(ManagedSerializableDictionary* obj, SPtr<ManagedSerializableTypeInfoDictionary> val) { obj->mDictionaryTypeInfo = val; }

		ManagedSerializableDictionaryKeyValue& GetEntry(ManagedSerializableDictionary* obj, u32 arrayIdx)
		{
			return mSequentialData[arrayIdx];
		}

		void SetEntry(ManagedSerializableDictionary* obj, u32 arrayIdx, ManagedSerializableDictionaryKeyValue& val)
		{
			obj->SetFieldData(val.Key, val.Value);
		}

		u32 GetNumEntries(ManagedSerializableDictionary* obj)
		{
			return (u32)mSequentialData.size();
		}

		void SetNumEntries(ManagedSerializableDictionary* obj, u32 numEntries)
		{
			// Do nothing
		}

	public:
		ManagedSerializableDictionaryRTTI()
		{
			AddReflectablePtrField("mListTypeInfo", 0, &ManagedSerializableDictionaryRTTI::GetTypeInfo, &ManagedSerializableDictionaryRTTI::SetTypeInfo);
			AddReflectableArrayField("mEntries", 1, &ManagedSerializableDictionaryRTTI::GetEntry, &ManagedSerializableDictionaryRTTI::GetNumEntries, &ManagedSerializableDictionaryRTTI::SetEntry, &ManagedSerializableDictionaryRTTI::SetNumEntries);
		}

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context)
		{
			ManagedSerializableDictionary* serializableObject = static_cast<ManagedSerializableDictionary*>(obj);

			auto enumerator = serializableObject->GetEnumerator();
			while(enumerator.MoveNext())
				mSequentialData.push_back(ManagedSerializableDictionaryKeyValue(enumerator.GetKey(), enumerator.GetValue()));
		}

		const String& GetRttiName()
		{
			static String name = "ScriptSerializableDictionary";
			return name;
		}

		u32 GetRttiId()
		{
			return TID_ScriptSerializableDictionary;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return ManagedSerializableDictionary::CreateEmpty();
		}

	private:
		Vector<ManagedSerializableDictionaryKeyValue> mSequentialData;
	};

	/** @} */
	/** @endcond */
} // namespace bs
