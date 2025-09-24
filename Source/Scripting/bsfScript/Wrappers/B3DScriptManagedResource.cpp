//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptManagedResource.h"
#include "BsScriptResourceManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsMonoClass.h"
#include "BsManagedResource.h"
#include "Resources/BsResources.h"
#include "BsMonoUtil.h"

using namespace b3d;
ScriptManagedResource::ScriptManagedResource(const HManagedResource& nativeObject)
	: TScriptResourceWrapper(nativeObject)
{
	RegisterEvents();
}

void ScriptManagedResource::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptManagedResource::InternalCreateInstance);
}

void ScriptManagedResource::InternalCreateInstance(MonoObject* scriptObject)
{
	HManagedResource resource = ManagedResource::CreateUninitialized();
	ScriptObjectWrapper::Create<ScriptManagedResource>(resource, scriptObject);
	resource->Initialize();
}

void ScriptManagedResource::CreateAndBindScriptObject()
{
	B3D_ENSURE(GetScriptObject() == nullptr);

	if(!IsNativeObjectValid())
		return;

	HManagedResource resource = GetNativeObjectAsHandle();
	SPtr<ManagedObjectInfo> objectInformation;
	MonoObject* const scriptObject = resource->CreateScriptObject(objectInformation);

	if(scriptObject != nullptr)
		BindToScriptObject(scriptObject);

	resource->SetupScriptBindings(objectInformation);
}

void ScriptManagedResource::RecreateScriptObjectAfterScriptReload()
{
	CreateAndBindScriptObject();
}

Optional<ScriptObjectReloadPersistentData> ScriptManagedResource::BackupDataBeforeScriptReload()
{
	if(!IsNativeObjectValid())
		return { };

	HManagedResource managedResource = GetNativeObjectAsHandle();

	ScriptObjectReloadPersistentData backupData;
	backupData.Data = managedResource->Backup();

	return backupData;
}

void ScriptManagedResource::RestoreDataAfterScriptReload(const ScriptObjectReloadPersistentData& data)
{
	HManagedResource managedResource = GetNativeObjectAsHandle();

	ResourceBackupData resourceBackup = AnyCast<ResourceBackupData>(data.Data);
	managedResource->Restore(resourceBackup);
}
