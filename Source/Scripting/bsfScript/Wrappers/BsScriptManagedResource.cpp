//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		: ScriptObject(instance), mResource(resource)
	{
		BS_ASSERT(instance != nullptr);

		MonoUtil::GetClassName(instance, mNamespace, mType);
		mGCHandle = MonoUtil::NewGcHandle(instance, false);
	}

	void ScriptManagedResource::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptManagedResource::InternalCreateInstance);
	}

	void ScriptManagedResource::InternalCreateInstance(MonoObject* instance)
	{
		HManagedResource resource = ManagedResource::Create(instance);
	}

	MonoObject* ScriptManagedResource::CreateManagedInstanceInternal(bool construct)
	{
		SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

		// See if this type even still exists
		if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(mNamespace, mType, currentObjInfo))
			return nullptr;

		MonoObject* instance = currentObjInfo->MMonoClass->CreateInstance(construct);
		mGCHandle = MonoUtil::NewGcHandle(instance, false);

		return instance;
	}

	void ScriptManagedResource::ClearManagedInstanceInternal()
	{
		FreeManagedInstance();
	}

	ScriptObjectBackup ScriptManagedResource::BeginRefresh()
	{
		ScriptObjectBackup backupData;
		backupData.Data = mResource->Backup();

		return backupData;
	}

	void ScriptManagedResource::EndRefresh(const ScriptObjectBackup& backupData)
	{
		MonoObject* instance = MonoUtil::GetObjectFromGcHandle(mGCHandle);

		ResourceBackupData resourceBackup = any_cast<ResourceBackupData>(backupData.Data);
		mResource->Restore(resourceBackup);

		// If we could not find resource type after refresh, treat it as if it was destroyed
		if(instance == nullptr)
			OnManagedInstanceDeletedInternal(false);
	}

	void ScriptManagedResource::OnManagedInstanceDeletedInternal(bool assemblyRefresh)
	{
		mGCHandle = 0;

		if(!assemblyRefresh || mResource->IsDestroyed())
		{
			// The only way this method should be reachable is when Resource::unload is called, which means the resource
			// has had to been already freed. Even if all managed instances are released ManagedResource itself holds the last
			// instance which is only freed on unload().
			// Note: During domain unload this could get called even if not all instances are released, but ManagedResourceManager
			// should make sure all instances are unloaded before that happens.
			BS_ASSERT(mResource == nullptr || !mResource.IsLoaded());

			ScriptResourceManager::Instance().DestroyScriptResource(this);
		}
	}

	void ScriptManagedResource::NotifyDestroyedInternal()
	{
		FreeManagedInstance();
	}

	void ScriptManagedResource::SetResource(const HResource& resource)
	{
		mResource = static_resource_cast<ManagedResource>(resource);
	}
} // namespace bs
