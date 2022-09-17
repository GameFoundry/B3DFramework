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

namespace bs
{
	size_t ManagedSerializableObject::Hash::operator()(const ManagedSerializableFieldKey& x) const
	{
		size_t seed = 0;
		bs_hash_combine(seed, (UINT32)x.mFieldId);
		bs_hash_combine(seed, (UINT32)x.mTypeId);

		return seed;
	}

	bool ManagedSerializableObject::Equals::operator()(const ManagedSerializableFieldKey& a, const ManagedSerializableFieldKey& b) const
	{
		return a.mFieldId == b.mFieldId && a.mTypeId == b.mTypeId;
	}

	ManagedSerializableObject::ManagedSerializableObject(const ConstructPrivately& dummy)
	{

	}

	ManagedSerializableObject::ManagedSerializableObject(const ConstructPrivately& dummy, SPtr<ManagedSerializableObjectInfo> objInfo, MonoObject* managedInstance)
		:mObjInfo(objInfo)
	{
		mGCHandle = MonoUtil::newGCHandle(managedInstance, false);
	}

	ManagedSerializableObject::~ManagedSerializableObject()
	{
		if(mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}		
	}

	SPtr<ManagedSerializableObject> ManagedSerializableObject::CreateFromExisting(MonoObject* managedInstance)
	{
		if(managedInstance == nullptr)
			return nullptr;

		String elementNs;
		String elementTypeName;
		MonoUtil::getClassName(managedInstance, elementNs, elementTypeName);

		SPtr<ManagedSerializableObjectInfo> objInfo;
		if(!ScriptAssemblyManager::Instance().getSerializableObjectInfo(elementNs, elementTypeName, objInfo))
			return nullptr;

		return bs_shared_ptr_new<ManagedSerializableObject>(ConstructPrivately(), objInfo, managedInstance);
	}

	SPtr<ManagedSerializableObject> ManagedSerializableObject::CreateNew(const SPtr<ManagedSerializableTypeInfoObject>& type)
	{
		SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

		// See if this type even still exists
		if (!ScriptAssemblyManager::Instance().getSerializableObjectInfo(type->mTypeNamespace, type->mTypeName, currentObjInfo))
			return nullptr;

		return bs_shared_ptr_new<ManagedSerializableObject>(ConstructPrivately(), currentObjInfo, createManagedInstance(type));
	}

	MonoObject* ManagedSerializableObject::CreateManagedInstance(const SPtr<ManagedSerializableTypeInfoObject>& type)
	{
		SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

		// See if this type even still exists
		if (!ScriptAssemblyManager::Instance().getSerializableObjectInfo(type->mTypeNamespace, type->mTypeName, currentObjInfo))
			return nullptr;

		if(!currentObjInfo->mTypeInfo->mFlags.IsSet(ScriptTypeFlag::Serializable))
			return nullptr;

		const bool construct = currentObjInfo->mMonoClass->GetMethod(".ctor", 0) != nullptr;
		return currentObjInfo->mMonoClass->createInstance(construct);
	}

	SPtr<ManagedSerializableObject> ManagedSerializableObject::CreateEmpty()
	{
		return bs_shared_ptr_new<ManagedSerializableObject>(ConstructPrivately());
	}

	MonoObject* ManagedSerializableObject::GetManagedInstance() const
	{
		if(mGCHandle != 0)
			return MonoUtil::getObjectFromGCHandle(mGCHandle);

		return nullptr;
	}

	void ManagedSerializableObject::Serialize()
	{
		if(mGCHandle == 0)
			return;

		mCachedData.clear();

		SPtr<ManagedSerializableObjectInfo> curType = mObjInfo;
		while (curType != nullptr)
		{
			for (auto& field : curType->mFields)
			{
				if (field.second->isSerializable())
				{
					ManagedSerializableFieldKey key(field.second->mParentTypeId, field.second->mFieldId);
					mCachedData[key] = getFieldData(field.second);
				}
			}

			curType = curType->mBaseClass;
		}

		// Serialize children
		for (auto& fieldEntry : mCachedData)
			fieldEntry.second->serialize();

		MonoUtil::freeGCHandle(mGCHandle);
		mGCHandle = 0;
	}

	MonoObject* ManagedSerializableObject::Deserialize()
	{
		// See if this type even still exists
		SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;
		if (!ScriptAssemblyManager::Instance().getSerializableObjectInfo(mObjInfo->mTypeInfo->mTypeNamespace,
			mObjInfo->mTypeInfo->mTypeName, currentObjInfo))
		{
			return nullptr;
		}

		MonoObject* managedInstance = createManagedInstance(currentObjInfo->mTypeInfo);
		deserialize(managedInstance, currentObjInfo);

		return managedInstance;
	}

	void ManagedSerializableObject::Deserialize(MonoObject* instance, const SPtr<ManagedSerializableObjectInfo>& objInfo)
	{
		if (instance == nullptr)
			return;

		// Deserialize children
		for (auto& fieldEntry : mCachedData)
			fieldEntry.second->deserialize();

		// Scan all fields and ensure the fields still exist
		UINT32 i = 0;
		SPtr<ManagedSerializableObjectInfo> curType = mObjInfo;
		while (curType != nullptr)
		{
			for (auto& field : curType->mFields)
			{
				if (field.second->isSerializable())
				{
					UINT32 fieldId = field.second->mFieldId;
					UINT32 typeID = field.second->mParentTypeId;

					ManagedSerializableFieldKey key(typeID, fieldId);

					SPtr<ManagedSerializableMemberInfo> matchingFieldInfo = objInfo->findMatchingField(field.second, curType->mTypeInfo);
					if (matchingFieldInfo != nullptr)
						matchingFieldInfo->SetValue(instance, mCachedData[key]->GetValue(matchingFieldInfo->mTypeInfo));

					i++;
				}
			}

			curType = curType->mBaseClass;
		}
	}

	bool ManagedSerializableObject::Equals(ManagedSerializableObject& other)
	{
		SPtr<ManagedSerializableObjectInfo> otherObjInfo = other.getObjectInfo();

		if (!mObjInfo->mTypeInfo->matches(otherObjInfo->mTypeInfo))
			return false;

		SPtr<ManagedSerializableObjectInfo> curObjInfo = mObjInfo;
		while (curObjInfo != nullptr)
		{
			for (auto& field : curObjInfo->mFields)
			{
				if (!field.second->isSerializable())
					continue;

				SPtr<ManagedSerializableFieldData> oldData = getFieldData(field.second);
				SPtr<ManagedSerializableFieldData> newData = other.getFieldData(field.second);

				if (!oldData)
					return !newData;
				else
				{
					if (!newData)
						return false;
				}

				if(!oldData->equals(newData))
					return false;
			}

			curObjInfo = curObjInfo->mBaseClass;
		}

		return true;
	}

	void ManagedSerializableObject::SetFieldData(const SPtr<ManagedSerializableMemberInfo>& fieldInfo, const SPtr<ManagedSerializableFieldData>& val)
	{
		if (mGCHandle != 0)
		{
			MonoObject* managedInstance = MonoUtil::getObjectFromGCHandle(mGCHandle);
			fieldInfo->SetValue(managedInstance, val->GetValue(fieldInfo->mTypeInfo));
		}
		else
		{
			ManagedSerializableFieldKey key(fieldInfo->mParentTypeId, fieldInfo->mFieldId);
			mCachedData[key] = val;
		}
	}

	SPtr<ManagedSerializableFieldData> ManagedSerializableObject::GetFieldData(const SPtr<ManagedSerializableMemberInfo>& fieldInfo) const
	{
		if (mGCHandle != 0)
		{
			MonoObject* managedInstance = MonoUtil::getObjectFromGCHandle(mGCHandle);
			MonoObject* fieldValue = fieldInfo->GetValue(managedInstance);

			return ManagedSerializableFieldData::Create(fieldInfo->mTypeInfo, fieldValue);
		}
		else
		{
			ManagedSerializableFieldKey key(fieldInfo->mParentTypeId, fieldInfo->mFieldId);
			auto iterFind = mCachedData.find(key);

			if (iterFind != mCachedData.end())
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
}
