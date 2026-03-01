//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DManagedSerializableArray.h"
#include "RTTI/B3DManagedSerializableArrayRTTI.h"
#include "B3DMonoManager.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Serialization/B3DManagedSerializableField.h"
#include "B3DMonoClass.h"
#include "B3DMonoMethod.h"
#include "B3DMonoArray.h"

using namespace b3d;
ManagedSerializableArray::ManagedSerializableArray(const ConstructPrivately& dummy)
{
}

ManagedSerializableArray::ManagedSerializableArray(const ConstructPrivately& dummy, const SPtr<ManagedTypeInfoArray>& typeInfo, MonoObject* managedInstance)
	: mArrayTypeInfo(typeInfo)

{
	mGCHandle = MonoUtil::NewGcHandle(managedInstance, false);

	ScriptArray scriptArray((MonoArray*)managedInstance);
	mElemSize = scriptArray.ElementSize();

	InitMonoObjects();

	mNumElements.resize(typeInfo->Rank);
	for(u32 i = 0; i < typeInfo->Rank; i++)
		mNumElements[i] = GetLengthInternal(i);
}

ManagedSerializableArray::~ManagedSerializableArray()
{
	if(mGCHandle != 0)
	{
		MonoUtil::FreeGcHandle(mGCHandle);
		mGCHandle = 0;
	}
}

SPtr<ManagedSerializableArray> ManagedSerializableArray::CreateFromExisting(MonoObject* managedInstance, const SPtr<ManagedTypeInfoArray>& typeInfo)
{
	if(managedInstance == nullptr)
		return nullptr;

	if(!ScriptAssemblyManager::Instance().GetBuiltinClasses().SystemArrayClass->IsInstanceOfType(managedInstance))
		return nullptr;

	return B3DMakeShared<ManagedSerializableArray>(ConstructPrivately(), typeInfo, managedInstance);
}

SPtr<ManagedSerializableArray> ManagedSerializableArray::CreateNew(const SPtr<ManagedTypeInfoArray>& typeInfo, const Vector<u32>& sizes)
{
	return B3DMakeShared<ManagedSerializableArray>(ConstructPrivately(), typeInfo, CreateManagedInstance(typeInfo, sizes));
}

SPtr<ManagedSerializableArray> ManagedSerializableArray::CreateNew()
{
	return B3DMakeShared<ManagedSerializableArray>(ConstructPrivately());
}

MonoObject* ManagedSerializableArray::CreateManagedInstance(const SPtr<ManagedTypeInfoArray>& typeInfo, const Vector<u32>& sizes)
{
	if(!typeInfo->IsTypeLoaded())
		return nullptr;

	MonoClass* arrayClass = ScriptAssemblyManager::Instance().GetBuiltinClasses().SystemArrayClass;

	MonoMethod* createInstance = arrayClass->GetMethodExact("CreateInstance", "Type,int[]");

	ScriptArray lengthArray(MonoUtil::GetInt32Class(), (u32)sizes.size());
	for(u32 i = 0; i < (u32)sizes.size(); i++)
		lengthArray.Set(i, sizes[i]);

	void* params[2] = { MonoUtil::GetType(typeInfo->ElementType->GetMonoClass()), lengthArray.GetInternal() };
	return createInstance->Invoke(nullptr, params);
}

MonoObject* ManagedSerializableArray::GetManagedInstance() const
{
	if(mGCHandle != 0)
		return MonoUtil::GetObjectFromGcHandle(mGCHandle);

	return nullptr;
}

void ManagedSerializableArray::SetFieldData(u32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val)
{
	if(mGCHandle != 0)
	{
		MonoArray* array = (MonoArray*)MonoUtil::GetObjectFromGcHandle(mGCHandle);
		SetFieldData(array, arrayIdx, val);
	}
	else
	{
		mCachedEntries[arrayIdx] = val;
	}
}

void ManagedSerializableArray::SetFieldData(MonoArray* obj, u32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val)
{
	if(MonoUtil::IsValueType(mElementMonoClass))
		SetValueInternal(obj, arrayIdx, val->GetValue(mArrayTypeInfo->ElementType));
	else
	{
		MonoObject* ptrToObj = (MonoObject*)val->GetValue(mArrayTypeInfo->ElementType);
		SetValueInternal(obj, arrayIdx, &ptrToObj);
	}
}

SPtr<ManagedSerializableFieldData> ManagedSerializableArray::GetFieldData(u32 arrayIdx)
{
	if(mGCHandle != 0)
	{
		MonoArray* array = (MonoArray*)MonoUtil::GetObjectFromGcHandle(mGCHandle);
		ScriptArray scriptArray(array);

		u32 numElems = scriptArray.Size();
		B3D_ASSERT(arrayIdx < numElems);

		void* arrayValue = scriptArray.GetRaw(arrayIdx, mElemSize);

		if(MonoUtil::IsValueType(mElementMonoClass))
		{
			MonoObject* boxedObj = nullptr;

			if(arrayValue != nullptr)
				boxedObj = MonoUtil::Box(mElementMonoClass, arrayValue);

			return ManagedSerializableFieldData::Create(mArrayTypeInfo->ElementType, boxedObj);
		}
		else
			return ManagedSerializableFieldData::Create(mArrayTypeInfo->ElementType, *(MonoObject**)arrayValue);
	}
	else
		return mCachedEntries[arrayIdx];
}

void ManagedSerializableArray::Serialize()
{
	if(mGCHandle == 0)
		return;

	mNumElements.resize(mArrayTypeInfo->Rank);
	for(u32 i = 0; i < mArrayTypeInfo->Rank; i++)
		mNumElements[i] = GetLengthInternal(i);

	u32 numElements = GetTotalLength();
	mCachedEntries = Vector<SPtr<ManagedSerializableFieldData>>(numElements);

	for(u32 i = 0; i < numElements; i++)
		mCachedEntries[i] = GetFieldData(i);

	// Serialize children
	for(auto& fieldEntry : mCachedEntries)
		fieldEntry->Serialize();

	MonoUtil::FreeGcHandle(mGCHandle);
	mGCHandle = 0;
}

MonoObject* ManagedSerializableArray::Deserialize()
{
	MonoObject* managedInstance = CreateManagedInstance(mArrayTypeInfo, mNumElements);

	if(managedInstance == nullptr)
		return nullptr;

	ScriptArray scriptArray((MonoArray*)managedInstance);
	mElemSize = scriptArray.ElementSize();

	InitMonoObjects();

	// Deserialize children
	for(auto& fieldEntry : mCachedEntries)
		fieldEntry->Deserialize();

	u32 idx = 0;
	for(auto& arrayEntry : mCachedEntries)
	{
		SetFieldData((MonoArray*)managedInstance, idx, arrayEntry);
		idx++;
	}

	return managedInstance;
}

void ManagedSerializableArray::SetValueInternal(MonoArray* obj, u32 arrayIdx, void* val)
{
	ScriptArray scriptArray(obj);
	u32 numElems = (u32)scriptArray.Size();
	B3D_ASSERT(arrayIdx < numElems);

	scriptArray.SetRaw(arrayIdx, (u8*)val, mElemSize);
}

void ManagedSerializableArray::InitMonoObjects()
{
	mElementMonoClass = mArrayTypeInfo->ElementType->GetMonoClass();

	MonoClass* arrayClass = ScriptAssemblyManager::Instance().GetBuiltinClasses().SystemArrayClass;
	mCopyMethod = arrayClass->GetMethodExact("Copy", "Array,Array,int");
}

u32 ManagedSerializableArray::ToSequentialIdx(const Vector<u32>& idx) const
{
	u32 mNumDims = (u32)mNumElements.size();

	if(!B3D_ENSURE_LOG(idx.size() == mNumDims, "Provided index doesn't have the correct number of dimensions"))
		return 0;

	if(mNumElements.size() == 0)
		return 0;

	u32 curIdx = 0;
	u32 prevDimensionSize = 1;

	for(i32 i = mNumDims - 1; i >= 0; i--)
	{
		curIdx += idx[i] * prevDimensionSize;

		prevDimensionSize *= mNumElements[i];
	}

	return curIdx;
}

void ManagedSerializableArray::Resize(const Vector<u32>& newSizes)
{
	if(mGCHandle != 0)
	{
		B3D_ASSERT(mArrayTypeInfo->Rank == (u32)newSizes.size());

		u32 srcCount = 1;
		for(auto& numElems : mNumElements)
			srcCount *= numElems;

		u32 dstCount = 1;
		for(auto& numElems : newSizes)
			dstCount *= numElems;

		u32 copyCount = std::min(srcCount, dstCount);

		MonoObject* newArray = CreateManagedInstance(mArrayTypeInfo, newSizes);

		void* params[3];
		params[0] = GetManagedInstance();
		params[1] = newArray;
		params[2] = &copyCount;

		mCopyMethod->Invoke(nullptr, params);

		MonoUtil::FreeGcHandle(mGCHandle);
		mGCHandle = MonoUtil::NewGcHandle(newArray, false);

		mNumElements = newSizes;
	}
	else
	{
		mNumElements = newSizes;
		mCachedEntries.resize(GetTotalLength());
	}
}

u32 ManagedSerializableArray::GetLengthInternal(u32 dimension) const
{
	MonoObject* managedInstace = MonoUtil::GetObjectFromGcHandle(mGCHandle);

	MonoClass* systemArray = ScriptAssemblyManager::Instance().GetBuiltinClasses().SystemArrayClass;
	MonoMethod* getLength = systemArray->GetMethod("GetLength", 1);

	void* params[1] = { &dimension };
	MonoObject* returnObj = getLength->Invoke(managedInstace, params);

	return *(u32*)MonoUtil::Unbox(returnObj);
}

u32 ManagedSerializableArray::GetTotalLength() const
{
	u32 totalNumElements = 1;
	for(auto& numElems : mNumElements)
		totalNumElements *= numElems;

	return totalNumElements;
}

RTTIType* ManagedSerializableArray::GetRttiStatic()
{
	return ManagedSerializableArrayRTTI::Instance();
}

RTTIType* ManagedSerializableArray::GetRtti() const
{
	return ManagedSerializableArray::GetRttiStatic();
}
