//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DManagedSerializableList.h"
#include "RTTI/B3DManagedSerializableListRTTI.h"
#include "B3DMonoManager.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "Serialization/B3DManagedSerializableField.h"
#include "B3DMonoClass.h"
#include "B3DMonoMethod.h"
#include "B3DMonoProperty.h"
#include "B3DMonoUtil.h"

using namespace b3d;
ManagedSerializableList::ManagedSerializableList(const ConstructPrivately& dummy)
{
}

ManagedSerializableList::ManagedSerializableList(const ConstructPrivately& dummy, const SPtr<ManagedTypeInfoList>& typeInfo, MonoObject* managedInstance)
	: mListTypeInfo(typeInfo)
{
	mGCHandle = MonoUtil::NewGcHandle(managedInstance, false);

	MonoClass* listClass = MonoManager::Instance().FindClass(MonoUtil::GetClass(managedInstance));
	if(listClass == nullptr)
		return;

	InitMonoObjects(listClass);

	mNumElements = GetLengthInternal();
}

ManagedSerializableList::~ManagedSerializableList()
{
	if(mGCHandle != 0)
	{
		MonoUtil::FreeGcHandle(mGCHandle);
		mGCHandle = 0;
	}
}

SPtr<ManagedSerializableList> ManagedSerializableList::CreateFromExisting(MonoObject* managedInstance, const SPtr<ManagedTypeInfoList>& typeInfo)
{
	if(managedInstance == nullptr)
		return nullptr;

	String elementNs;
	String elementTypeName;
	MonoUtil::GetClassName(managedInstance, elementNs, elementTypeName);

	String fullName = elementNs + "." + elementTypeName;

	if(ScriptAssemblyManager::Instance().GetBuiltinClasses().SystemGenericListClass->GetFullName() != fullName)
		return nullptr;

	return B3DMakeShared<ManagedSerializableList>(ConstructPrivately(), typeInfo, managedInstance);
}

SPtr<ManagedSerializableList> ManagedSerializableList::CreateNew(const SPtr<ManagedTypeInfoList>& typeInfo, u32 size)
{
	return B3DMakeShared<ManagedSerializableList>(ConstructPrivately(), typeInfo, CreateManagedInstance(typeInfo, size));
}

MonoObject* ManagedSerializableList::CreateManagedInstance(const SPtr<ManagedTypeInfoList>& typeInfo, u32 size)
{
	if(!typeInfo->IsTypeLoaded())
		return nullptr;

	::MonoClass* listMonoClass = typeInfo->GetMonoClass();
	MonoClass* listClass = MonoManager::Instance().FindClass(listMonoClass);
	if(listClass == nullptr)
		return nullptr;

	void* params[1] = { &size };
	MonoObject* instance = listClass->CreateInstance("int", params);

	ScriptArray tempArray(typeInfo->ElementType->GetMonoClass(), size);
	params[0] = tempArray.GetInternal();

	MonoMethod* addRangeMethod = listClass->GetMethod("AddRange", 1);
	addRangeMethod->Invoke(instance, params);

	return instance;
}

SPtr<ManagedSerializableList> ManagedSerializableList::CreateEmpty()
{
	return B3DMakeShared<ManagedSerializableList>(ConstructPrivately());
}

MonoObject* ManagedSerializableList::GetManagedInstance() const
{
	if(mGCHandle != 0)
		return MonoUtil::GetObjectFromGcHandle(mGCHandle);

	return nullptr;
}

void ManagedSerializableList::SetFieldData(u32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val)
{
	if(mGCHandle != 0)
	{
		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
		SetFieldData(managedInstance, arrayIdx, val);
	}
	else
		mCachedEntries[arrayIdx] = val;
}

void ManagedSerializableList::SetFieldData(MonoObject* obj, u32 arrayIdx, const SPtr<ManagedSerializableFieldData>& val)
{
	mItemProp->SetIndexed(obj, arrayIdx, val->GetValue(mListTypeInfo->ElementType));
}

void ManagedSerializableList::AddFieldDataInternal(const SPtr<ManagedSerializableFieldData>& val)
{
	MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);

	void* params[1];
	params[0] = val->GetValue(mListTypeInfo->ElementType);
	mAddMethod->Invoke(managedInstance, params);
}

SPtr<ManagedSerializableFieldData> ManagedSerializableList::GetFieldData(u32 arrayIdx)
{
	if(mGCHandle != 0)
	{
		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
		MonoObject* obj = mItemProp->GetIndexed(managedInstance, arrayIdx);

		return ManagedSerializableFieldData::Create(mListTypeInfo->ElementType, obj);
	}
	else
		return mCachedEntries[arrayIdx];
}

void ManagedSerializableList::Resize(u32 newSize)
{
	if(mGCHandle != 0)
	{
		ScriptArray tempArray(mListTypeInfo->ElementType->GetMonoClass(), newSize);

		u32 minSize = std::min(mNumElements, newSize);
		u32 dummy = 0;

		void* params[4];
		params[0] = &dummy;
		;
		params[1] = tempArray.GetInternal();
		params[2] = &dummy;
		params[3] = &minSize;

		mCopyToMethod->Invoke(GetManagedInstance(), params);
		mClearMethod->Invoke(GetManagedInstance(), nullptr);

		params[0] = tempArray.GetInternal();
		mAddRangeMethod->Invoke(GetManagedInstance(), params);
	}
	else
	{
		mCachedEntries.resize(newSize);
	}

	mNumElements = newSize;
}

void ManagedSerializableList::Serialize()
{
	if(mGCHandle == 0)
		return;

	mNumElements = GetLengthInternal();
	mCachedEntries = Vector<SPtr<ManagedSerializableFieldData>>(mNumElements);

	for(u32 i = 0; i < mNumElements; i++)
		mCachedEntries[i] = GetFieldData(i);

	// Serialize children
	for(auto& fieldEntry : mCachedEntries)
		fieldEntry->Serialize();

	MonoUtil::FreeGcHandle(mGCHandle);
	mGCHandle = 0;
}

MonoObject* ManagedSerializableList::Deserialize()
{
	MonoObject* managedInstance = CreateManagedInstance(mListTypeInfo, mNumElements);

	if(managedInstance == nullptr)
		return nullptr;

	MonoClass* listClass = MonoManager::Instance().FindClass(mListTypeInfo->GetMonoClass());
	InitMonoObjects(listClass);

	// Deserialize children
	for(auto& fieldEntry : mCachedEntries)
		fieldEntry->Deserialize();

	u32 idx = 0;
	for(auto& entry : mCachedEntries)
	{
		SetFieldData(managedInstance, idx, entry);
		idx++;
	}

	return managedInstance;
}

u32 ManagedSerializableList::GetLengthInternal() const
{
	MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
	MonoObject* length = mCountProp->Get(managedInstance);

	if(length == nullptr)
		return 0;

	return *(u32*)MonoUtil::Unbox(length);
}

void ManagedSerializableList::InitMonoObjects(MonoClass* listClass)
{
	mItemProp = listClass->GetProperty("Item");
	mCountProp = listClass->GetProperty("Count");
	mAddMethod = listClass->GetMethod("Add", 1);
	mAddRangeMethod = listClass->GetMethod("AddRange", 1);
	mClearMethod = listClass->GetMethod("Clear");
	mCopyToMethod = listClass->GetMethod("CopyTo", 4);
}

RTTIType* ManagedSerializableList::GetRttiStatic()
{
	return ManagedSerializableListRTTI::Instance();
}

RTTIType* ManagedSerializableList::GetRtti() const
{
	return ManagedSerializableList::GetRttiStatic();
}
