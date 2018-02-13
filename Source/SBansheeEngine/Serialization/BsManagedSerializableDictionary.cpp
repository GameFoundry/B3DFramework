//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Serialization/BsManagedSerializableDictionary.h"
#include "RTTI/BsManagedSerializableDictionaryRTTI.h"
#include "BsMonoManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoProperty.h"
#include "BsMonoUtil.h"

namespace bs
{
	ManagedSerializableDictionaryKeyValue::ManagedSerializableDictionaryKeyValue(const SPtr<ManagedSerializableFieldData>& key,
		const SPtr<ManagedSerializableFieldData>& value)
		:key(key), value(value)
	{
		
	}

	RTTITypeBase* ManagedSerializableDictionaryKeyValue::getRTTIStatic()
	{
		return ManagedSerializableDictionaryKeyValueRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDictionaryKeyValue::getRTTI() const
	{
		return ManagedSerializableDictionaryKeyValue::getRTTIStatic();
	}

	inline size_t ManagedSerializableDictionary::Hash::operator()(const SPtr<ManagedSerializableFieldData>& x) const
	{
		return x->getHash();
	}

	inline bool ManagedSerializableDictionary::Equals::operator()(const SPtr<ManagedSerializableFieldData>& a, const SPtr<ManagedSerializableFieldData>& b) const
	{
		return a->equals(b);
	}

	ManagedSerializableDictionary::Enumerator::Enumerator(const ManagedSerializableDictionary* parent)
		: mIteratorInitialized(false), mParent(parent)
	{
		MonoArray* keysArray = nullptr;
		MonoArray* valuesArray = nullptr;
		if(parent->mGCHandle != 0)
		{
			MonoObject* managedInstance = MonoUtil::getObjectFromGCHandle(parent->mGCHandle);

			mNumEntries = *(UINT32*)MonoUtil::unbox(parent->mCountProp->get(managedInstance));
			MonoObject* keyCollection = parent->mKeysProp->get(managedInstance);
			MonoObject* valueCollection = parent->mValuesProp->get(managedInstance);

			mKeyType = parent->mDictionaryTypeInfo->mKeyType->getMonoClass();
			mValueType = parent->mDictionaryTypeInfo->mValueType->getMonoClass();

			ScriptArray keys(mKeyType, mNumEntries);
			ScriptArray values(mValueType, mNumEntries);

			UINT32 offset = 0;
			void* keyParams[2] = { keys.getInternal(), &offset };
			parent->mKeysCopyTo->invoke(keyCollection, keyParams);

			void* valueParams[2] = { values.getInternal(), &offset };
			parent->mValuesCopyTo->invoke(valueCollection, valueParams);

			keysArray = keys.getInternal();
			valuesArray = values.getInternal();
		}
		else
			mNumEntries = (UINT32)parent->mCachedEntries.size();

		// Note: Handle needed since Enumerator will be on the stack? meaning the GC should be able to find the references.
		if(keysArray && valuesArray)
		{
			mKeysArrayHandle = MonoUtil::newGCHandle((MonoObject*)keysArray, false);
			mValuesArrayHandle = MonoUtil::newGCHandle((MonoObject*)valuesArray, false);
		}
	}

	ManagedSerializableDictionary::Enumerator::Enumerator(const Enumerator& other)
		: mNumEntries(other.mNumEntries), mIteratorInitialized(false), mParent(other.mParent)
	{
		if(other.mKeysArrayHandle != 0 && other.mValuesArrayHandle != 0)
		{
			MonoObject* keysArray = MonoUtil::getObjectFromGCHandle(other.mKeysArrayHandle);
			mKeysArrayHandle = MonoUtil::newGCHandle(keysArray, false);

			MonoObject* valuesArray = MonoUtil::getObjectFromGCHandle(other.mValuesArrayHandle);
			mValuesArrayHandle = MonoUtil::newGCHandle(valuesArray, false);

			mKeyType = other.mKeyType;
			mValueType = other.mValueType;
		}
	}

	ManagedSerializableDictionary::Enumerator::~Enumerator()
	{
		if(mKeysArrayHandle != 0)
			MonoUtil::freeGCHandle(mKeysArrayHandle);

		if(mValuesArrayHandle != 0)
			MonoUtil::freeGCHandle(mValuesArrayHandle);
	}

	ManagedSerializableDictionary::Enumerator& 
		ManagedSerializableDictionary::Enumerator::operator=(const Enumerator& other)
	{
		mNumEntries = other.mNumEntries;
		mIteratorInitialized = false;
		mParent = other.mParent;
		mKeyType = nullptr;
		mValueType = nullptr;

		if(mKeysArrayHandle != 0)
		{
			MonoUtil::freeGCHandle(mKeysArrayHandle);
			mKeysArrayHandle = 0;
		}

		if(mValuesArrayHandle != 0)
		{
			MonoUtil::freeGCHandle(mValuesArrayHandle);
			mValuesArrayHandle = 0;
		}

		if(other.mKeysArrayHandle != 0 && other.mValuesArrayHandle != 0)
		{
			MonoObject* keysArray = MonoUtil::getObjectFromGCHandle(other.mKeysArrayHandle);
			mKeysArrayHandle = MonoUtil::newGCHandle(keysArray, false);

			MonoObject* valuesArray = MonoUtil::getObjectFromGCHandle(other.mValuesArrayHandle);
			mValuesArrayHandle = MonoUtil::newGCHandle(valuesArray, false);

			mKeyType = other.mKeyType;
			mValueType = other.mValueType;
		}

		return *this;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDictionary::Enumerator::getKey() const
	{
		if (mKeysArrayHandle != 0)
		{
			MonoArray* keysArray = (MonoArray*)MonoUtil::getObjectFromGCHandle(mKeysArrayHandle);
			ScriptArray keys(keysArray);

			if(mCurrentIdx != (UINT32)-1)
			{
				void* val = (void*)keys.getRaw(mCurrentIdx, keys.elementSize());

				MonoObject* obj = nullptr;
				if (MonoUtil::isValueType(mKeyType))
				{
					if (val != nullptr)
						obj = MonoUtil::box(mKeyType, val);
				}
				else
					obj = *(MonoObject**)val;

				return ManagedSerializableFieldData::create(mParent->mDictionaryTypeInfo->mKeyType, obj);
			}
			else
				return nullptr;
		}
		else
		{
			return mCachedIter->first;
		}
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDictionary::Enumerator::getValue() const
	{
		if (mValuesArrayHandle != 0)
		{
			MonoArray* valuesArray = (MonoArray*)MonoUtil::getObjectFromGCHandle(mValuesArrayHandle);
			ScriptArray values(valuesArray);

			if(mCurrentIdx != (UINT32)-1)
			{
				void* val = (void*)values.getRaw(mCurrentIdx, values.elementSize());

				MonoObject* obj = nullptr;
				if (MonoUtil::isValueType(mValueType))
				{
					if (val != nullptr)
						obj = MonoUtil::box(mValueType, val);
				}
				else
					obj = *(MonoObject**)val;

				return ManagedSerializableFieldData::create(mParent->mDictionaryTypeInfo->mValueType, obj);
			}
			else
				return nullptr;
		}
		else
		{
			return mCachedIter->second;
		}
	}

	bool ManagedSerializableDictionary::Enumerator::moveNext()
	{
		if (mKeysArrayHandle != 0 && mValuesArrayHandle != 0)
		{
			if((mCurrentIdx + 1) < mNumEntries)
			{
				mCurrentIdx++;
				return true;
			}

			return false;
		}
		else
		{
			if (!mIteratorInitialized)
			{
				mCachedIter = mParent->mCachedEntries.begin();
				mIteratorInitialized = true;
			}
			else
				++mCachedIter;

			return mCachedIter != mParent->mCachedEntries.end();
		}
	}

	ManagedSerializableDictionary::ManagedSerializableDictionary(const ConstructPrivately& dummy)
	{ }

	ManagedSerializableDictionary::ManagedSerializableDictionary(const ConstructPrivately& dummy, const SPtr<ManagedSerializableTypeInfoDictionary>& typeInfo, MonoObject* managedInstance)
		: mDictionaryTypeInfo(typeInfo)
	{
		mGCHandle = MonoUtil::newGCHandle(managedInstance, false);

		MonoClass* dictClass = MonoManager::instance().findClass(MonoUtil::getClass(managedInstance));
		if (dictClass == nullptr)
			return;

		initMonoObjects(dictClass);
	}

	ManagedSerializableDictionary::~ManagedSerializableDictionary()
	{
		if(mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}
	}

	SPtr<ManagedSerializableDictionary> ManagedSerializableDictionary::createFromExisting(MonoObject* managedInstance, 
		const SPtr<ManagedSerializableTypeInfoDictionary>& typeInfo)
	{
		if(managedInstance == nullptr)
			return nullptr;

		String elementNs;
		String elementTypeName;
		MonoUtil::getClassName(managedInstance, elementNs, elementTypeName);

		String fullName = elementNs + "." + elementTypeName;

		if(ScriptAssemblyManager::instance().getSystemGenericDictionaryClass()->getFullName() != fullName)
			return nullptr;

		return bs_shared_ptr_new<ManagedSerializableDictionary>(ConstructPrivately(), typeInfo, managedInstance);
	}

	SPtr<ManagedSerializableDictionary> ManagedSerializableDictionary::createNew(const SPtr<ManagedSerializableTypeInfoDictionary>& typeInfo)
	{
		return bs_shared_ptr_new<ManagedSerializableDictionary>(ConstructPrivately(), typeInfo, createManagedInstance(typeInfo));
	}

	MonoObject* ManagedSerializableDictionary::createManagedInstance(const SPtr<ManagedSerializableTypeInfoDictionary>& typeInfo)
	{
		if (!typeInfo->isTypeLoaded())
			return nullptr;

		::MonoClass* dictionaryMonoClass = typeInfo->getMonoClass();
		MonoClass* dictionaryClass = MonoManager::instance().findClass(dictionaryMonoClass);
		if (dictionaryClass == nullptr)
			return nullptr;

		return dictionaryClass->createInstance();
	}

	SPtr<ManagedSerializableDictionary> ManagedSerializableDictionary::createEmpty()
	{
		return bs_shared_ptr_new<ManagedSerializableDictionary>(ConstructPrivately());
	}

	MonoObject* ManagedSerializableDictionary::getManagedInstance() const
	{
		if(mGCHandle != 0)
			return MonoUtil::getObjectFromGCHandle(mGCHandle);

		return nullptr;
	}

	void ManagedSerializableDictionary::serialize()
	{
		if (mGCHandle == 0)
			return;

		MonoObject* managedInstance = MonoUtil::getObjectFromGCHandle(mGCHandle);
		MonoClass* dictionaryClass = MonoManager::instance().findClass(MonoUtil::getClass(managedInstance));
		if (dictionaryClass == nullptr)
			return;

		initMonoObjects(dictionaryClass);
		mCachedEntries.clear();

		Enumerator enumerator = getEnumerator();

		while (enumerator.moveNext())
		{
			SPtr<ManagedSerializableFieldData> key = enumerator.getKey();
			mCachedEntries.insert(std::make_pair(key, enumerator.getValue()));
		}

		// Serialize children
		for (auto& fieldEntry : mCachedEntries)
		{
			fieldEntry.first->serialize();
			fieldEntry.second->serialize();
		}

		MonoUtil::freeGCHandle(mGCHandle);
		mGCHandle = 0;
	}

	MonoObject* ManagedSerializableDictionary::deserialize()
	{
		MonoObject* managedInstance = createManagedInstance(mDictionaryTypeInfo);
		if (managedInstance == nullptr)
			return nullptr;

		::MonoClass* dictionaryMonoClass = mDictionaryTypeInfo->getMonoClass();
		MonoClass* dictionaryClass = MonoManager::instance().findClass(dictionaryMonoClass);
		if (dictionaryClass == nullptr)
			return nullptr;

		initMonoObjects(dictionaryClass);

		// Deserialize children
		for (auto& fieldEntry : mCachedEntries)
		{
			fieldEntry.first->deserialize();
			fieldEntry.second->deserialize();
		}

		UINT32 idx = 0;
		for (auto& entry : mCachedEntries)
		{
			setFieldData(managedInstance, entry.first, entry.second);
			idx++;
		}

		return managedInstance;
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableDictionary::getFieldData(const SPtr<ManagedSerializableFieldData>& key)
	{
		if (mGCHandle != 0)
		{
			MonoObject* value = nullptr;

			void* params[2];
			params[0] = key->getValue(mDictionaryTypeInfo->mKeyType);
			params[1] = &value;

			MonoObject* managedInstance = MonoUtil::getObjectFromGCHandle(mGCHandle);
			mTryGetValueMethod->invoke(managedInstance, params);

			MonoObject* boxedValue = value;
			::MonoClass* valueTypeClass = mDictionaryTypeInfo->mValueType->getMonoClass();
			if (MonoUtil::isValueType(valueTypeClass))
			{
				if (value != nullptr)
					boxedValue = MonoUtil::box(valueTypeClass, &value);
			}

			return ManagedSerializableFieldData::create(mDictionaryTypeInfo->mValueType, boxedValue);
		}
		else
		{
			return mCachedEntries[key];
		}
	}

	void ManagedSerializableDictionary::setFieldData(const SPtr<ManagedSerializableFieldData>& key, const SPtr<ManagedSerializableFieldData>& val)
	{
		if (mGCHandle != 0)
		{
			MonoObject* managedInstance = MonoUtil::getObjectFromGCHandle(mGCHandle);
			setFieldData(managedInstance, key, val);
		}
		else
		{
			mCachedEntries[key] = val;
		}
	}

	void ManagedSerializableDictionary::setFieldData(MonoObject* obj, const SPtr<ManagedSerializableFieldData>& key, const SPtr<ManagedSerializableFieldData>& val)
	{
		void* params[2];
		params[0] = key->getValue(mDictionaryTypeInfo->mKeyType);
		params[1] = val->getValue(mDictionaryTypeInfo->mValueType);

		mAddMethod->invoke(obj, params);
	}

	void ManagedSerializableDictionary::removeFieldData(const SPtr<ManagedSerializableFieldData>& key)
	{
		if (mGCHandle != 0)
		{
			void* params[1];
			params[0] = key->getValue(mDictionaryTypeInfo->mKeyType);

			MonoObject* managedInstance = MonoUtil::getObjectFromGCHandle(mGCHandle);
			mRemoveMethod->invoke(managedInstance, params);
		}
		else
		{
			auto findIter = mCachedEntries.find(key);
			if (findIter != mCachedEntries.end())
				mCachedEntries.erase(findIter);
		}
	}

	bool ManagedSerializableDictionary::contains(const SPtr<ManagedSerializableFieldData>& key) const
	{
		if (mGCHandle != 0)
		{
			void* params[1];
			params[0] = key->getValue(mDictionaryTypeInfo->mKeyType);

			MonoObject* managedInstance = MonoUtil::getObjectFromGCHandle(mGCHandle);
			MonoObject* returnVal = mContainsKeyMethod->invoke(managedInstance, params);
			return *(bool*)MonoUtil::unbox(returnVal);
		}
		else
			return mCachedEntries.find(key) != mCachedEntries.end();
	}

	ManagedSerializableDictionary::Enumerator ManagedSerializableDictionary::getEnumerator() const
	{
		return Enumerator(this);
	}

	void ManagedSerializableDictionary::initMonoObjects(MonoClass* dictionaryClass)
	{
		mAddMethod = dictionaryClass->getMethod("Add", 2);
		mRemoveMethod = dictionaryClass->getMethod("Remove", 1);
		mTryGetValueMethod = dictionaryClass->getMethod("TryGetValue", 2);
		mContainsKeyMethod = dictionaryClass->getMethod("ContainsKey", 1);
		mCountProp = dictionaryClass->getProperty("Count");
		mKeysProp = dictionaryClass->getProperty("Keys");
		mValuesProp = dictionaryClass->getProperty("Values");

		MonoClass* keyCollectionClass = mKeysProp->getReturnType();
		mKeysCopyTo = keyCollectionClass->getMethod("CopyTo", 2);

		MonoClass* valueCollectionClass = mValuesProp->getReturnType();
		mValuesCopyTo = valueCollectionClass->getMethod("CopyTo", 2);
	}

	RTTITypeBase* ManagedSerializableDictionary::getRTTIStatic()
	{
		return ManagedSerializableDictionaryRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableDictionary::getRTTI() const
	{
		return ManagedSerializableDictionary::getRTTIStatic();
	}
}