//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/B3DScriptManagedResource.h"
#include "B3DScriptResourceManager.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "B3DMonoClass.h"
#include "B3DManagedResource.h"
#include "Resources/B3DResources.h"
#include "B3DMonoUtil.h"

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
	TShared<ManagedObjectInfo> objectInformation;
	MonoObject* const scriptObject = resource->CreateScriptObject(objectInformation);

	if(scriptObject != nullptr)
		BindToScriptObject(scriptObject);

	resource->SetupScriptBindings(objectInformation);
}

void ScriptManagedResource::RecreateScriptObjectAfterScriptReload()
{
	CreateAndBindScriptObject();
}

TOptional<ScriptObjectReloadPersistentData> ScriptManagedResource::BackupDataBeforeScriptReload()
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
