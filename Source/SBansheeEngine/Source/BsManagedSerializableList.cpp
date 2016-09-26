//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedSerializableList.h"
#include "BsManagedSerializableListRTTI.h"
#include "BsMonoManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsManagedSerializableField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoProperty.h"
#include "BsMonoUtil.h"

namespace BansheeEngine
{
	ManagedSerializableList::ManagedSerializableList(const ConstructPrivately& dummy)
		: mManagedInstance(nullptr), mAddMethod(nullptr), mAddRangeMethod(nullptr), mClearMethod(nullptr)
		, mCopyToMethod(nullptr), mItemProp(nullptr), mCountProp(nullptr), mNumElements(0)
	{

	}

	ManagedSerializableList::ManagedSerializableList(const ConstructPrivately& dummy, const SPtr<ManagedSerializableTypeInfoList>& typeInfo, MonoObject* managedInstance)
		: mManagedInstance(managedInstance), mAddMethod(nullptr), mAddRangeMethod(nullptr), mClearMethod(nullptr)
		, mCopyToMethod(nullptr), mItemProp(nullptr), mCountProp(nullptr), mListTypeInfo(typeInfo), mNumElements(0)
	{
		MonoClass* listClass = MonoManager::instance().findClass(MonoUtil::getClass(managedInstance));
		if(listClass == nullptr)
			return;

		initMonoObjects(listClass);

		mNumElements = getLengthInternal();
	}

	SPtr<ManagedSerializableList> ManagedSerializableList::createFromExisting(MonoObject* managedInstance, const SPtr<ManagedSerializableTypeInfoList>& typeInfo)
	{
		if(managedInstance == nullptr)
			return nullptr;

		String elementNs;
		String elementTypeName;
		MonoUtil::getClassName(managedInstance, elementNs, elementTypeName);

		String fullName = elementNs + "." + elementTypeName;

		if(ScriptAssemblyManager::instance().getSystemGenericListClass()->getFullName() != fullName)
			return nullptr;

		return bs_shared_ptr_new<ManagedSerializableList>(ConstructPrivately(), typeInfo, managedInstance);
	}

	SPtr<ManagedSerializableList> ManagedSerializableList::createNew(const SPtr<ManagedSerializableTypeInfoList>& typeInfo, UINT32 size)
	{
		return bs_shared_ptr_new<ManagedSerializableList>(ConstructPrivately(), typeInfo, createManagedInstance(typeInfo, size));
	}

	MonoObject* ManagedSerializableList::createManagedInstance(const SPtr<ManagedSerializableTypeInfoList>& typeInfo, UINT32 size)
	{
		if (!typeInfo->isTypeLoaded())
			return nullptr;

		::MonoClass* listMonoClass = typeInfo->getMonoClass();
		MonoClass* listClass = MonoManager::instance().findClass(listMonoClass);
		if (listClass == nullptr)
			return nullptr;

		void* params[1] = { &size };
		MonoObject* instance = listClass->createInstance("int", params);
		
		ScriptArray tempArray(typeInfo->mElementType->getMonoClass(), size);
		params[0] = tempArray.getInternal();

		MonoMethod* addRangeMethod = listClass->getMethod("AddRange", 1);
		addRangeMethod->invoke(instance, params);

		return instance;
	}

	SPtr<ManagedSerializableList> ManagedSerializableList::createEmpty()
	{
		return bs_shared_ptr_new<ManagedSerializableList>(ConstructPrivately());
	}

	void ManagedSerializableList::setFieldData(UINT32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val)
	{
		if (mManagedInstance != nullptr)
			mItemProp->setIndexed(mManagedInstance, arrayIdx, val->getValue(mListTypeInfo->mElementType));
		else
			mCachedEntries[arrayIdx] = val;
	}

	void ManagedSerializableList::addFieldDataInternal(const SPtr<ManagedSerializableFieldData>& val)
	{
		void* params[1];
		params[0] = val->getValue(mListTypeInfo->mElementType);
		mAddMethod->invoke(mManagedInstance, params);
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableList::getFieldData(UINT32 arrayIdx)
	{
		if (mManagedInstance != nullptr)
		{
			MonoObject* obj = mItemProp->getIndexed(mManagedInstance, arrayIdx);

			return ManagedSerializableFieldData::create(mListTypeInfo->mElementType, obj);
		}
		else
			return mCachedEntries[arrayIdx];
	}

	void ManagedSerializableList::resize(UINT32 newSize)
	{
		if (mManagedInstance != nullptr)
		{
			ScriptArray tempArray(mListTypeInfo->mElementType->getMonoClass(), newSize);

			UINT32 minSize = std::min(mNumElements, newSize);
			UINT32 dummy = 0;

			void* params[4];
			params[0] = &dummy;;
			params[1] = tempArray.getInternal();
			params[2] = &dummy;
			params[3] = &minSize;

			mCopyToMethod->invoke(getManagedInstance(), params);
			mClearMethod->invoke(getManagedInstance(), nullptr);

			params[0] = tempArray.getInternal();
			mAddRangeMethod->invoke(getManagedInstance(), params);
		}
		else
		{
			mCachedEntries.resize(newSize);
		}

		mNumElements = newSize;
	}

	void ManagedSerializableList::serialize()
	{
		if (mManagedInstance == nullptr)
			return;

		mNumElements = getLengthInternal();
		mCachedEntries = Vector<SPtr<ManagedSerializableFieldData>>(mNumElements);

		for (UINT32 i = 0; i < mNumElements; i++)
			mCachedEntries[i] = getFieldData(i);

		// Serialize children
		for (auto& fieldEntry : mCachedEntries)
			fieldEntry->serialize();

		mManagedInstance = nullptr;
	}

	void ManagedSerializableList::deserialize()
	{
		mManagedInstance = createManagedInstance(mListTypeInfo, mNumElements);

		if (mManagedInstance == nullptr)
		{
			mCachedEntries.clear();
			return;
		}

		MonoClass* listClass = MonoManager::instance().findClass(mListTypeInfo->getMonoClass());
		initMonoObjects(listClass);

		// Deserialize children
		for (auto& fieldEntry : mCachedEntries)
			fieldEntry->deserialize();

		UINT32 idx = 0;
		for (auto& entry : mCachedEntries)
		{
			setFieldData(idx, entry);
			idx++;
		}

		mCachedEntries.clear();
	}

	UINT32 ManagedSerializableList::getLengthInternal() const
	{
		MonoObject* length = mCountProp->get(mManagedInstance);

		if(length == nullptr)
			return 0;

		return *(UINT32*)MonoUtil::unbox(length);
	}

	void ManagedSerializableList::initMonoObjects(MonoClass* listClass)
	{
		mItemProp = listClass->getProperty("Item");
		mCountProp = listClass->getProperty("Count");
		mAddMethod = listClass->getMethod("Add", 1);
		mAddRangeMethod = listClass->getMethod("AddRange", 1);
		mClearMethod = listClass->getMethod("Clear");
		mCopyToMethod = listClass->getMethod("CopyTo", 4);
	}

	RTTITypeBase* ManagedSerializableList::getRTTIStatic()
	{
		return ManagedSerializableListRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableList::getRTTI() const
	{
		return ManagedSerializableList::getRTTIStatic();
	}
}