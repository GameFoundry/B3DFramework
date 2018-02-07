//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptManagedResource.h"
#include "BsScriptResourceManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsManagedResource.h"
#include "Resources/BsResources.h"
#include "Error/BsException.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptManagedResource::ScriptManagedResource(MonoObject* instance, const HManagedResource& resource)
		:ScriptObject(instance), mResource(resource)
	{
		BS_ASSERT(instance != nullptr);

		MonoUtil::getClassName(instance, mNamespace, mType);
		mGCHandle = MonoUtil::newWeakGCHandle(instance);
	}

	void ScriptManagedResource::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptManagedResource::internal_createInstance);
	}

	void ScriptManagedResource::internal_createInstance(MonoObject* instance)
	{
		HManagedResource resource = ManagedResource::create(instance);
	}

	MonoObject* ScriptManagedResource::_createManagedInstance(bool construct)
	{
		SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

		// See if this type even still exists
		if (!ScriptAssemblyManager::instance().getSerializableObjectInfo(mNamespace, mType, currentObjInfo))
			return nullptr;

		MonoObject* instance = currentObjInfo->mMonoClass->createInstance(construct);
		mGCHandle = MonoUtil::newWeakGCHandle(instance);

		return instance;
	}

	void ScriptManagedResource::_clearManagedInstance()
	{
		mGCHandle = 0;
	}

	ScriptObjectBackup ScriptManagedResource::beginRefresh()
	{
		ScriptObjectBackup backupData;
		backupData.data = mResource->backup(true);

		return backupData;
	}

	void ScriptManagedResource::endRefresh(const ScriptObjectBackup& backupData)
	{
		MonoObject* instance = MonoUtil::getObjectFromGCHandle(mGCHandle);

		ResourceBackupData resourceBackup = any_cast<ResourceBackupData>(backupData.data);
		mResource->restore(instance, resourceBackup);

		// If we could not find resource type after refresh, treat it as if it was destroyed
		if (instance == nullptr)
			_onManagedInstanceDeleted(false);
	}

	void ScriptManagedResource::_onManagedInstanceDeleted(bool assemblyRefresh)
	{
		mGCHandle = 0;
		
		if (!assemblyRefresh)
		{
			// The only way this method should be reachable is when Resource::unload is called, which means the resource
			// has had to been already freed. Even if all managed instances are released ManagedResource itself holds the last
			// instance which is only freed on unload().
			// Note: During domain unload this could get called even if not all instances are released, but ManagedResourceManager
			// should make sure all instances are unloaded before that happens.
			BS_ASSERT(mResource == nullptr || !mResource.isLoaded());

			ScriptResourceManager::instance().destroyScriptResource(this);
		}
	}

	void ScriptManagedResource::setResource(const HResource& resource)
	{
		mResource = static_resource_cast<ManagedResource>(resource);
	}
}