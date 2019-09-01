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
		:Resource(false)
	{ }

	ManagedResource::ManagedResource(MonoObject* managedInstance)
		:Resource(false)
	{
		SPtr<ManagedResourceMetaData> metaData = bs_shared_ptr_new<ManagedResourceMetaData>();
		mMetaData = metaData;

		MonoUtil::getClassName(managedInstance, metaData->typeNamespace, metaData->typeName);

		MonoClass* managedClass = MonoManager::instance().findClass(metaData->typeNamespace, metaData->typeName);
		if (managedClass == nullptr)
		{
			BS_LOG(Warning, Script, "Cannot create managed component: {0}.{1} because that type doesn't exist.",
				metaData->typeNamespace, metaData->typeName);
			return;
		}
	}

	MonoObject* ManagedResource::getManagedInstance() const
	{
		if(mOwner)
			return mOwner->getManagedInstance();
		
		return nullptr;
	}

	ResourceBackupData ManagedResource::backup()
	{
		MonoObject* instance = mOwner->getManagedInstance();
		SPtr<ManagedSerializableObject> serializableObject = ManagedSerializableObject::createFromExisting(instance);

		ResourceBackupData backupData;
		if (serializableObject != nullptr)
		{
			SPtr<MemoryDataStream> stream = bs_shared_ptr_new<MemoryDataStream>();
			BinarySerializer bs;

			bs.encode(serializableObject.get(), stream);

			backupData.size = (UINT32)stream->size();
			backupData.data = stream->disownMemory();
		}
		else
		{
			backupData.size = 0;
			backupData.data = nullptr;
		}

		return backupData;
	}

	void ManagedResource::restore(const ResourceBackupData& data)
	{
		MonoObject* instance = mOwner->getManagedInstance();
		if (instance != nullptr)
		{
			if (data.data != nullptr)
			{
				BinarySerializer bs;
				SPtr<ManagedSerializableObject> serializableObject = std::static_pointer_cast<ManagedSerializableObject>(
					bs.decode(bs_shared_ptr_new<MemoryDataStream>(data.data, data.size), data.size));
				
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
		mMyHandle = myHandle.getWeak();

		mOwner = ScriptResourceManager::instance().createManagedScriptResource(myHandle, object);
		ManagedResourceManager::instance().registerManagedResource(mMyHandle);
	}

	void ManagedResource::destroy()
	{
		Resource::destroy();

		mOwner->_notifyDestroyed();
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
