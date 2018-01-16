//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedResource.h"
#include "RTTI/BsManagedResourceRTTI.h"
#include "BsManagedResourceMetaData.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "Resources/BsResources.h"
#include "BsManagedResourceManager.h"
#include "Serialization/BsManagedSerializableObject.h"
#include "Wrappers/BsScriptManagedResource.h"
#include "Serialization/BsMemorySerializer.h"
#include "BsScriptResourceManager.h"
#include "BsMonoUtil.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "Debug/BsDebug.h"

namespace bs
{
	ManagedResource::ManagedResource()
		:Resource(false), mGCHandle(0)
	{ }

	ManagedResource::ManagedResource(MonoObject* managedInstance)
		:Resource(false), mGCHandle(0)
	{
		SPtr<ManagedResourceMetaData> metaData = bs_shared_ptr_new<ManagedResourceMetaData>();
		mMetaData = metaData;

		MonoUtil::getClassName(managedInstance, metaData->typeNamespace, metaData->typeName);

		MonoClass* managedClass = MonoManager::instance().findClass(metaData->typeNamespace, metaData->typeName);
		if (managedClass == nullptr)
		{
			LOGWRN("Cannot create managed component: " + metaData->typeNamespace + "." + metaData->typeName + " because that type doesn't exist.");
			return;
		}
	}

	MonoObject* ManagedResource::getManagedInstance() const
	{
		if(mGCHandle != 0)
			return MonoUtil::getObjectFromGCHandle(mGCHandle);
		
		return nullptr;
	}

	ResourceBackupData ManagedResource::backup(bool clearExisting)
	{
		MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);
		SPtr<ManagedSerializableObject> serializableObject = ManagedSerializableObject::createFromExisting(instance);

		ResourceBackupData backupData;
		if (serializableObject != nullptr)
		{
			MemorySerializer ms;

			backupData.size = 0;
			backupData.data = ms.encode(serializableObject.get(), backupData.size);
		}
		else
		{
			backupData.size = 0;
			backupData.data = nullptr;
		}

		if (clearExisting)
		{
			if (mGCHandle != 0)
			{
				MonoUtil::freeGCHandle(mGCHandle);
				mGCHandle = 0;
			}
		}

		return backupData;
	}

	void ManagedResource::restore(MonoObject* instance, const ResourceBackupData& data)
	{
		if (instance != nullptr)
		{
			mGCHandle = MonoUtil::newGCHandle(instance, false);

			if (data.data != nullptr)
			{
				MemorySerializer ms;
				SPtr<ManagedSerializableObject> serializableObject = std::static_pointer_cast<ManagedSerializableObject>(ms.decode(data.data, data.size));
				
				SPtr<ManagedResourceMetaData> managedResMetaData = std::static_pointer_cast<ManagedResourceMetaData>(mMetaData);
				SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

				if (ScriptAssemblyManager::instance().getSerializableObjectInfo(managedResMetaData->typeNamespace, managedResMetaData->typeName, currentObjInfo))
					serializableObject->deserialize(instance, currentObjInfo);
			}
		}
		else
		{
			// Could not restore resource
			ManagedResourceManager::instance().unregisterManagedResource(mMyHandle);
		}
	}

	HManagedResource ManagedResource::create(MonoObject* managedResource)
	{
		SPtr<ManagedResource> newRes = bs_core_ptr<ManagedResource>(new (bs_alloc<ManagedResource>()) ManagedResource(managedResource));
		newRes->_setThisPtr(newRes);
		newRes->initialize();

		HManagedResource handle = static_resource_cast<ManagedResource>(gResources()._createResourceHandle(newRes));
		newRes->setHandle(managedResource, handle);

		return handle;
	}

	SPtr<ManagedResource> ManagedResource::createEmpty()
	{
		SPtr<ManagedResource> newRes = bs_core_ptr<ManagedResource>(new (bs_alloc<ManagedResource>()) ManagedResource());
		newRes->_setThisPtr(newRes);
		newRes->initialize();

		return newRes;
	}

	void ManagedResource::setHandle(MonoObject* object, const HManagedResource& myHandle)
	{
		mGCHandle = MonoUtil::newGCHandle(object, false);
		mMyHandle = myHandle.getWeak();

		ScriptResourceManager::instance().createManagedScriptResource(myHandle, object);
		ManagedResourceManager::instance().registerManagedResource(mMyHandle);
	}

	void ManagedResource::destroy()
	{
		Resource::destroy();

		if (mGCHandle != 0)
		{
			MonoUtil::freeGCHandle(mGCHandle);
			mGCHandle = 0;
		}

		ManagedResourceManager::instance().unregisterManagedResource(mMyHandle);
	}

	RTTITypeBase* ManagedResource::getRTTIStatic()
	{
		return ManagedResourceRTTI::instance();
	}

	RTTITypeBase* ManagedResource::getRTTI() const
	{
		return ManagedResource::getRTTIStatic();
	}
}