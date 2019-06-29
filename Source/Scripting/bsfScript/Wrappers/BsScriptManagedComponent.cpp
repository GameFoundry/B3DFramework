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

		component->initialize(this);
	}

	void ScriptManagedComponent::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Invoke", (void*)&ScriptManagedComponent::internal_invoke);
	}

	void ScriptManagedComponent::internal_invoke(ScriptManagedComponent* nativeInstance, MonoString* name)
	{
		HManagedComponent comp = nativeInstance->mComponent;
		if (checkIfDestroyed(nativeInstance->mComponent))
			return;

		MonoObject* compObj = comp->getManagedInstance();
		MonoClass* compClass = comp->getClass();

		bool found = false;
		String methodName = MonoUtil::monoToString(name);
		while (compClass != nullptr)
		{
			MonoMethod* method = compClass->getMethod(methodName);
			if (method != nullptr)
			{
				method->invoke(compObj, nullptr);
				found = true;
				break;
			}

			// Search for methods on base class if there is one
			MonoClass* baseClass = compClass->getBaseClass();
			if (baseClass != metaData.scriptClass)
				compClass = baseClass;
			else
				break;
		}

		if (!found)
		{
			BS_LOG(Warning, Script, "Method invoke failed. Cannot find method \"{0}\" on component of type \"{1}\".", 
				methodName, compClass->getTypeName());
		}
	}

	MonoObject* ScriptManagedComponent::_createManagedInstance(bool construct)
	{
		SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

		// See if this type even still exists
		MonoObject* instance;
		if (!ScriptAssemblyManager::instance().getSerializableObjectInfo(mNamespace, mType, currentObjInfo))
		{
			mTypeMissing = true;
			instance = ScriptAssemblyManager::instance().getBuiltinClasses().missingComponentClass->createInstance(true);
		}
		else
		{
			mTypeMissing = false;
			instance = currentObjInfo->mMonoClass->createInstance(construct);
		}

		mGCHandle = MonoUtil::newGCHandle(instance, false);
		return instance;
	}

	void ScriptManagedComponent::_clearManagedInstance()
	{
		freeManagedInstance();
	}

	ScriptObjectBackup ScriptManagedComponent::beginRefresh()
	{
		HManagedComponent managedComponent = static_object_cast<ManagedComponent>(mComponent);
		ScriptObjectBackup backupData;

		// It's possible that managed component is destroyed but a reference to it
		// is still kept. Don't backup such components.
		if (!managedComponent.isDestroyed(true))
			backupData.data = managedComponent->backup(true);

		return backupData;
	}

	void ScriptManagedComponent::endRefresh(const ScriptObjectBackup& backupData)
	{
		HManagedComponent managedComponent = static_object_cast<ManagedComponent>(mComponent);

		RawBackupData componentBackup = any_cast<RawBackupData>(backupData.data);
		managedComponent->restore(componentBackup, mTypeMissing);
	}

	void ScriptManagedComponent::_onManagedInstanceDeleted(bool assemblyRefresh)
	{
		mGCHandle = 0;

		// It's possible that managed component is destroyed but a reference to it
		// is still kept during assembly refresh. Such components shouldn't be restored
		// so we delete them.
		if (!assemblyRefresh || mComponent.isDestroyed(true))
			ScriptGameObjectManager::instance().destroyScriptComponent(this);
	}

	void ScriptManagedComponent::_notifyDestroyed()
	{
		freeManagedInstance();
	}
}