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

	class BS_SCR_BE_EXPORT ManagedSerializableDictionaryKeyValueRTTI :
		public RTTIType <ManagedSerializableDictionaryKeyValue, IReflectable, ManagedSerializableDictionaryKeyValueRTTI>
	{
	private:
		SPtr<ManagedSerializableFieldData> GetKey(ManagedSerializableDictionaryKeyValue* obj)
		{
			return obj->key;
		}

		void SetKey(ManagedSerializableDictionaryKeyValue* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->key = val;
		}

		SPtr<ManagedSerializableFieldData> GetValue(ManagedSerializableDictionaryKeyValue* obj)
		{
			return obj->value;
		}

		void SetValue(ManagedSerializableDictionaryKeyValue* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->value = val;
		}

	public:
		ManagedSerializableDictionaryKeyValueRTTI()
		{
			addReflectablePtrField("key", 0, &ManagedSerializableDictionaryKeyValueRTTI::getKey, &ManagedSerializableDictionaryKeyValueRTTI::setKey);
			addReflectablePtrField("value", 1, &ManagedSerializableDictionaryKeyValueRTTI::getValue, &ManagedSerializableDictionaryKeyValueRTTI::setValue);
		}
		
		const String& GetRTTIName() override
		{
			static String name = "ManagedSerializableDictionaryKeyValue";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_ScriptSerializableDictionaryKeyValue;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableDictionaryKeyValue>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableDictionaryRTTI : public RTTIType<ManagedSerializableDictionary, IReflectable, ManagedSerializableDictionaryRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfoDictionary> GetTypeInfo(ManagedSerializableDictionary* obj) { return obj->mDictionaryTypeInfo; }
		void SetTypeInfo(ManagedSerializableDictionary* obj, SPtr<ManagedSerializableTypeInfoDictionary> val) { obj->mDictionaryTypeInfo = val; }

		ManagedSerializableDictionaryKeyValue& GetEntry(ManagedSerializableDictionary* obj, UINT32 arrayIdx)
		{
			return mSequentialData[arrayIdx];
		}

		void SetEntry(ManagedSerializableDictionary* obj, UINT32 arrayIdx, ManagedSerializableDictionaryKeyValue& val)
		{
			obj->setFieldData(val.key, val.value);
		}

		UINT32 GetNumEntries(ManagedSerializableDictionary* obj)
		{
			return (UINT32)mSequentialData.size();
		}

		void SetNumEntries(ManagedSerializableDictionary* obj, UINT32 numEntries)
		{
			// Do nothing
		}
		
	public:
		ManagedSerializableDictionaryRTTI()
		{
			addReflectablePtrField("mListTypeInfo", 0, &ManagedSerializableDictionaryRTTI::getTypeInfo, &ManagedSerializableDictionaryRTTI::setTypeInfo);
			addReflectableArrayField("mEntries", 1, &ManagedSerializableDictionaryRTTI::getEntry, &ManagedSerializableDictionaryRTTI::getNumEntries,
				&ManagedSerializableDictionaryRTTI::setEntry, &ManagedSerializableDictionaryRTTI::setNumEntries);
		}

		void OnSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			ManagedSerializableDictionary* serializableObject = static_cast<ManagedSerializableDictionary*>(obj);

			auto enumerator = serializableObject->getEnumerator();
			while (enumerator.moveNext())
				mSequentialData.push_back(ManagedSerializableDictionaryKeyValue(enumerator.getKey(), enumerator.getValue()));
		}

		const String& GetRTTIName() override
		{
			static String name = "ScriptSerializableDictionary";
			return name;
		}

		UINT32 GetRTTIId() override
		{
			return TID_ScriptSerializableDictionary;
		}

		SPtr<IReflectable> NewRTTIObject() override
		{
			return ManagedSerializableDictionary::CreateEmpty();
		}

	private:
		Vector<ManagedSerializableDictionaryKeyValue> mSequentialData;
	};

	/** @} */
	/** @endcond */
}
