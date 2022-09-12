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
		:ScriptObject(instance), mComponent(component), mTypeMissing(false)
	{
		assert(instance != nullptr);

		MonoUtil::getClassName(instance, mNamespace, mType);
		mGCHandle = MonoUtil::newGCHandle(instance, false);

		component->Initialize(this);
	}

	void ScriptManagedComponent::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_Invoke", (void*)&ScriptManagedComponent::internal_invoke);
	}

	void ScriptManagedComponent::internal_invoke(ScriptManagedComponent* nativeInstance, MonoString* name)
	{
		HManagedComponent comp = nativeInstance->mComponent;
		if (checkIfDestroyed(nativeInstance->mComponent))
			return;

		MonoObject* compObj = comp->GetManagedInstance();
		MonoClass* compClass = comp->GetClass();

		bool found = false;
		String methodName = MonoUtil::monoToString(name);
		while (compClass != nullptr)
		{
			MonoMethod* method = compClass->GetMethod(methodName);
			if (method != nullptr)
			{
				method->Invoke(compObj, nullptr);
				found = true;
				break;
			}

			// Search for methods on base class if there is one
			MonoClass* baseClass = compClass->GetBaseClass();
			if (baseClass != metaData.scriptClass)
				compClass = baseClass;
			else
				break;
		}

		if (!found)
		{
			BS_LOG(Warning, Script, "Method invoke failed. Cannot find method \"{0}\" on component of type \"{1}\".",
				methodName, compClass->GetTypeName());
		}
	}

	MonoObject* ScriptManagedComponent::_createManagedInstance(bool construct)
	{
		SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

		// See if this type even still exists
		MonoObject* instance;
		if (!ScriptAssemblyManager::instance().GetSerializableObjectInfo(mNamespace, mType, currentObjInfo))
		{
			mTypeMissing = true;
			instance = ScriptAssemblyManager::instance().GetBuiltinClasses().missingComponentClass->CreateInstance(true);
		}
		else
		{
			mTypeMissing = false;
			instance = currentObjInfo->mMonoClass->CreateInstance(construct);
		}

		mGCHandle = MonoUtil::newGCHandle(instance, false);
		return instance;
	}

	void ScriptManagedComponent::_clearManagedInstance()
	{
		freeManagedInstance();
	}

	ScriptObjectBackup ScriptManagedComponent::BeginRefresh()
	{
		HManagedComponent managedComponent = static_object_cast<ManagedComponent>(mComponent);
		ScriptObjectBackup backupData;

		// It's possible that managed component is destroyed but a reference to it
		// is still kept. Don't backup such components.
		if (!managedComponent.IsDestroyed(true))
			backupData.data = managedComponent->Backup(true);

		return backupData;
	}

	void ScriptManagedComponent::EndRefresh(const ScriptObjectBackup& backupData)
	{
		HManagedComponent managedComponent = static_object_cast<ManagedComponent>(mComponent);

		RawBackupData componentBackup = any_cast<RawBackupData>(backupData.data);
		managedComponent->Restore(componentBackup, mTypeMissing);
	}

	void ScriptManagedComponent::_onManagedInstanceDeleted(bool assemblyRefresh)
	{
		mGCHandle = 0;

		// It's possible that managed component is destroyed but a reference to it
		// is still kept during assembly refresh. Such components shouldn't be restored
		// so we delete them.
		if (!assemblyRefresh || mComponent.IsDestroyed(true))
			ScriptGameObjectManager::instance().DestroyScriptComponent(this);
	}

	void ScriptManagedComponent::_notifyDestroyed()
	{
		freeManagedInstance();
	}
}
