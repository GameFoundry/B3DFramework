//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		: MArrayTypeInfo(typeInfo)
		
	{
		mGCHandle = MonoUtil::newGCHandle(managedInstance, false);

		ScriptArray ScriptArray((MonoArray*)managedInstance);
		mElemSize = scriptArray.ElementSize();

		initMonoObjects();

		mNumElements.Resize(typeInfo->mRank);
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

		if(!ScriptAssemblyManager::instance().GetBuiltinClasses().systemArrayClass->IsInstanceOfType(managedInstance))
			return nullptr;

		return bs_shared_ptr_new<ManagedSerializableArray>(ConstructPrivately(), typeInfo, managedInstance);
	}

	SPtr<ManagedSerializableArray> ManagedSerializableArray::CreateNew(const SPtr<ManagedSerializableTypeInfoArray>& typeInfo, const Vector<UINT32>& sizes)
	{
		return bs_shared_ptr_new<ManagedSerializableArray>(ConstructPrivately(), typeInfo, createManagedInstance(typeInfo, sizes));
	}

	SPtr<ManagedSerializableArray> ManagedSerializableArray::CreateNew()
	{
		return bs_shared_ptr_new<ManagedSerializableArray>(ConstructPrivately());
	}

	MonoObject* ManagedSerializableArray::createManagedInstance(const SPtr<ManagedSerializableTypeInfoArray>& typeInfo, const Vector<UINT32>& sizes)
	{
		if (!typeInfo->IsTypeLoaded())
			return nullptr;

		MonoClass* arrayClass = ScriptAssemblyManager::instance().GetBuiltinClasses().systemArrayClass;

		MonoMethod* createInstance = arrayClass->GetMethodExact("CreateInstance", "Type,int[]");

		ScriptArray LengthArray(MonoUtil::getINT32Class(), (UINT32)sizes.Size());
		for (UINT32 i = 0; i < (UINT32)sizes.Size(); i++)
			lengthArray.Set(i, sizes[i]);

		void* params[2] = { MonoUtil::getType(typeInfo->mElementType->GetMonoClass()), lengthArray.GetInternal() };
		return createInstance->Invoke(nullptr, params);
	}

	MonoObject* ManagedSerializableArray::getManagedInstance() const
	{
		if(mGCHandle != 0)
			return MonoUtil::GetObjectFromGCHandle(mGCHandle);

		return nullptr;
	}

	void ManagedSerializableArray::SetFieldData(UINT32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val)
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

	void ManagedSerializableArray::SetFieldData(MonoArray* obj, UINT32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val)
	{
		if (MonoUtil::isValueType(mElementMonoClass))
			setValueInternal(obj, arrayIdx, val->GetValue(mArrayTypeInfo->mElementType));
		else
		{
			MonoObject* ptrToObj = (MonoObject*)val->GetValue(mArrayTypeInfo->mElementType);
			setValueInternal(obj, arrayIdx, &ptrToObj);
		}
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableArray::GetFieldData(UINT32 arrayIdx)
	{
		if (mGCHandle != 0)
		{
			MonoArray* array = (MonoArray*)MonoUtil::getObjectFromGCHandle(mGCHandle);
			ScriptArray ScriptArray(array);

			UINT32 numElems = scriptArray.Size();
			assert(arrayIdx < numElems);

			void* arrayValue = scriptArray.GetRaw(arrayIdx, mElemSize);

			if (MonoUtil::isValueType(mElementMonoClass))
			{
				MonoObject* boxedObj = nullptr;

				if (arrayValue != nullptr)
					boxedObj = MonoUtil::box(mElementMonoClass, arrayValue);

				return ManagedSerializableFieldData::Create(mArrayTypeInfo->mElementType, boxedObj);
			}
			else
				return ManagedSerializableFieldData::Create(mArrayTypeInfo->mElementType, *(MonoObject**)arrayValue);
		}
		else
			return mCachedEntries[arrayIdx];
	}

	void ManagedSerializableArray::Serialize()
	{
		if(mGCHandle == 0)
			return;

		mNumElements.Resize(mArrayTypeInfo->mRank);
		for (UINT32 i = 0; i < mArrayTypeInfo->mRank; i++)
			mNumElements[i] = getLengthInternal(i);

		UINT32 numElements = getTotalLength();
		mCachedEntries = Vector<SPtr<ManagedSerializableFieldData>>(numElements);

		for (UINT32 i = 0; i < numElements; i++)
			mCachedEntries[i] = getFieldData(i);

		// Serialize children
		for (auto& fieldEntry : mCachedEntries)
			fieldEntry->Serialize();

		MonoUtil::freeGCHandle(mGCHandle);
		mGCHandle = 0;
	}

	MonoObject* ManagedSerializableArray::deserialize()
	{
		MonoObject* managedInstance = createManagedInstance(mArrayTypeInfo, mNumElements);

		if (managedInstance == nullptr)
			return nullptr;

		ScriptArray ScriptArray((MonoArray*)managedInstance);
		mElemSize = scriptArray.ElementSize();

		initMonoObjects();

		// Deserialize children
		for (auto& fieldEntry : mCachedEntries)
			fieldEntry->Deserialize();

		UINT32 idx = 0;
		for (auto& arrayEntry : mCachedEntries)
		{
			setFieldData((MonoArray*)managedInstance, idx, arrayEntry);
			idx++;
		}

		return managedInstance;
	}
	
	void ManagedSerializableArray::SetValueInternal(MonoArray* obj, UINT32 arrayIdx, void* val)
	{
		ScriptArray ScriptArray(obj);
		UINT32 numElems = (UINT32)scriptArray.Size();
		assert(arrayIdx < numElems);
	
		scriptArray.SetRaw(arrayIdx, (UINT8*)val, mElemSize);
	}

	void ManagedSerializableArray::InitMonoObjects()
	{
		mElementMonoClass = mArrayTypeInfo->mElementType->GetMonoClass();

		MonoClass* arrayClass = ScriptAssemblyManager::instance().GetBuiltinClasses().systemArrayClass;
		mCopyMethod = arrayClass->GetMethodExact("Copy", "Array,Array,int");
	}

	UINT32 ManagedSerializableArray::ToSequentialIdx(const Vector<UINT32>& idx) const
	{
		UINT32 mNumDims = (UINT32)mNumElements.Size();

		if(idx.Size() != mNumDims)
			BS_EXCEPT(InvalidParametersException, "Provided index doesn't have the correct number of dimensions");

		if(mNumElements.Size() == 0)
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

	void ManagedSerializableArray::Resize(const Vector<UINT32>& newSizes)
	{
		if (mGCHandle != 0)
		{
			assert(mArrayTypeInfo->mRank == (UINT32)newSizes.Size());

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

			mCopyMethod->Invoke(nullptr, params);

			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = MonoUtil::newGCHandle(newArray, false);

			mNumElements = newSizes;
		}
		else
		{
			mNumElements = newSizes;
			mCachedEntries.Resize(getTotalLength());
		}
	}

	UINT32 ManagedSerializableArray::GetLengthInternal(UINT32 dimension) const
	{
		MonoObject* managedInstace = MonoUtil::getObjectFromGCHandle(mGCHandle);

		MonoClass* systemArray = ScriptAssemblyManager::instance().GetBuiltinClasses().systemArrayClass;
		MonoMethod* getLength = systemArray->GetMethod("GetLength", 1);

		void* params[1] = { &dimension };
		MonoObject* returnObj = getLength->Invoke(managedInstace, params);

		return *(UINT32*)MonoUtil::unbox(returnObj);
	}

	UINT32 ManagedSerializableArray::GetTotalLength() const
	{
		UINT32 totalNumElements = 1;
		for (auto& numElems : mNumElements)
			totalNumElements *= numElems;

		return totalNumElements;
	}

	RTTITypeBase* ManagedSerializableArray::getRTTIStatic()
	{
		return ManagedSerializableArrayRTTI::Instance();
	}

	RTTITypeBase* ManagedSerializableArray::getRTTI() const
	{
		return ManagedSerializableArray::GetRTTIStatic();
	}
}
