//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/B3DScriptComponent.h"
#include "Serialization/B3DScriptAssemblyManager.h"
#include "B3DMonoClass.h"
#include "B3DMonoMethod.h"
#include "B3DMonoUtil.h"
#include "Wrappers/B3DScriptSceneObject.h"
#include "B3DManagedComponent.h"
#include "Scene/B3DSceneObject.h"
#include "B3DMonoUtil.h"

using namespace b3d;

ScriptComponent::ScriptComponent(const HComponent& nativeObject)
	: TScriptGameObjectWrapper(nativeObject)
{ }

void ScriptComponent::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_AddComponent", (void*)&ScriptComponent::InternalAddComponent);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetComponent", (void*)&ScriptComponent::InternalGetComponent);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetComponents", (void*)&ScriptComponent::InternalGetComponents);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetComponentsPerType", (void*)&ScriptComponent::InternalGetComponentsPerType);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_RemoveComponent", (void*)&ScriptComponent::InternalRemoveComponent);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSceneObject", (void*)&ScriptComponent::InternalGetSceneObject);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNotifyFlags", (void*)&ScriptComponent::InternalGetNotifyFlags);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNotifyFlags", (void*)&ScriptComponent::InternalSetNotifyFlags);
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Destroy", (void*)&ScriptComponent::InternalDestroy);
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
		TGameObjectHandle<ManagedComponent> managedComponent = sceneObject->AddComponent<ManagedComponent>(type);
		return managedComponent->GetManagedInstance();
	}
	else
	{
		const ScriptTypeMetaData* const scriptWrapperObjectMetaData = scriptAssemblyManager.GetScriptWrapperMetaData(type);
		if(scriptWrapperObjectMetaData == nullptr)
			return nullptr;

		HComponent component = sceneObject->AddComponent(scriptWrapperObjectMetaData->TypeId);
		return GetOrCreateScriptObject(component);
	}
}

MonoObject* ScriptComponent::InternalGetComponent(MonoObject* parentSceneObject, MonoReflectionType* type)
{
	ScriptSceneObject* scriptSceneObjectWrapper = ScriptSceneObject::GetScriptObjectWrapper(parentSceneObject);
	if(!scriptSceneObjectWrapper->IsNativeObjectValid())
		return nullptr;

	HSceneObject sceneObject = scriptSceneObjectWrapper->GetNativeObjectAsHandle();

	::MonoClass* baseClass = MonoUtil::GetClass(type);

	const Vector<HComponent>& mComponents = sceneObject->GetComponents();
	for(auto& component : mComponents)
	{
		if(component->GetTypeId() == TID_ManagedComponent)
		{
			TGameObjectHandle<ManagedComponent> managedComponent = B3DStaticGameObjectCast<ManagedComponent>(component);

			MonoReflectionType* componentReflType = managedComponent->GetRuntimeType();
			::MonoClass* componentClass = MonoUtil::GetClass(componentReflType);

			if(MonoUtil::IsSubClassOf(componentClass, baseClass))
			{
				return managedComponent->GetManagedInstance();
			}
		}
		else
		{
			ScriptAssemblyManager& scriptAssemblyManager = ScriptAssemblyManager::Instance();
			const ScriptTypeMetaData* const scriptWrapperObjectMetaData = scriptAssemblyManager.GetScriptWrapperMetaData(type);
			if(scriptWrapperObjectMetaData == nullptr)
				continue;

			if(scriptWrapperObjectMetaData->TypeId == component->GetTypeId())
				return GetOrCreateScriptObject(component);
		}
	}

	return nullptr;
}

MonoArray* ScriptComponent::InternalGetComponentsPerType(MonoObject* parentSceneObject, MonoReflectionType* type)
{
	ScriptSceneObject* scriptSceneObjectWrapper = ScriptSceneObject::GetScriptObjectWrapper(parentSceneObject);

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
				TGameObjectHandle<ManagedComponent> managedComponent = B3DStaticGameObjectCast<ManagedComponent>(component);

				MonoReflectionType* componentReflType = managedComponent->GetRuntimeType();
				::MonoClass* componentClass = MonoUtil::GetClass(componentReflType);

				if(MonoUtil::IsSubClassOf(componentClass, baseClass))
					managedComponents.push_back(managedComponent->GetManagedInstance());
			}
			else
			{
				ScriptAssemblyManager& scriptAssemblyManager = ScriptAssemblyManager::Instance();
				const ScriptTypeMetaData* const scriptWrapperObjectMetaData = scriptAssemblyManager.GetScriptWrapperMetaData(type);
				if(scriptWrapperObjectMetaData == nullptr)
					continue;

				if(scriptWrapperObjectMetaData->TypeId == component->GetTypeId())
					managedComponents.push_back(GetOrCreateScriptObject(component));
			}
		}
	}

	ScriptArray scriptArray(sInteropMetaData.ScriptClass->GetInternalClass(), (u32)managedComponents.size());
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
				TGameObjectHandle<ManagedComponent> managedComponent = B3DStaticGameObjectCast<ManagedComponent>(component);

				managedComponents.push_back(managedComponent->GetManagedInstance());
			}
			else
			{
				MonoObject* const scriptObjectComponent = GetOrCreateScriptObject(component);
				if(scriptObjectComponent != nullptr)
					managedComponents.push_back(scriptObjectComponent);
			}
		}
	}

	ScriptArray scriptArray(sInteropMetaData.ScriptClass->GetInternalClass(), (u32)managedComponents.size());
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

	::MonoClass* baseClass = MonoUtil::GetClass(type);

	const Vector<HComponent>& components = sceneObject->GetComponents();
	for(auto& component : components)
	{
		if(component->GetTypeId() == TID_ManagedComponent)
		{
			TGameObjectHandle<ManagedComponent> managedComponent = B3DStaticGameObjectCast<ManagedComponent>(component);

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
			ScriptAssemblyManager& scriptAssemblyManager = ScriptAssemblyManager::Instance();
			const ScriptTypeMetaData* const scriptWrapperObjectMetaData = scriptAssemblyManager.GetScriptWrapperMetaData(type);
			if(scriptWrapperObjectMetaData == nullptr)
				continue;

			if(scriptWrapperObjectMetaData->TypeId == component->GetTypeId())
			{
				component->Destroy();
				return;
			}
		}
	}

	B3D_LOG(Warning, LogScene, "Attempting to remove a component that doesn't exists on SceneObject \"{0}\"", sceneObject->GetName());
}

MonoObject* ScriptComponent::InternalGetSceneObject(ScriptGameObjectWrapper* self)
{
	if(!self->IsNativeObjectValid())
		return nullptr;

	HComponent component = B3DStaticGameObjectCast<Component>(self->GetBaseNativeObjectAsHandle());
	HSceneObject sceneObject = component->SceneObject();
	return ScriptSceneObject::GetOrCreateScriptObject(sceneObject);
}

TransformChangedFlags ScriptComponent::InternalGetNotifyFlags(ScriptGameObjectWrapper* self)
{
	if(self->IsNativeObjectValid())
	{
		HComponent component = B3DStaticGameObjectCast<Component>(self->GetBaseNativeObjectAsHandle());
		return component->GetNotifyFlags();
	}

	return TCF_None;
}

void ScriptComponent::InternalSetNotifyFlags(ScriptGameObjectWrapper* self, TransformChangedFlags flags)
{
	if(!self->IsNativeObjectValid())
		return;

	HComponent component = B3DStaticGameObjectCast<Component>(self->GetBaseNativeObjectAsHandle());
	component->SetNotifyFlags(flags);
}

void ScriptComponent::InternalDestroy(ScriptGameObjectWrapper* self, bool immediate)
{
	if(!self->IsNativeObjectValid())
		return;

	HComponent component = B3DStaticGameObjectCast<Component>(self->GetBaseNativeObjectAsHandle());
	component->Destroy(immediate);
}
