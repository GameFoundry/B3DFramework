//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsRTTIType.h"
#include "BsManagedSerializableDictionary.h"

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
			Vector<ManagedSerializableDictionaryKeyValue>& sequentialData = any_cast_ref<Vector<ManagedSerializableDictionaryKeyValue>>(obj->mRTTIData);

			return sequentialData[arrayIdx];
		}

		void setEntry(ManagedSerializableDictionary* obj, UINT32 arrayIdx, ManagedSerializableDictionaryKeyValue& val)
		{ 
			obj->setFieldData(val.key, val.value);
		}

		UINT32 getNumEntries(ManagedSerializableDictionary* obj) 
		{ 
			Vector<ManagedSerializableDictionaryKeyValue>& sequentialData = any_cast_ref<Vector<ManagedSerializableDictionaryKeyValue>>(obj->mRTTIData);

			return (UINT32)sequentialData.size();
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

		void onSerializationStarted(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			ManagedSerializableDictionary* serializableObject = static_cast<ManagedSerializableDictionary*>(obj);

			Vector<ManagedSerializableDictionaryKeyValue> sequentialData;
			auto enumerator = serializableObject->getEnumerator();

			while (enumerator.moveNext())
				sequentialData.push_back(ManagedSerializableDictionaryKeyValue(enumerator.getKey(), enumerator.getValue()));

			serializableObject->mRTTIData = sequentialData;
		}

		void onSerializationEnded(IReflectable* obj, const UnorderedMap<String, UINT64>& params) override
		{
			ManagedSerializableDictionary* serializableObject = static_cast<ManagedSerializableDictionary*>(obj);
			serializableObject->mRTTIData = nullptr;
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
	};

	/** @} */
	/** @endcond */
}