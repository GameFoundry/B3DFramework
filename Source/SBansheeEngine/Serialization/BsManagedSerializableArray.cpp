//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Serialization/BsManagedSerializableArray.h"
#include "RTTI/BsManagedSerializableArrayRTTI.h"
#include "BsMonoManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Serialization/BsManagedSerializableField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoArray.h"

namespace bs
{
	ManagedSerializableArray::ManagedSerializableArray(const ConstructPrivately& dummy)
	{

	}

	ManagedSerializableArray::ManagedSerializableArray(const ConstructPrivately& dummy, 
		const SPtr<ManagedSerializableTypeInfoArray>& typeInfo, MonoObject* managedInstance)
		: mArrayTypeInfo(typeInfo)
		
	{
		mGCHandle = MonoUtil::newGCHandle(managedInstance, false);

		ScriptArray scriptArray((MonoArray*)managedInstance);
		mElemSize = scriptArray.elementSize();

		initMonoObjects();

		mNumElements.resize(typeInfo->mRank);
		for(UINT32 i = 0; i < typeInfo->mRank; i++)
			mNumElements[i] = getLengthInternal(i);
	}

	ManagedSerializableArray::~ManagedSerializableArray()
	{
		if(mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}
	}

	SPtr<ManagedSerializableArray> ManagedSerializableArray::createFromExisting(MonoObject* managedInstance, 
		const SPtr<ManagedSerializableTypeInfoArray>& typeInfo)
	{
		if(managedInstance == nullptr)
			return nullptr;

		if(!ScriptAssemblyManager::instance().getSystemArrayClass()->isInstanceOfType(managedInstance))
			return nullptr;

		return bs_shared_ptr_new<ManagedSerializableArray>(ConstructPrivately(), typeInfo, managedInstance);
	}

	SPtr<ManagedSerializableArray> ManagedSerializableArray::createNew(const SPtr<ManagedSerializableTypeInfoArray>& typeInfo, const Vector<UINT32>& sizes)
	{
		return bs_shared_ptr_new<ManagedSerializableArray>(ConstructPrivately(), typeInfo, createManagedInstance(typeInfo, sizes));
	}

	SPtr<ManagedSerializableArray> ManagedSerializableArray::createNew()
	{
		return bs_shared_ptr_new<ManagedSerializableArray>(ConstructPrivately());
	}

	MonoObject* ManagedSerializableArray::createManagedInstance(const SPtr<ManagedSerializableTypeInfoArray>& typeInfo, const Vector<UINT32>& sizes)
	{
		if (!typeInfo->isTypeLoaded())
			return nullptr;

		MonoClass* arrayClass = ScriptAssemblyManager::instance().getSystemArrayClass();

		MonoMethod* createInstance = arrayClass->getMethodExact("CreateInstance", "Type,int[]");

		ScriptArray lengthArray(MonoUtil::getINT32Class(), (UINT32)sizes.size());
		for (UINT32 i = 0; i < (UINT32)sizes.size(); i++)
			lengthArray.set(i, sizes[i]);

		void* params[2] = { MonoUtil::getType(typeInfo->mElementType->getMonoClass()), lengthArray.getInternal() };
		return createInstance->invoke(nullptr, params);
	}

	MonoObject* ManagedSerializableArray::getManagedInstance() const
	{
		if(mGCHandle != 0)
			return MonoUtil::getObjectFromGCHandle(mGCHandle);

		return nullptr;
	}

	void ManagedSerializableArray::setFieldData(UINT32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val)
	{
		if (mGCHandle != 0)
		{
			MonoArray* array = (MonoArray*)MonoUtil::getObjectFromGCHandle(mGCHandle);
			setFieldData(array, arrayIdx, val);
		}
		else
		{
			mCachedEntries[arrayIdx] = val;
		}
	}

	void ManagedSerializableArray::setFieldData(MonoArray* obj, UINT32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val)
	{
		if (MonoUtil::isValueType(mElementMonoClass))
			setValueInternal(obj, arrayIdx, val->getValue(mArrayTypeInfo->mElementType));
		else
		{
			MonoObject* ptrToObj = (MonoObject*)val->getValue(mArrayTypeInfo->mElementType);
			setValueInternal(obj, arrayIdx, &ptrToObj);
		}
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableArray::getFieldData(UINT32 arrayIdx)
	{
		if (mGCHandle != 0)
		{
			MonoArray* array = (MonoArray*)MonoUtil::getObjectFromGCHandle(mGCHandle);
			ScriptArray scriptArray(array);

			UINT32 numElems = scriptArray.size();
			assert(arrayIdx < numElems);

			void* arrayValue = scriptArray.getRaw(arrayIdx, mElemSize);

			if (MonoUtil::isValueType(mElementMonoClass))
			{
				MonoObject* boxedObj = nullptr;

				if (arrayValue != nullptr)
					boxedObj = MonoUtil::box(mElementMonoClass, arrayValue);

				return ManagedSerializableFieldData::create(mArrayTypeInfo->mElementType, boxedObj);
			}
			else
				return ManagedSerializableFieldData::create(mArrayTypeInfo->mElementType, *(MonoObject**)arrayValue);
		}
		else
			return mCachedEntries[arrayIdx];
	}

	void ManagedSerializableArray::serialize()
	{
		if(mGCHandle == 0)
			return;

		mNumElements.resize(mArrayTypeInfo->mRank);
		for (UINT32 i = 0; i < mArrayTypeInfo->mRank; i++)
			mNumElements[i] = getLengthInternal(i);

		UINT32 numElements = getTotalLength();
		mCachedEntries = Vector<SPtr<ManagedSerializableFieldData>>(numElements);

		for (UINT32 i = 0; i < numElements; i++)
			mCachedEntries[i] = getFieldData(i);

		// Serialize children
		for (auto& fieldEntry : mCachedEntries)
			fieldEntry->serialize();

		MonoUtil::freeGCHandle(mGCHandle);
		mGCHandle = 0;
	}

	MonoObject* ManagedSerializableArray::deserialize()
	{
		MonoObject* managedInstance = createManagedInstance(mArrayTypeInfo, mNumElements);

		if (managedInstance == nullptr)
			return nullptr;

		ScriptArray scriptArray((MonoArray*)managedInstance);
		mElemSize = scriptArray.elementSize();

		initMonoObjects();

		// Deserialize children
		for (auto& fieldEntry : mCachedEntries)
			fieldEntry->deserialize();

		UINT32 idx = 0;
		for (auto& arrayEntry : mCachedEntries)
		{
			setFieldData((MonoArray*)managedInstance, idx, arrayEntry);
			idx++;
		}

		return managedInstance;
	}
	
	void ManagedSerializableArray::setValueInternal(MonoArray* obj, UINT32 arrayIdx, void* val)
	{
		ScriptArray scriptArray(obj);
		UINT32 numElems = (UINT32)scriptArray.size();
		assert(arrayIdx < numElems);
	
		scriptArray.setRaw(arrayIdx, (UINT8*)val, mElemSize);
	}

	void ManagedSerializableArray::initMonoObjects()
	{
		mElementMonoClass = mArrayTypeInfo->mElementType->getMonoClass();

		MonoClass* arrayClass = ScriptAssemblyManager::instance().getSystemArrayClass();
		mCopyMethod = arrayClass->getMethodExact("Copy", "Array,Array,int");
	}

	UINT32 ManagedSerializableArray::toSequentialIdx(const Vector<UINT32>& idx) const
	{
		UINT32 mNumDims = (UINT32)mNumElements.size();

		if(idx.size() != mNumDims)
			BS_EXCEPT(InvalidParametersException, "Provided index doesn't have the correct number of dimensions");

		if(mNumElements.size() == 0)
			return 0;

		UINT32 curIdx = 0;
		UINT32 prevDimensionSize = 1;
		
		for(INT32 i = mNumDims - 1; i >= 0; i--)
		{
			curIdx += idx[i] * prevDimensionSize;

			prevDimensionSize *= mNumElements[i];
		}

		return curIdx;
	}

	void ManagedSerializableArray::resize(const Vector<UINT32>& newSizes)
	{
		if (mGCHandle != 0)
		{
			assert(mArrayTypeInfo->mRank == (UINT32)newSizes.size());

			UINT32 srcCount = 1;
			for (auto& numElems : mNumElements)
				srcCount *= numElems;

			UINT32 dstCount = 1;
			for (auto& numElems : newSizes)
				dstCount *= numElems;

			UINT32 copyCount = std::min(srcCount, dstCount);

			MonoObject* newArray = createManagedInstance(mArrayTypeInfo, newSizes);

			void* params[3];
			params[0] = getManagedInstance();
			params[1] = newArray;
			params[2] = &copyCount;

			mCopyMethod->invoke(nullptr, params);

			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = MonoUtil::newGCHandle(newArray, false);

			mNumElements = newSizes;
		}
		else
		{
			mNumElements = newSizes;
			mCachedEntries.resize(getTotalLength());
		}
	}

	UINT32 ManagedSerializableArray::getLengthInternal(UINT32 dimension) const
	{
		MonoObject* managedInstace = MonoUtil::getObjectFromGCHandle(mGCHandle);

		MonoClass* systemArray = ScriptAssemblyManager::instance().getSystemArrayClass();
		MonoMethod* getLength = systemArray->getMethod("GetLength", 1);

		void* params[1] = { &dimension };
		MonoObject* returnObj = getLength->invoke(managedInstace, params);

		return *(UINT32*)MonoUtil::unbox(returnObj);
	}

	UINT32 ManagedSerializableArray::getTotalLength() const
	{
		UINT32 totalNumElements = 1;
		for (auto& numElems : mNumElements)
			totalNumElements *= numElems;

		return totalNumElements;
	}

	RTTITypeBase* ManagedSerializableArray::getRTTIStatic()
	{
		return ManagedSerializableArrayRTTI::instance();
	}

	RTTITypeBase* ManagedSerializableArray::getRTTI() const
	{
		return ManagedSerializableArray::getRTTIStatic();
	}
}