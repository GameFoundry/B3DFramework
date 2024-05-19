//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/BsManagedSerializableObject.h"
#include "RTTI/BsManagedSerializableObjectRTTI.h"
#include "Serialization/BsManagedSerializableObjectInfo.h"
#include "Serialization/BsManagedSerializableField.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

using namespace bs;
size_t ManagedSerializableObject::Hash::operator()(const ManagedSerializableFieldKey& x) const
{
	size_t seed = 0;
	B3DCombineHash(seed, (u32)x.MFieldId);
	B3DCombineHash(seed, (u32)x.MTypeId);

	return seed;
}

bool ManagedSerializableObject::Equals::operator()(const ManagedSerializableFieldKey& a, const ManagedSerializableFieldKey& b) const
{
	return a.MFieldId == b.MFieldId && a.MTypeId == b.MTypeId;
}

ManagedSerializableObject::ManagedSerializableObject(const ConstructPrivately& dummy)
{
}

ManagedSerializableObject::ManagedSerializableObject(const ConstructPrivately& dummy, SPtr<ManagedSerializableObjectInfo> objInfo, MonoObject* managedInstance)
	: mObjInfo(objInfo)
{
	mGCHandle = MonoUtil::NewGcHandle(managedInstance, false);
}

ManagedSerializableObject::~ManagedSerializableObject()
{
	if(mGCHandle != 0)
	{
		MonoUtil::FreeGcHandle(mGCHandle);
		mGCHandle = 0;
	}
}

SPtr<ManagedSerializableObject> ManagedSerializableObject::CreateFromExisting(MonoObject* managedInstance)
{
	if(managedInstance == nullptr)
		return nullptr;

	String elementNs;
	String elementTypeName;
	MonoUtil::GetClassName(managedInstance, elementNs, elementTypeName);

	SPtr<ManagedSerializableObjectInfo> objInfo;
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(elementNs, elementTypeName, objInfo))
		return nullptr;

	return B3DMakeShared<ManagedSerializableObject>(ConstructPrivately(), objInfo, managedInstance);
}

SPtr<ManagedSerializableObject> ManagedSerializableObject::CreateNew(const SPtr<ManagedSerializableTypeInfoObject>& type)
{
	SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

	// See if this type even still exists
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(type->MTypeNamespace, type->MTypeName, currentObjInfo))
		return nullptr;

	return B3DMakeShared<ManagedSerializableObject>(ConstructPrivately(), currentObjInfo, CreateManagedInstance(type));
}

MonoObject* ManagedSerializableObject::CreateManagedInstance(const SPtr<ManagedSerializableTypeInfoObject>& type)
{
	SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

	// See if this type even still exists
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(type->MTypeNamespace, type->MTypeName, currentObjInfo))
		return nullptr;

	if(!currentObjInfo->MTypeInfo->MFlags.IsSet(ScriptTypeFlag::Serializable))
		return nullptr;

	const bool construct = currentObjInfo->MMonoClass->GetMethod(".ctor", 0) != nullptr;
	return currentObjInfo->MMonoClass->CreateInstance(construct);
}

SPtr<ManagedSerializableObject> ManagedSerializableObject::CreateEmpty()
{
	return B3DMakeShared<ManagedSerializableObject>(ConstructPrivately());
}

MonoObject* ManagedSerializableObject::GetManagedInstance() const
{
	if(mGCHandle != 0)
		return MonoUtil::GetObjectFromGcHandle(mGCHandle);

	return nullptr;
}

void ManagedSerializableObject::Serialize()
{
	if(mGCHandle == 0)
		return;

	mCachedData.clear();

	SPtr<ManagedSerializableObjectInfo> curType = mObjInfo;
	while(curType != nullptr)
	{
		for(auto& field : curType->MFields)
		{
			if(field.second->IsSerializable())
			{
				ManagedSerializableFieldKey key(field.second->MParentTypeId, field.second->MFieldId);
				mCachedData[key] = GetFieldData(field.second);
			}
		}

		curType = curType->MBaseClass;
	}

	// Serialize children
	for(auto& fieldEntry : mCachedData)
		fieldEntry.second->Serialize();

	MonoUtil::FreeGcHandle(mGCHandle);
	mGCHandle = 0;
}

MonoObject* ManagedSerializableObject::Deserialize()
{
	// See if this type even still exists
	SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(mObjInfo->MTypeInfo->MTypeNamespace, mObjInfo->MTypeInfo->MTypeName, currentObjInfo))
	{
		return nullptr;
	}

	MonoObject* managedInstance = CreateManagedInstance(currentObjInfo->MTypeInfo);
	Deserialize(managedInstance, currentObjInfo);

	return managedInstance;
}

void ManagedSerializableObject::Deserialize(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo)
{
	if(instance == nullptr)
		return;

	// Deserialize children
	for(auto& fieldEntry : mCachedData)
		fieldEntry.second->Deserialize();

	// Scan all fields and ensure the fields still exist
	u32 i = 0;
	SPtr<ManagedSerializableObjectInfo> curType = mObjInfo;
	while(curType != nullptr)
	{
		for(auto& field : curType->MFields)
		{
			if(field.second->IsSerializable())
			{
				u32 fieldId = field.second->MFieldId;
				u32 typeID = field.second->MParentTypeId;

				ManagedSerializableFieldKey key(typeID, fieldId);

				SPtr<ManagedSerializableMemberInfo> matchingFieldInfo = objInfo->FindMatchingField(field.second, curType->MTypeInfo);
				if(matchingFieldInfo != nullptr)
					matchingFieldInfo->SetValue(instance, mCachedData[key]->GetValue(matchingFieldInfo->MTypeInfo));

				i++;
			}
		}

		curType = curType->MBaseClass;
	}
}

bool ManagedSerializableObject::Equals(ManagedSerializableObject& other, RTTIOperationContext* context)
{
	SPtr<ManagedSerializableObjectInfo> otherObjInfo = other.GetObjectInfo();

	if(!mObjInfo->MTypeInfo->Matches(otherObjInfo->MTypeInfo))
		return false;

	SPtr<ManagedSerializableObjectInfo> curObjInfo = mObjInfo;
	while(curObjInfo != nullptr)
	{
		for(auto& field : curObjInfo->MFields)
		{
			if(!field.second->IsSerializable())
				continue;

			SPtr<ManagedSerializableFieldData> oldData = GetFieldData(field.second);
			SPtr<ManagedSerializableFieldData> newData = other.GetFieldData(field.second);

			if(!oldData)
				return !newData;
			else
			{
				if(!newData)
					return false;
			}

			if(!oldData->Equals(newData, context))
				return false;
		}

		curObjInfo = curObjInfo->MBaseClass;
	}

	return true;
}

void ManagedSerializableObject::SetFieldData(const SPtr<ManagedSerializableMemberInfo>& fieldInfo, const SPtr<ManagedSerializableFieldData>& val)
{
	if(mGCHandle != 0)
	{
		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
		fieldInfo->SetValue(managedInstance, val->GetValue(fieldInfo->MTypeInfo));
	}
	else
	{
		ManagedSerializableFieldKey key(fieldInfo->MParentTypeId, fieldInfo->MFieldId);
		mCachedData[key] = val;
	}
}

SPtr<ManagedSerializableFieldData> ManagedSerializableObject::GetFieldData(const SPtr<ManagedSerializableMemberInfo>& fieldInfo) const
{
	if(mGCHandle != 0)
	{
		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
		MonoObject* fieldValue = fieldInfo->GetValue(managedInstance);

		return ManagedSerializableFieldData::Create(fieldInfo->MTypeInfo, fieldValue);
	}
	else
	{
		ManagedSerializableFieldKey key(fieldInfo->MParentTypeId, fieldInfo->MFieldId);
		auto iterFind = mCachedData.find(key);

		if(iterFind != mCachedData.end())
			return iterFind->second;

		return nullptr;
	}
}

RTTITypeBase* ManagedSerializableObject::GetRttiStatic()
{
	return ManagedSerializableObjectRTTI::Instance();
}

RTTITypeBase* ManagedSerializableObject::GetRtti() const
{
	return ManagedSerializableObject::GetRttiStatic();
}
