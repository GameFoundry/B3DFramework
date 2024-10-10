//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptManagedComponent.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "BsManagedComponent.h"
#include "Scene/BsSceneObject.h"
#include "BsMonoUtil.h"

using namespace bs;
ScriptManagedComponent::ScriptManagedComponent(const HManagedComponent& nativeObject)
	: TScriptGameObjectWrapper(nativeObject)
{
	RegisterEvents();
}

void ScriptManagedComponent::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Invoke", (void*)&ScriptManagedComponent::InternalInvoke);
}

void ScriptManagedComponent::InternalInvoke(ScriptManagedComponent* self, MonoString* name)
{
	if(!self->IsNativeObjectValid())
		return;

	HManagedComponent comp = self->GetNativeObjectAsHandle();

	MonoObject* compObj = comp->GetManagedInstance();
	MonoClass* compClass = comp->GetClass();

	bool found = false;
	String methodName = MonoUtil::MonoToString(name);
	while(compClass != nullptr)
	{
		MonoMethod* method = compClass->GetMethod(methodName);
		if(method != nullptr)
		{
			method->Invoke(compObj, nullptr);
			found = true;
			break;
		}

		// Search for methods on base class if there is one
		MonoClass* baseClass = compClass->GetBaseClass();
		if(baseClass != sInteropMetaData.ScriptClass)
			compClass = baseClass;
		else
			break;
	}

	if(!found)
	{
		B3D_LOG(Warning, Script, "Method invoke failed. Cannot find method \"{0}\" on component of type \"{1}\".", methodName, compClass->GetTypeName());
	}
}

void ScriptManagedComponent::CreateAndBindScriptObject()
{
	B3D_ENSURE(GetScriptObject() == nullptr);

	if(!IsNativeObjectValid())
		return;

	HManagedComponent component = GetNativeObjectAsHandle();
	SPtr<ManagedObjectInfo> objectInformation;
	MonoObject* const scriptObject = component->CreateScriptObject(objectInformation);

	if(scriptObject != nullptr)
		BindToScriptObject(scriptObject);

	component->SetupScriptBindings(objectInformation);
}

void ScriptManagedComponent::RecreateScriptObjectAfterScriptReload()
{
	CreateAndBindScriptObject();
}

Optional<ScriptObjectReloadPersistentData> ScriptManagedComponent::BackupDataBeforeScriptReload()
{
	if(!IsNativeObjectValid())
		return { };

	HManagedComponent managedComponent = GetNativeObjectAsHandle();

	ScriptObjectReloadPersistentData backupData;
	backupData.Data = managedComponent->Backup(true);

	return backupData;
}

void ScriptManagedComponent::RestoreDataAfterScriptReload(const ScriptObjectReloadPersistentData& data)
{
	HManagedComponent managedComponent = GetNativeObjectAsHandle();

	RawBackupData componentBackup = AnyCast<RawBackupData>(data.Data);
	managedComponent->Restore(componentBackup);
}

void ScriptManagedComponent::NotifyScriptReloadFinished()
{
	if(!IsNativeObjectValid())
		return;

	HManagedComponent managedComponent = GetNativeObjectAsHandle();
	managedComponent->TriggerOnReset();
}

