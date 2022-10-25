//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsManagedResource.h"
#include "RTTI/BsManagedResourceRTTI.h"
#include "BsManagedResourceMetaData.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "Resources/BsResources.h"
#include "BsManagedResourceManager.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Wrappers/BsScriptManagedResource.h"
#include "BsScriptResourceManager.h"
#include "BsMonoUtil.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Debug/BsDebug.h"
#include "Serialization/BsBinarySerializer.h"
#include "FileSystem/BsDataStream.h"

namespace bs
{
	ManagedResource::ManagedResource()
		: Resource(false)
	{}

	ManagedResource::ManagedResource(MonoObject* managedInstance)
		: Resource(false)
	{
		SPtr<ManagedResourceMetaData> metaData = bs_shared_ptr_new<ManagedResourceMetaData>();
		mMetaData = metaData;

		MonoUtil::GetClassName(managedInstance, metaData->TypeNamespace, metaData->TypeName);

		MonoClass* managedClass = MonoManager::Instance().FindClass(metaData->TypeNamespace, metaData->TypeName);
		if(managedClass == nullptr)
		{
			BS_LOG(Warning, Script, "Cannot create managed component: {0}.{1} because that type doesn't exist.", metaData->TypeNamespace, metaData->TypeName);
			return;
		}
	}

	MonoObject* ManagedResource::GetManagedInstance() const
	{
		if(mOwner)
			return mOwner->GetManagedInstance();

		return nullptr;
	}

	ResourceBackupData ManagedResource::Backup()
	{
		MonoObject* instance = mOwner->GetManagedInstance();
		SPtr<ManagedSerializableObject> serializableObject = ManagedSerializableObject::CreateFromExisting(instance);

		ResourceBackupData backupData;
		if(serializableObject != nullptr)
		{
			SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>();
			BinarySerializer bs;

			bs.Encode(serializableObject.get(), stream);

			backupData.Size = (u32)stream->Size();
			backupData.Data = stream->DisownMemory();
		}
		else
		{
			backupData.Size = 0;
			backupData.Data = nullptr;
		}

		return backupData;
	}

	void ManagedResource::Restore(const ResourceBackupData& data)
	{
		MonoObject* instance = mOwner->GetManagedInstance();
		if(instance != nullptr)
		{
			if(data.Data != nullptr)
			{
				BinarySerializer bs;
				SPtr<ManagedSerializableObject> serializableObject = std::static_pointer_cast<ManagedSerializableObject>(
					bs.Decode(bs_shared_ptr_new<MemoryDataStream>(data.Data, data.Size), data.Size));

				SPtr<ManagedResourceMetaData> managedResMetaData = std::static_pointer_cast<ManagedResourceMetaData>(mMetaData);
				SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

				if(ScriptAssemblyManager::Instance().GetSerializableObjectInfo(managedResMetaData->TypeNamespace, managedResMetaData->TypeName, currentObjInfo))
					serializableObject->Deserialize(instance, currentObjInfo);
			}
		}
		else
		{
			// Could not restore resource
			ManagedResourceManager::Instance().UnregisterManagedResource(mMyHandle);
		}
	}

	HManagedResource ManagedResource::Create(MonoObject* managedResource)
	{
		SPtr<ManagedResource> newRes = bs_core_ptr<ManagedResource>(new(bs_alloc<ManagedResource>()) ManagedResource(managedResource));
		newRes->SetThisPtrInternal(newRes);
		newRes->Initialize();

		HManagedResource handle = static_resource_cast<ManagedResource>(gResources().CreateResourceHandleInternal(newRes));
		newRes->SetHandle(managedResource, handle);

		return handle;
	}

	SPtr<ManagedResource> ManagedResource::CreateEmpty()
	{
		SPtr<ManagedResource> newRes = bs_core_ptr<ManagedResource>(new(bs_alloc<ManagedResource>()) ManagedResource());
		newRes->SetThisPtrInternal(newRes);
		newRes->Initialize();

		return newRes;
	}

	void ManagedResource::SetHandle(MonoObject* object, const HManagedResource& myHandle)
	{
		mMyHandle = myHandle.GetWeak();

		mOwner = ScriptResourceManager::Instance().CreateManagedScriptResource(myHandle, object);
		ManagedResourceManager::Instance().RegisterManagedResource(mMyHandle);
	}

	void ManagedResource::Destroy()
	{
		Resource::Destroy();

		mOwner->NotifyDestroyedInternal();
		ManagedResourceManager::Instance().UnregisterManagedResource(mMyHandle);
	}

	RTTITypeBase* ManagedResource::GetRttiStatic()
	{
		return ManagedResourceRTTI::Instance();
	}

	RTTITypeBase* ManagedResource::GetRtti() const
	{
		return ManagedResource::GetRttiStatic();
	}
} // namespace bs
