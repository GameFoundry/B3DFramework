//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptManagedComponent.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptObjectManager.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "Serialization/BsScriptAssemblyManager.h"
#include "BsManagedComponent.h"
#include "Scene/BsSceneObject.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptManagedComponent::ScriptManagedComponent(MonoObject* instance, const HManagedComponent& component)
		: ScriptObject(instance), mComponent(component), mTypeMissing(false)
	{
		assert(instance != nullptr);

		MonoUtil::GetClassName(instance, mNamespace, mType);
		mGCHandle = MonoUtil::NewGcHandle(instance, false);

		component->Initialize(this);
	}

	void ScriptManagedComponent::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Invoke", (void*)&ScriptManagedComponent::InternalInvoke);
	}

	void ScriptManagedComponent::InternalInvoke(ScriptManagedComponent* nativeInstance, MonoString* name)
	{
		HManagedComponent comp = nativeInstance->mComponent;
		if(CheckIfDestroyed(nativeInstance->mComponent))
			return;

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
			if(baseClass != metaData.ScriptClass)
				compClass = baseClass;
			else
				break;
		}

		if(!found)
		{
			BS_LOG(Warning, Script, "Method invoke failed. Cannot find method \"{0}\" on component of type \"{1}\".", methodName, compClass->GetTypeName());
		}
	}

	MonoObject* ScriptManagedComponent::CreateManagedInstanceInternal(bool construct)
	{
		SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

		// See if this type even still exists
		MonoObject* instance;
		if(!ScriptAssemblyManager::Instance().GetSerializableObjectInfo(mNamespace, mType, currentObjInfo))
		{
			mTypeMissing = true;
			instance = ScriptAssemblyManager::Instance().GetBuiltinClasses().MissingComponentClass->CreateInstance(true);
		}
		else
		{
			mTypeMissing = false;
			instance = currentObjInfo->MMonoClass->CreateInstance(construct);
		}

		mGCHandle = MonoUtil::NewGcHandle(instance, false);
		return instance;
	}

	void ScriptManagedComponent::ClearManagedInstanceInternal()
	{
		FreeManagedInstance();
	}

	ScriptObjectBackup ScriptManagedComponent::BeginRefresh()
	{
		HManagedComponent managedComponent = static_object_cast<ManagedComponent>(mComponent);
		ScriptObjectBackup backupData;

		// It's possible that managed component is destroyed but a reference to it
		// is still kept. Don't backup such components.
		if(!managedComponent.IsDestroyed(true))
			backupData.Data = managedComponent->Backup(true);

		return backupData;
	}

	void ScriptManagedComponent::EndRefresh(const ScriptObjectBackup& backupData)
	{
		HManagedComponent managedComponent = static_object_cast<ManagedComponent>(mComponent);

		RawBackupData componentBackup = any_cast<RawBackupData>(backupData.Data);
		managedComponent->Restore(componentBackup, mTypeMissing);
	}

	void ScriptManagedComponent::OnManagedInstanceDeletedInternal(bool assemblyRefresh)
	{
		mGCHandle = 0;

		// It's possible that managed component is destroyed but a reference to it
		// is still kept during assembly refresh. Such components shouldn't be restored
		// so we delete them.
		if(!assemblyRefresh || mComponent.IsDestroyed(true))
			ScriptGameObjectManager::Instance().DestroyScriptComponent(this);
	}

	void ScriptManagedComponent::NotifyDestroyedInternal()
	{
		FreeManagedInstance();
	}
} // namespace bs
