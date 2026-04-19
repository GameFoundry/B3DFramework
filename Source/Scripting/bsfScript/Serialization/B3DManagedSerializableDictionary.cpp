//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DManagedSerializableDictionary.h"
#include "RTTI/B3DManagedSerializableDictionaryRTTI.h"
#include "B3DMonoManager.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Serialization/B3DManagedSerializableField.h"
#include "B3DMonoClass.h"
#include "B3DMonoMethod.h"
#include "B3DMonoProperty.h"
#include "B3DMonoUtil.h"

using namespace b3d;
ManagedSerializableDictionaryKeyValue::ManagedSerializableDictionaryKeyValue(const SPtr<ManagedSerializableFieldData>& key, const SPtr<ManagedSerializableFieldData>& value)
	: Key(key), Value(value)
{
}

RTTIType* ManagedSerializableDictionaryKeyValue::GetRttiStatic()
{
	return ManagedSerializableDictionaryKeyValueRTTI::Instance();
}

RTTIType* ManagedSerializableDictionaryKeyValue::GetRtti() const
{
	return ManagedSerializableDictionaryKeyValue::GetRttiStatic();
}

ManagedSerializableDictionary::Enumerator::Enumerator(const ManagedSerializableDictionary* parent)
	: mIteratorInitialized(false), mParent(parent)
{
	MonoArray* keysArray = nullptr;
	MonoArray* valuesArray = nullptr;
	if(parent->mGCHandle != 0)
	{
		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(parent->mGCHandle);

		mNumEntries = *(u32*)MonoUtil::Unbox(parent->mCountProp->Get(managedInstance));
		MonoObject* keyCollection = parent->mKeysProp->Get(managedInstance);
		MonoObject* valueCollection = parent->mValuesProp->Get(managedInstance);

		mKeyType = parent->mDictionaryTypeInfo->KeyType->GetMonoClass();
		mValueType = parent->mDictionaryTypeInfo->ValueType->GetMonoClass();

		ScriptArray keys(mKeyType, mNumEntries);
		ScriptArray values(mValueType, mNumEntries);

		u32 offset = 0;
		void* keyParams[2] = { keys.GetInternal(), &offset };
		parent->mKeysCopyTo->Invoke(keyCollection, keyParams);

		void* valueParams[2] = { values.GetInternal(), &offset };
		parent->mValuesCopyTo->Invoke(valueCollection, valueParams);

		keysArray = keys.GetInternal();
		valuesArray = values.GetInternal();
	}
	else
		mNumEntries = (u32)parent->mCachedEntries.size();

	// Note: Handle needed since Enumerator will be on the stack? meaning the GC should be able to find the references.
	if(keysArray && valuesArray)
	{
		mKeysArrayHandle = MonoUtil::NewGcHandle((MonoObject*)keysArray, false);
		mValuesArrayHandle = MonoUtil::NewGcHandle((MonoObject*)valuesArray, false);
	}
}

ManagedSerializableDictionary::Enumerator::Enumerator(const Enumerator& other)
	: mNumEntries(other.mNumEntries), mIteratorInitialized(false), mParent(other.mParent)
{
	if(other.mKeysArrayHandle != 0 && other.mValuesArrayHandle != 0)
	{
		MonoObject* keysArray = MonoUtil::GetObjectFromGcHandle(other.mKeysArrayHandle);
		mKeysArrayHandle = MonoUtil::NewGcHandle(keysArray, false);

		MonoObject* valuesArray = MonoUtil::GetObjectFromGcHandle(other.mValuesArrayHandle);
		mValuesArrayHandle = MonoUtil::NewGcHandle(valuesArray, false);

		mKeyType = other.mKeyType;
		mValueType = other.mValueType;
	}
}

ManagedSerializableDictionary::Enumerator::~Enumerator()
{
	if(mKeysArrayHandle != 0)
		MonoUtil::FreeGcHandle(mKeysArrayHandle);

	if(mValuesArrayHandle != 0)
		MonoUtil::FreeGcHandle(mValuesArrayHandle);
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
		MonoUtil::FreeGcHandle(mKeysArrayHandle);
		mKeysArrayHandle = 0;
	}

	if(mValuesArrayHandle != 0)
	{
		MonoUtil::FreeGcHandle(mValuesArrayHandle);
		mValuesArrayHandle = 0;
	}

	if(other.mKeysArrayHandle != 0 && other.mValuesArrayHandle != 0)
	{
		MonoObject* keysArray = MonoUtil::GetObjectFromGcHandle(other.mKeysArrayHandle);
		mKeysArrayHandle = MonoUtil::NewGcHandle(keysArray, false);

		MonoObject* valuesArray = MonoUtil::GetObjectFromGcHandle(other.mValuesArrayHandle);
		mValuesArrayHandle = MonoUtil::NewGcHandle(valuesArray, false);

		mKeyType = other.mKeyType;
		mValueType = other.mValueType;
	}

	return *this;
}

SPtr<ManagedSerializableFieldData> ManagedSerializableDictionary::Enumerator::GetKey() const
{
	if(mKeysArrayHandle != 0)
	{
		MonoArray* keysArray = (MonoArray*)MonoUtil::GetObjectFromGcHandle(mKeysArrayHandle);
		ScriptArray keys(keysArray);

		if(mCurrentIdx != (u32)-1)
		{
			void* val = (void*)keys.GetRaw(mCurrentIdx, keys.ElementSize());

			MonoObject* obj = nullptr;
			if(MonoUtil::IsValueType(mKeyType))
			{
				if(val != nullptr)
					obj = MonoUtil::Box(mKeyType, val);
			}
			else
				obj = *(MonoObject**)val;

			return ManagedSerializableFieldData::Create(mParent->mDictionaryTypeInfo->KeyType, obj);
		}
		else
			return nullptr;
	}
	else
	{
		return mCachedIter->first;
	}
}

SPtr<ManagedSerializableFieldData> ManagedSerializableDictionary::Enumerator::GetValue() const
{
	if(mValuesArrayHandle != 0)
	{
		MonoArray* valuesArray = (MonoArray*)MonoUtil::GetObjectFromGcHandle(mValuesArrayHandle);
		ScriptArray values(valuesArray);

		if(mCurrentIdx != (u32)-1)
		{
			void* val = (void*)values.GetRaw(mCurrentIdx, values.ElementSize());

			MonoObject* obj = nullptr;
			if(MonoUtil::IsValueType(mValueType))
			{
				if(val != nullptr)
					obj = MonoUtil::Box(mValueType, val);
			}
			else
				obj = *(MonoObject**)val;

			return ManagedSerializableFieldData::Create(mParent->mDictionaryTypeInfo->ValueType, obj);
		}
		else
			return nullptr;
	}
	else
	{
		return mCachedIter->second;
	}
}

bool ManagedSerializableDictionary::Enumerator::MoveNext()
{
	if(mKeysArrayHandle != 0 && mValuesArrayHandle != 0)
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
		if(!mIteratorInitialized)
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
{}

ManagedSerializableDictionary::ManagedSerializableDictionary(const ConstructPrivately& dummy, const SPtr<ManagedTypeInfoDictionary>& typeInfo, MonoObject* managedInstance)
	: mDictionaryTypeInfo(typeInfo)
{
	mGCHandle = MonoUtil::NewGcHandle(managedInstance, false);

	MonoClass* dictClass = MonoManager::Instance().FindClass(MonoUtil::GetClass(managedInstance));
	if(dictClass == nullptr)
		return;

	InitMonoObjects(dictClass);
}

ManagedSerializableDictionary::~ManagedSerializableDictionary()
{
	if(mGCHandle != 0)
	{
		MonoUtil::FreeGcHandle(mGCHandle);
		mGCHandle = 0;
	}
}

SPtr<ManagedSerializableDictionary> ManagedSerializableDictionary::CreateFromExisting(MonoObject* managedInstance, const SPtr<ManagedTypeInfoDictionary>& typeInfo)
{
	if(managedInstance == nullptr)
		return nullptr;

	String elementNs;
	String elementTypeName;
	MonoUtil::GetClassName(managedInstance, elementNs, elementTypeName);

	String fullName = elementNs + "." + elementTypeName;

	if(ScriptAssemblyManager::Instance().GetBuiltinClasses().SystemGenericDictionaryClass->GetFullName() != fullName)
		return nullptr;

	return B3DMakeShared<ManagedSerializableDictionary>(ConstructPrivately(), typeInfo, managedInstance);
}

SPtr<ManagedSerializableDictionary> ManagedSerializableDictionary::CreateNew(const SPtr<ManagedTypeInfoDictionary>& typeInfo)
{
	return B3DMakeShared<ManagedSerializableDictionary>(ConstructPrivately(), typeInfo, CreateManagedInstance(typeInfo));
}

MonoObject* ManagedSerializableDictionary::CreateManagedInstance(const SPtr<ManagedTypeInfoDictionary>& typeInfo)
{
	if(!typeInfo->IsTypeLoaded())
		return nullptr;

	::MonoClass* dictionaryMonoClass = typeInfo->GetMonoClass();
	MonoClass* dictionaryClass = MonoManager::Instance().FindClass(dictionaryMonoClass);
	if(dictionaryClass == nullptr)
		return nullptr;

	return dictionaryClass->CreateInstance();
}

SPtr<ManagedSerializableDictionary> ManagedSerializableDictionary::CreateEmpty()
{
	return B3DMakeShared<ManagedSerializableDictionary>(ConstructPrivately());
}

MonoObject* ManagedSerializableDictionary::GetManagedInstance() const
{
	if(mGCHandle != 0)
		return MonoUtil::GetObjectFromGcHandle(mGCHandle);

	return nullptr;
}

void ManagedSerializableDictionary::Serialize()
{
	if(mGCHandle == 0)
		return;

	MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
	MonoClass* dictionaryClass = MonoManager::Instance().FindClass(MonoUtil::GetClass(managedInstance));
	if(dictionaryClass == nullptr)
		return;

	InitMonoObjects(dictionaryClass);
	mCachedEntries.clear();

	Enumerator enumerator = GetEnumerator();

	while(enumerator.MoveNext())
	{
		SPtr<ManagedSerializableFieldData> key = enumerator.GetKey();
		mCachedEntries.insert(std::make_pair(key, enumerator.GetValue()));
	}

	// Serialize children
	for(auto& fieldEntry : mCachedEntries)
	{
		fieldEntry.first->Serialize();
		fieldEntry.second->Serialize();
	}

	MonoUtil::FreeGcHandle(mGCHandle);
	mGCHandle = 0;
}

MonoObject* ManagedSerializableDictionary::Deserialize()
{
	MonoObject* managedInstance = CreateManagedInstance(mDictionaryTypeInfo);
	if(managedInstance == nullptr)
		return nullptr;

	::MonoClass* dictionaryMonoClass = mDictionaryTypeInfo->GetMonoClass();
	MonoClass* dictionaryClass = MonoManager::Instance().FindClass(dictionaryMonoClass);
	if(dictionaryClass == nullptr)
		return nullptr;

	InitMonoObjects(dictionaryClass);

	// Deserialize children
	for(auto& fieldEntry : mCachedEntries)
	{
		fieldEntry.first->Deserialize();
		fieldEntry.second->Deserialize();
	}

	u32 idx = 0;
	for(auto& entry : mCachedEntries)
	{
		SetFieldData(managedInstance, entry.first, entry.second);
		idx++;
	}

	return managedInstance;
}

SPtr<ManagedSerializableFieldData> ManagedSerializableDictionary::GetFieldData(const SPtr<ManagedSerializableFieldData>& key)
{
	if(mGCHandle != 0)
	{
		MonoObject* value = nullptr;

		void* params[2];
		params[0] = key->GetValue(mDictionaryTypeInfo->KeyType);
		params[1] = &value;

		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
		mTryGetValueMethod->Invoke(managedInstance, params);

		MonoObject* boxedValue = value;
		::MonoClass* valueTypeClass = mDictionaryTypeInfo->ValueType->GetMonoClass();
		if(MonoUtil::IsValueType(valueTypeClass))
		{
			if(value != nullptr)
				boxedValue = MonoUtil::Box(valueTypeClass, &value);
		}

		return ManagedSerializableFieldData::Create(mDictionaryTypeInfo->ValueType, boxedValue);
	}
	else
	{
		return mCachedEntries[key];
	}
}

void ManagedSerializableDictionary::SetFieldData(const SPtr<ManagedSerializableFieldData>& key, const SPtr<ManagedSerializableFieldData>& val)
{
	if(mGCHandle != 0)
	{
		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
		SetFieldData(managedInstance, key, val);
	}
	else
	{
		mCachedEntries[key] = val;
	}
}

void ManagedSerializableDictionary::SetFieldData(MonoObject* obj, const SPtr<ManagedSerializableFieldData>& key, const SPtr<ManagedSerializableFieldData>& val)
{
	void* params[2];
	params[0] = key->GetValue(mDictionaryTypeInfo->KeyType);
	params[1] = val->GetValue(mDictionaryTypeInfo->ValueType);

	mAddMethod->Invoke(obj, params);
}

void ManagedSerializableDictionary::RemoveFieldData(const SPtr<ManagedSerializableFieldData>& key)
{
	if(mGCHandle != 0)
	{
		void* params[1];
		params[0] = key->GetValue(mDictionaryTypeInfo->KeyType);

		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
		mRemoveMethod->Invoke(managedInstance, params);
	}
	else
	{
		auto findIter = mCachedEntries.find(key);
		if(findIter != mCachedEntries.end())
			mCachedEntries.erase(findIter);
	}
}

bool ManagedSerializableDictionary::Contains(const SPtr<ManagedSerializableFieldData>& key) const
{
	if(mGCHandle != 0)
	{
		void* params[1];
		params[0] = key->GetValue(mDictionaryTypeInfo->KeyType);

		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
		MonoObject* returnVal = mContainsKeyMethod->Invoke(managedInstance, params);
		return *(bool*)MonoUtil::Unbox(returnVal);
	}
	else
		return mCachedEntries.find(key) != mCachedEntries.end();
}

ManagedSerializableDictionary::Enumerator ManagedSerializableDictionary::GetEnumerator() const
{
	return Enumerator(this);
}

void ManagedSerializableDictionary::InitMonoObjects(MonoClass* dictionaryClass)
{
	mAddMethod = dictionaryClass->GetMethod("Add", 2);
	mRemoveMethod = dictionaryClass->GetMethod("Remove", 1);
	mTryGetValueMethod = dictionaryClass->GetMethod("TryGetValue", 2);
	mContainsKeyMethod = dictionaryClass->GetMethod("ContainsKey", 1);
	mCountProp = dictionaryClass->GetProperty("Count");
	mKeysProp = dictionaryClass->GetProperty("Keys");
	mValuesProp = dictionaryClass->GetProperty("Values");

	MonoClass* keyCollectionClass = mKeysProp->GetReturnType();
	mKeysCopyTo = keyCollectionClass->GetMethod("CopyTo", 2);

	MonoClass* valueCollectionClass = mValuesProp->GetReturnType();
	mValuesCopyTo = valueCollectionClass->GetMethod("CopyTo", 2);
}

RTTIType* ManagedSerializableDictionary::GetRttiStatic()
{
	return ManagedSerializableDictionaryRTTI::Instance();
}

RTTIType* ManagedSerializableDictionary::GetRtti() const
{
	return ManagedSerializableDictionary::GetRttiStatic();
}
