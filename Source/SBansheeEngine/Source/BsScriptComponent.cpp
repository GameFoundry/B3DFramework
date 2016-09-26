//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptComponent.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptObjectManager.h"
#include "BsScriptAssemblyManager.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"
#include "BsScriptSceneObject.h"
#include "BsManagedComponent.h"
#include "BsSceneObject.h"
#include "BsMonoUtil.h"

namespace BansheeEngine
{
	ScriptComponent::ScriptComponent(MonoObject* instance)
		:ScriptObject(instance), mTypeMissing(false)
	{ 
		assert(instance != nullptr);

		MonoUtil::getClassName(instance, mNamespace, mType);
	}

	void ScriptComponent::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_AddComponent", &ScriptComponent::internal_addComponent);
		metaData.scriptClass->addInternalCall("Internal_GetComponent", &ScriptComponent::internal_getComponent);
		metaData.scriptClass->addInternalCall("Internal_GetComponents", &ScriptComponent::internal_getComponents);
		metaData.scriptClass->addInternalCall("Internal_GetComponentsPerType", &ScriptComponent::internal_getComponentsPerType);
		metaData.scriptClass->addInternalCall("Internal_RemoveComponent", &ScriptComponent::internal_removeComponent);
		metaData.scriptClass->addInternalCall("Internal_GetSceneObject", &ScriptComponent::internal_getSceneObject);
		metaData.scriptClass->addInternalCall("Internal_GetNotifyFlags", &ScriptComponent::internal_getNotifyFlags);
		metaData.scriptClass->addInternalCall("Internal_SetNotifyFlags", &ScriptComponent::internal_setNotifyFlags);
		metaData.scriptClass->addInternalCall("Internal_Invoke", &ScriptComponent::internal_invoke);
		metaData.scriptClass->addInternalCall("Internal_Destroy", &ScriptComponent::internal_destroy);
	}

	MonoObject* ScriptComponent::internal_addComponent(MonoObject* parentSceneObject, MonoReflectionType* type)
	{
		ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(parentSceneObject);
		HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

		if (checkIfDestroyed(so))
			return nullptr;

		GameObjectHandle<ManagedComponent> mc = so->addComponent<ManagedComponent>(type);
		return mc->getManagedInstance();
	}

	MonoObject* ScriptComponent::internal_getComponent(MonoObject* parentSceneObject, MonoReflectionType* type)
	{
		ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(parentSceneObject);
		HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

		if (checkIfDestroyed(so))
			return nullptr;

		::MonoClass* baseClass = MonoUtil::getClass(type);

		const Vector<HComponent>& mComponents = so->getComponents();
		for(auto& component : mComponents)
		{
			if(component->getTypeId() == TID_ManagedComponent)
			{
				GameObjectHandle<ManagedComponent> managedComponent = static_object_cast<ManagedComponent>(component);

				MonoReflectionType* componentReflType = managedComponent->getRuntimeType();
				::MonoClass* componentClass = MonoUtil::getClass(componentReflType);
				
				if(MonoUtil::isSubClassOf(componentClass, baseClass))
				{
					return managedComponent->getManagedInstance();
				}
			}
		}

		return nullptr;
	}

	MonoArray* ScriptComponent::internal_getComponentsPerType(MonoObject* parentSceneObject, MonoReflectionType* type)
	{
		ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(parentSceneObject);
		HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

		::MonoClass* baseClass = MonoUtil::getClass(type);
		Vector<MonoObject*> managedComponents;

		if (!checkIfDestroyed(so))
		{
			const Vector<HComponent>& mComponents = so->getComponents();
			for (auto& component : mComponents)
			{
				if (component->getTypeId() == TID_ManagedComponent)
				{
					GameObjectHandle<ManagedComponent> managedComponent = static_object_cast<ManagedComponent>(component);

					MonoReflectionType* componentReflType = managedComponent->getRuntimeType();
					::MonoClass* componentClass = MonoUtil::getClass(componentReflType);

					if (MonoUtil::isSubClassOf(componentClass, baseClass))
						managedComponents.push_back(managedComponent->getManagedInstance());
				}
			}
		}

		ScriptArray scriptArray(metaData.scriptClass->_getInternalClass(), (UINT32)managedComponents.size());
		for (UINT32 i = 0; i < (UINT32)managedComponents.size(); i++)
			scriptArray.set(i, managedComponents[i]);

		return scriptArray.getInternal();
	}

	MonoArray* ScriptComponent::internal_getComponents(MonoObject* parentSceneObject)
	{
		ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(parentSceneObject);
		HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

		Vector<MonoObject*> managedComponents;

		if (!checkIfDestroyed(so))
		{
			const Vector<HComponent>& mComponents = so->getComponents();
			for (auto& component : mComponents)
			{
				if (component->getTypeId() == TID_ManagedComponent)
				{
					GameObjectHandle<ManagedComponent> managedComponent = static_object_cast<ManagedComponent>(component);

					managedComponents.push_back(managedComponent->getManagedInstance());
				}
			}
		}

		ScriptArray scriptArray(metaData.scriptClass->_getInternalClass(), (UINT32)managedComponents.size());
		for(UINT32 i = 0; i < (UINT32)managedComponents.size(); i++)
			scriptArray.set(i, managedComponents[i]);

		return scriptArray.getInternal();
	}

	void ScriptComponent::internal_removeComponent(MonoObject* parentSceneObject, MonoReflectionType* type)
	{
		ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(parentSceneObject);
		HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

		if (checkIfDestroyed(so))
			return;

		::MonoClass* baseClass = MonoUtil::getClass(type);

		const Vector<HComponent>& mComponents = so->getComponents();
		for(auto& component : mComponents)
		{
			if(component->getTypeId() == TID_ManagedComponent)
			{
				GameObjectHandle<ManagedComponent> managedComponent = static_object_cast<ManagedComponent>(component);

				MonoReflectionType* componentReflType = managedComponent->getRuntimeType();
				::MonoClass* componentClass = MonoUtil::getClass(componentReflType);

				if (MonoUtil::isSubClassOf(componentClass, baseClass))
				{
					managedComponent->destroy();
					return;
				}
			}
		}

		LOGWRN("Attempting to remove a component that doesn't exists on SceneObject \"" + so->getName() + "\"");
	}

	MonoObject* ScriptComponent::internal_getSceneObject(ScriptComponent* nativeInstance)
	{
		if (checkIfDestroyed(nativeInstance->mManagedComponent))
			return nullptr;

		HSceneObject sceneObject = nativeInstance->mManagedComponent->sceneObject();

		ScriptSceneObject* scriptSO = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(sceneObject);

		assert(scriptSO->getManagedInstance() != nullptr);
		return scriptSO->getManagedInstance();
	}

	TransformChangedFlags ScriptComponent::internal_getNotifyFlags(ScriptComponent* nativeInstance)
	{
		if (!checkIfDestroyed(nativeInstance->mManagedComponent))
			return nativeInstance->mManagedComponent->mNotifyFlags;

		return TCF_None;
	}

	void ScriptComponent::internal_setNotifyFlags(ScriptComponent* nativeInstance, TransformChangedFlags flags)
	{
		if (!checkIfDestroyed(nativeInstance->mManagedComponent))
			nativeInstance->mManagedComponent->mNotifyFlags = flags;
	}

	void ScriptComponent::internal_invoke(ScriptComponent* nativeInstance, MonoString* name)
	{
		HManagedComponent comp = nativeInstance->mManagedComponent;
		if (checkIfDestroyed(nativeInstance->mManagedComponent))
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
			LOGWRN("Method invoke failed. Cannot find method \"" + methodName + "\" on component of type \"" + 
				compClass->getTypeName() + "\".");
		}
	}

	void ScriptComponent::internal_destroy(ScriptComponent* nativeInstance, bool immediate)
	{
		if (!checkIfDestroyed(nativeInstance->mManagedComponent))
			nativeInstance->mManagedComponent->destroy(immediate);
	}

	bool ScriptComponent::checkIfDestroyed(const GameObjectHandleBase& handle)
	{
		if (handle.isDestroyed())
		{
			LOGWRN("Trying to access a destroyed GameObject with instance ID: " + toString(handle.getInstanceId()));
			return true;
		}

		return false;
	}

	MonoObject* ScriptComponent::_createManagedInstance(bool construct)
	{
		SPtr<ManagedSerializableObjectInfo> currentObjInfo = nullptr;

		// See if this type even still exists
		if (!ScriptAssemblyManager::instance().getSerializableObjectInfo(mNamespace, mType, currentObjInfo))
		{
			mTypeMissing = true;
			return ScriptAssemblyManager::instance().getMissingComponentClass()->createInstance(true);
		}

		mTypeMissing = false;
		return currentObjInfo->mMonoClass->createInstance(construct);
	}

	ScriptObjectBackup ScriptComponent::beginRefresh()
	{
		ScriptGameObjectBase::beginRefresh();

		ScriptObjectBackup backupData;

		// It's possible that managed component is destroyed but a reference to it
		// is still kept. Don't backup such components.
		if (!mManagedComponent.isDestroyed(true))
			backupData.data = mManagedComponent->backup(true);

		return backupData;
	}

	void ScriptComponent::endRefresh(const ScriptObjectBackup& backupData)
	{
		ComponentBackupData componentBackup = any_cast<ComponentBackupData>(backupData.data);
		mManagedComponent->restore(mManagedInstance, componentBackup, mTypeMissing);

		ScriptGameObjectBase::endRefresh(backupData);
	}

	void ScriptComponent::_onManagedInstanceDeleted()
	{
		mManagedInstance = nullptr;

		// It's possible that managed component is destroyed but a reference to it
		// is still kept during assembly refresh. Such components shouldn't be restored
		// so we delete them.
		if (!mRefreshInProgress || mManagedComponent.isDestroyed(true))
			ScriptGameObjectManager::instance().destroyScriptComponent(this);
	}

	void ScriptComponent::setNativeHandle(const HGameObject& gameObject)
	{
		mManagedComponent = static_object_cast<ManagedComponent>(gameObject);
	}
}