//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Serialization/B3DManagedSerializableObject.h"
#include "RTTI/B3DManagedSerializableObjectRTTI.h"
#include "Serialization/B3DManagedTypeInfo.h"
#include "Serialization/B3DManagedSerializableField.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "B3DMonoField.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

using namespace b3d;
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

ManagedSerializableObject::ManagedSerializableObject(const ConstructPrivately& dummy, SPtr<ManagedObjectInfo> objInfo, MonoObject* managedInstance)
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

	SPtr<ManagedObjectInfo> objInfo;
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(elementNs, elementTypeName, objInfo))
		return nullptr;

	return B3DMakeShared<ManagedSerializableObject>(ConstructPrivately(), objInfo, managedInstance);
}

SPtr<ManagedSerializableObject> ManagedSerializableObject::CreateNew(const SPtr<ManagedTypeInfoObject>& type)
{
	SPtr<ManagedObjectInfo> currentObjInfo = nullptr;

	// See if this type even still exists
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(type->TypeNamespace, type->TypeName, currentObjInfo))
		return nullptr;

	return B3DMakeShared<ManagedSerializableObject>(ConstructPrivately(), currentObjInfo, CreateManagedInstance(type));
}

MonoObject* ManagedSerializableObject::CreateManagedInstance(const SPtr<ManagedTypeInfoObject>& type)
{
	SPtr<ManagedObjectInfo> currentObjInfo = nullptr;

	// See if this type even still exists
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(type->TypeNamespace, type->TypeName, currentObjInfo))
		return nullptr;

	if(!currentObjInfo->TypeInfo->MetaDataFlags.IsSet(ManagedObjectMetaDataFlag::Serializable))
		return nullptr;

	const bool construct = currentObjInfo->ScriptClass->GetMethod(".ctor", 0) != nullptr;
	return currentObjInfo->ScriptClass->CreateInstance(construct);
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

	SPtr<ManagedObjectInfo> curType = mObjInfo;
	while(curType != nullptr)
	{
		for(auto& memberInfo : curType->Members)
		{
			if(memberInfo->IsSerializable())
			{
				ManagedSerializableFieldKey key(memberInfo->ParentTypeId, memberInfo->FieldId);
				mCachedData[key] = GetFieldData(memberInfo);
			}
		}

		curType = curType->BaseClass;
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
	SPtr<ManagedObjectInfo> currentObjInfo = nullptr;
	if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(mObjInfo->TypeInfo->TypeNamespace, mObjInfo->TypeInfo->TypeName, currentObjInfo))
	{
		return nullptr;
	}

	MonoObject* managedInstance = CreateManagedInstance(currentObjInfo->TypeInfo);
	Deserialize(managedInstance, currentObjInfo);

	return managedInstance;
}

void ManagedSerializableObject::Deserialize(MonoObject* instance, const SPtr<ManagedObjectInfo>& objInfo)
{
	if(instance == nullptr)
		return;

	// Deserialize children
	for(auto& fieldEntry : mCachedData)
		fieldEntry.second->Deserialize();

	// Scan all fields and ensure the fields still exist
	u32 i = 0;
	SPtr<ManagedObjectInfo> curType = mObjInfo;
	while(curType != nullptr)
	{
		for(auto& member : curType->Members)
		{
			if(member->IsSerializable())
			{
				u32 fieldId = member->FieldId;
				u32 typeID = member->ParentTypeId;

				ManagedSerializableFieldKey key(typeID, fieldId);

				SPtr<ManagedMemberInfo> matchingFieldInfo = objInfo->FindMatchingField(member, curType->TypeInfo);
				if(matchingFieldInfo != nullptr)
					matchingFieldInfo->SetUnboxedValue(instance, mCachedData[key]->GetValue(matchingFieldInfo->TypeInfo));

				i++;
			}
		}

		curType = curType->BaseClass;
	}
}

bool ManagedSerializableObject::Equals(ManagedSerializableObject& other, RTTIOperationContext* context)
{
	SPtr<ManagedObjectInfo> otherObjInfo = other.GetObjectInfo();

	if(!mObjInfo->TypeInfo->Matches(otherObjInfo->TypeInfo))
		return false;

	SPtr<ManagedObjectInfo> curObjInfo = mObjInfo;
	while(curObjInfo != nullptr)
	{
		for(auto& member : curObjInfo->Members)
		{
			if(!member->IsSerializable())
				continue;

			SPtr<ManagedSerializableFieldData> oldData = GetFieldData(member);
			SPtr<ManagedSerializableFieldData> newData = other.GetFieldData(member);

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

		curObjInfo = curObjInfo->BaseClass;
	}

	return true;
}

void ManagedSerializableObject::SetFieldData(const SPtr<ManagedMemberInfo>& fieldInfo, const SPtr<ManagedSerializableFieldData>& val)
{
	if(mGCHandle != 0)
	{
		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
		fieldInfo->SetUnboxedValue(managedInstance, val->GetValue(fieldInfo->TypeInfo));
	}
	else
	{
		ManagedSerializableFieldKey key(fieldInfo->ParentTypeId, fieldInfo->FieldId);
		mCachedData[key] = val;
	}
}

SPtr<ManagedSerializableFieldData> ManagedSerializableObject::GetFieldData(const SPtr<ManagedMemberInfo>& fieldInfo) const
{
	if(mGCHandle != 0)
	{
		MonoObject* managedInstance = MonoUtil::GetObjectFromGcHandle(mGCHandle);
		MonoObject* fieldValue = fieldInfo->GetValue(managedInstance);

		return ManagedSerializableFieldData::Create(fieldInfo->TypeInfo, fieldValue);
	}
	else
	{
		ManagedSerializableFieldKey key(fieldInfo->ParentTypeId, fieldInfo->FieldId);
		auto iterFind = mCachedData.find(key);

		if(iterFind != mCachedData.end())
			return iterFind->second;

		return nullptr;
	}
}

RTTIType* ManagedSerializableObject::GetRttiStatic()
{
	return ManagedSerializableObjectRTTI::Instance();
}

RTTIType* ManagedSerializableObject::GetRtti() const
{
	return ManagedSerializableObject::GetRttiStatic();
}
