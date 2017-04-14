//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedResource.h"
#include "BsManagedResourceRTTI.h"
#include "BsManagedResourceMetaData.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsResources.h"
#include "BsManagedResourceManager.h"
#include "BsManagedSerializableObject.h"
#include "BsScriptManagedResource.h"
#include "BsMemorySerializer.h"
#include "BsScriptResourceManager.h"
#include "BsMonoUtil.h"
#include "BsScriptAssemblyManager.h"
#include "BsDebug.h"

namespace bs
{
	ManagedResource::ManagedResource()
		:Resource(false), mManagedInstance(nullptr)
	{ }

	ManagedResource::ManagedResource(MonoObject* managedInstance)
		:Resource(false), mManagedInstance(nullptr)
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

	ResourceBackupData ManagedResource::backup(bool clearExisting)
	{
		SPtr<ManagedSerializableObject> serializableObject = ManagedSerializableObject::createFromExisting(mManagedInstance);

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
			if (mManagedInstance != nullptr)
			{
				mManagedInstance = nullptr;
				MonoUtil::freeGCHandle(mManagedHandle);
				mManagedHandle = 0;
			}
		}

		return backupData;
	}

	void ManagedResource::restore(MonoObject* instance, const ResourceBackupData& data)
	{
		mManagedInstance = instance;

		if (mManagedInstance != nullptr)
		{
			mManagedHandle = MonoUtil::newGCHandle(mManagedInstance);

			if (data.data != nullptr)
			{
				MemorySerializer ms;
				SPtr<ManagedSerializableObject> serializableObject = std::static_pointer_cast<ManagedSerializableObject>(ms.decode(data.data, data.size));
				
				SPtr<ManagedResourceMetaData> managedResMetaData = std::static_pointer_cast<ManagedResourceMetaData>(mMetaData);
				SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

				if (ScriptAssemblyManager::instance().getSerializableObjectInfo(managedResMetaData->typeNamespace, managedResMetaData->typeName, currentObjInfo))
					serializableObject->deserialize(mManagedInstance, currentObjInfo);
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
		mManagedInstance = object;
		mManagedHandle = MonoUtil::newGCHandle(mManagedInstance);
		mMyHandle = myHandle.getWeak();

		ScriptManagedResource* scriptInstance = ScriptResourceManager::instance().createManagedScriptResource(myHandle, object);
		ManagedResourceManager::instance().registerManagedResource(mMyHandle);
	}

	void ManagedResource::destroy()
	{
		Resource::destroy();

		if (mManagedInstance != nullptr)
		{
			mManagedInstance = nullptr;
			MonoUtil::freeGCHandle(mManagedHandle);
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