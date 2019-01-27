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
		SPtr<ManagedSerializableFieldData> getKey(ManagedSerializableDictionaryKeyValue* obj)
		{
			return obj->key;
		}

		void setKey(ManagedSerializableDictionaryKeyValue* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->key = val;
		}

		SPtr<ManagedSerializableFieldData> getValue(ManagedSerializableDictionaryKeyValue* obj)
		{
			return obj->value;
		}

		void setValue(ManagedSerializableDictionaryKeyValue* obj, SPtr<ManagedSerializableFieldData> val)
		{
			obj->value = val;
		}

	public:
		ManagedSerializableDictionaryKeyValueRTTI()
		{
			addReflectablePtrField("key", 0, &ManagedSerializableDictionaryKeyValueRTTI::getKey, &ManagedSerializableDictionaryKeyValueRTTI::setKey);
			addReflectablePtrField("value", 1, &ManagedSerializableDictionaryKeyValueRTTI::getValue, &ManagedSerializableDictionaryKeyValueRTTI::setValue);
		}
		
		const String& getRTTIName() override
		{
			static String name = "ManagedSerializableDictionaryKeyValue";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptSerializableDictionaryKeyValue;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ManagedSerializableDictionaryKeyValue>();
		}
	};

	class BS_SCR_BE_EXPORT ManagedSerializableDictionaryRTTI : public RTTIType<ManagedSerializableDictionary, IReflectable, ManagedSerializableDictionaryRTTI>
	{
	private:
		SPtr<ManagedSerializableTypeInfoDictionary> getTypeInfo(ManagedSerializableDictionary* obj) { return obj->mDictionaryTypeInfo; }
		void setTypeInfo(ManagedSerializableDictionary* obj, SPtr<ManagedSerializableTypeInfoDictionary> val) { obj->mDictionaryTypeInfo = val; }

		ManagedSerializableDictionaryKeyValue& getEntry(ManagedSerializableDictionary* obj, UINT32 arrayIdx)
		{ 
			return mSequentialData[arrayIdx];
		}

		void setEntry(ManagedSerializableDictionary* obj, UINT32 arrayIdx, ManagedSerializableDictionaryKeyValue& val)
		{ 
			obj->setFieldData(val.key, val.value);
		}

		UINT32 getNumEntries(ManagedSerializableDictionary* obj) 
		{ 
			return (UINT32)mSequentialData.size();
		}

		void setNumEntries(ManagedSerializableDictionary* obj, UINT32 numEntries) 
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

		void onSerializationStarted(IReflectable* obj, SerializationContext* context) override
		{
			ManagedSerializableDictionary* serializableObject = static_cast<ManagedSerializableDictionary*>(obj);

			auto enumerator = serializableObject->getEnumerator();
			while (enumerator.moveNext())
				mSequentialData.push_back(ManagedSerializableDictionaryKeyValue(enumerator.getKey(), enumerator.getValue()));
		}

		const String& getRTTIName() override
		{
			static String name = "ScriptSerializableDictionary";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ScriptSerializableDictionary;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return ManagedSerializableDictionary::createEmpty();
		}

	private:
		Vector<ManagedSerializableDictionaryKeyValue> mSequentialData;
	};

	/** @} */
	/** @endcond */
}