//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptComponent.h"
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

using namespace bs;
ScriptComponentBase::ScriptComponentBase(MonoObject* instance)
	: ScriptGameObjectBase(instance)
{}

void ScriptComponentBase::Destroy(bool assemblyRefresh)
{
	// It's possible that managed component is destroyed but a reference to it is still kept during assembly refresh.
	// Such components shouldn't be restored so we delete them.

	HComponent component = GetComponent();
	if(!assemblyRefresh || component.IsDestroyed(true))
		ScriptGameObjectManager::Instance().DestroyScriptComponent(this);
}

bool ScriptComponentBase::CheckIfDestroyed(const GameObjectHandleBase& handle)
{
	if(handle.IsDestroyed())
	{
		B3D_LOG(Warning, Scene, "Trying to access a destroyed GameObject with ID: {0}", handle.GetId());
		return true;
	}

	return false;
}

ScriptComponent::ScriptComponent(MonoObject* instance)
	: ScriptObject(instance)
{
	B3D_ASSERT(instance != nullptr);
}

void ScriptComponent::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_AddComponent", (void*)&ScriptComponent::InternalAddComponent);
	metaData.ScriptClass->AddInternalCall("Internal_GetComponent", (void*)&ScriptComponent::InternalGetComponent);
	metaData.ScriptClass->AddInternalCall("Internal_GetComponents", (void*)&ScriptComponent::InternalGetComponents);
	metaData.ScriptClass->AddInternalCall("Internal_GetComponentsPerType", (void*)&ScriptComponent::InternalGetComponentsPerType);
	metaData.ScriptClass->AddInternalCall("Internal_RemoveComponent", (void*)&ScriptComponent::InternalRemoveComponent);
	metaData.ScriptClass->AddInternalCall("Internal_GetSceneObject", (void*)&ScriptComponent::InternalGetSceneObject);
	metaData.ScriptClass->AddInternalCall("Internal_GetNotifyFlags", (void*)&ScriptComponent::InternalGetNotifyFlags);
	metaData.ScriptClass->AddInternalCall("Internal_SetNotifyFlags", (void*)&ScriptComponent::InternalSetNotifyFlags);
	metaData.ScriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptComponent::InternalDestroy);
}

MonoObject* ScriptComponent::InternalAddComponent(MonoObject* parentSceneObject, MonoReflectionType* type)
{
	ScriptSceneObject* scriptSceneObjectWrapper = ScriptSceneObject::GetScriptObjectWrapper(parentSceneObject);
	if(!scriptSceneObjectWrapper->IsNativeObjectValid())
		return nullptr;

	HSceneObject sceneObject = scriptSceneObjectWrapper->GetNativeObjectAsHandle();

	ScriptAssemblyManager& scriptAssemblyManager = ScriptAssemblyManager::Instance();

	MonoClass* managedComponent = scriptAssemblyManager.GetBuiltinClasses().ManagedComponentClass;
	::MonoClass* requestedClass = MonoUtil::GetClass(type);

	const bool isManagedComponent = MonoUtil::IsSubClassOf(requestedClass, managedComponent->GetInternalClass());
	if(isManagedComponent)
	{
		GameObjectHandle<ManagedComponent> managedComponent = sceneObject->AddComponent<ManagedComponent>(type);
		return managedComponent->GetManagedInstance();
	}
	else
	{
		BuiltinComponentInfo* info = scriptAssemblyManager.GetBuiltinComponentInfo(type);
		if(info == nullptr)
			return nullptr;

		HComponent component = sceneObject->AddComponent(info->TypeId);
		ScriptComponentBase* scriptComponent =
			ScriptGameObjectManager::Instance().CreateBuiltinScriptComponent(component);

		return scriptComponent->GetManagedInstance();
	}
}

MonoObject* ScriptComponent::InternalGetComponent(MonoObject* parentSceneObject, MonoReflectionType* type)
{
	ScriptSceneObject* scriptSceneObjectWrapper = ScriptSceneObject::GetScriptObjectWrapper(parentSceneObject);
	if(!scriptSceneObjectWrapper->IsNativeObjectValid())
		return nullptr;

	HSceneObject sceneObject = scriptSceneObjectWrapper->GetNativeObjectAsHandle();

	ScriptAssemblyManager& sam = ScriptAssemblyManager::Instance();
	BuiltinComponentInfo* info = sam.GetBuiltinComponentInfo(type);

	::MonoClass* baseClass = MonoUtil::GetClass(type);

	const Vector<HComponent>& mComponents = sceneObject->GetComponents();
	for(auto& component : mComponents)
	{
		if(component->GetTypeId() == TID_ManagedComponent)
		{
			GameObjectHandle<ManagedComponent> managedComponent = B3DStaticGameObjectCast<ManagedComponent>(component);

			MonoReflectionType* componentReflType = managedComponent->GetRuntimeType();
			::MonoClass* componentClass = MonoUtil::GetClass(componentReflType);

			if(MonoUtil::IsSubClassOf(componentClass, baseClass))
			{
				return managedComponent->GetManagedInstance();
			}
		}
		else
		{
			if(info == nullptr)
				continue;

			if(info->TypeId == component->GetTypeId())
			{
				ScriptComponentBase* scriptComponent = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(component);
				return scriptComponent->GetManagedInstance();
			}
		}
	}

	return nullptr;
}

MonoArray* ScriptComponent::InternalGetComponentsPerType(MonoObject* parentSceneObject, MonoReflectionType* type)
{
	ScriptSceneObject* scriptSceneObjectWrapper = ScriptSceneObject::GetScriptObjectWrapper(parentSceneObject);

	ScriptAssemblyManager& sam = ScriptAssemblyManager::Instance();
	BuiltinComponentInfo* info = sam.GetBuiltinComponentInfo(type);

	::MonoClass* baseClass = MonoUtil::GetClass(type);
	Vector<MonoObject*> managedComponents;

	if(scriptSceneObjectWrapper->IsNativeObjectValid())
	{
		HSceneObject sceneObject = scriptSceneObjectWrapper->GetNativeObjectAsHandle();
		const Vector<HComponent>& components = sceneObject->GetComponents();
		for(auto& component : components)
		{
			if(component->GetTypeId() == TID_ManagedComponent)
			{
				GameObjectHandle<ManagedComponent> managedComponent = B3DStaticGameObjectCast<ManagedComponent>(component);

				MonoReflectionType* componentReflType = managedComponent->GetRuntimeType();
				::MonoClass* componentClass = MonoUtil::GetClass(componentReflType);

				if(MonoUtil::IsSubClassOf(componentClass, baseClass))
					managedComponents.push_back(managedComponent->GetManagedInstance());
			}
			else
			{
				if(info == nullptr)
					continue;

				if(info->TypeId == component->GetTypeId())
				{
					ScriptComponentBase* scriptComponent = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(component);
					managedComponents.push_back(scriptComponent->GetManagedInstance());
				}
			}
		}
	}

	ScriptArray scriptArray(metaData.ScriptClass->GetInternalClass(), (u32)managedComponents.size());
	for(u32 i = 0; i < (u32)managedComponents.size(); i++)
		scriptArray.Set(i, managedComponents[i]);

	return scriptArray.GetInternal();
}

MonoArray* ScriptComponent::InternalGetComponents(MonoObject* parentSceneObject)
{
	ScriptSceneObject* scriptSceneObjectWrapper = ScriptSceneObject::GetScriptObjectWrapper(parentSceneObject);

	Vector<MonoObject*> managedComponents;

	if(scriptSceneObjectWrapper->IsNativeObjectValid())
	{
		HSceneObject sceneObject = scriptSceneObjectWrapper->GetNativeObjectAsHandle();
		const Vector<HComponent>& components = sceneObject->GetComponents();
		for(auto& component : components)
		{
			if(component->GetTypeId() == TID_ManagedComponent)
			{
				GameObjectHandle<ManagedComponent> managedComponent = B3DStaticGameObjectCast<ManagedComponent>(component);

				managedComponents.push_back(managedComponent->GetManagedInstance());
			}
			else
			{
				ScriptComponentBase* scriptComponent = ScriptGameObjectManager::Instance().GetBuiltinScriptComponent(component);
				if(scriptComponent != nullptr)
					managedComponents.push_back(scriptComponent->GetManagedInstance());
			}
		}
	}

	ScriptArray scriptArray(metaData.ScriptClass->GetInternalClass(), (u32)managedComponents.size());
	for(u32 i = 0; i < (u32)managedComponents.size(); i++)
		scriptArray.Set(i, managedComponents[i]);

	return scriptArray.GetInternal();
}

void ScriptComponent::InternalRemoveComponent(MonoObject* parentSceneObject, MonoReflectionType* type)
{
	ScriptSceneObject* scriptSceneObjectWrapper = ScriptSceneObject::GetScriptObjectWrapper(parentSceneObject);

	if(!scriptSceneObjectWrapper->IsNativeObjectValid())
		return;

	HSceneObject sceneObject = scriptSceneObjectWrapper->GetNativeObjectAsHandle();

	ScriptAssemblyManager& sam = ScriptAssemblyManager::Instance();
	BuiltinComponentInfo* info = sam.GetBuiltinComponentInfo(type);

	::MonoClass* baseClass = MonoUtil::GetClass(type);

	const Vector<HComponent>& components = sceneObject->GetComponents();
	for(auto& component : components)
	{
		if(component->GetTypeId() == TID_ManagedComponent)
		{
			GameObjectHandle<ManagedComponent> managedComponent = B3DStaticGameObjectCast<ManagedComponent>(component);

			MonoReflectionType* componentReflType = managedComponent->GetRuntimeType();
			::MonoClass* componentClass = MonoUtil::GetClass(componentReflType);

			if(MonoUtil::IsSubClassOf(componentClass, baseClass))
			{
				managedComponent->Destroy();
				return;
			}
		}
		else
		{
			if(info == nullptr)
				continue;

			if(info->TypeId == component->GetTypeId())
			{
				component->Destroy();
				return;
			}
		}
	}

	B3D_LOG(Warning, Scene, "Attempting to remove a component that doesn't exists on SceneObject \"{0}\"", sceneObject->GetName());
}

MonoObject* ScriptComponent::InternalGetSceneObject(ScriptComponentBase* nativeInstance)
{
	HComponent component = nativeInstance->GetComponent();
	if(CheckIfDestroyed(component))
		return nullptr;

	HSceneObject sceneObject = component->SceneObject();
	return ScriptSceneObject::GetOrCreateScriptObject(sceneObject);
}

TransformChangedFlags ScriptComponent::InternalGetNotifyFlags(ScriptComponentBase* nativeInstance)
{
	HComponent component = nativeInstance->GetComponent();

	if(!CheckIfDestroyed(component))
		return component->GetNotifyFlagsInternal();

	return TCF_None;
}

void ScriptComponent::InternalSetNotifyFlags(ScriptComponentBase* nativeInstance, TransformChangedFlags flags)
{
	HComponent component = nativeInstance->GetComponent();

	if(!CheckIfDestroyed(component))
		component->SetNotifyFlags(flags);
}

void ScriptComponent::InternalDestroy(ScriptComponentBase* nativeInstance, bool immediate)
{
	HComponent component = nativeInstance->GetComponent();

	if(!CheckIfDestroyed(component))
		component->Destroy(immediate);
}
