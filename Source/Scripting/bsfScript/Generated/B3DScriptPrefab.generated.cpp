//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptPrefab.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Scene/B3DPrefab.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "B3DScriptSceneInstance.generated.h"
#include "Wrappers/B3DScriptSceneObject.h"
#include "../../../Engine/Core/Scene/B3DPrefab.h"

namespace b3d
{
	ScriptPrefab::ScriptPrefab(const TResourceHandle<Prefab>& nativeObject)
		:TScriptResourceWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptPrefab::~ScriptPrefab()
	{
		UnregisterEvents();
	}

	void ScriptPrefab::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRef", (void*)&ScriptPrefab::InternalGetRef);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Instantiate", (void*)&ScriptPrefab::InternalInstantiate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptPrefab::InternalCreate);

	}

	MonoObject* ScriptPrefab::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoObject* ScriptPrefab::InternalGetRef(ScriptPrefab* self)
	{
		return self->GetOrCreateResourceReference();
	}

	MonoObject* ScriptPrefab::InternalInstantiate(ScriptPrefab* self, MonoObject* sceneInstance)
	{
		TGameObjectHandle<SceneObject> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		TShared<SceneInstance> tmpsceneInstance;
		ScriptSceneInstance* scriptObjectWrappersceneInstance;
		scriptObjectWrappersceneInstance = ScriptSceneInstance::GetScriptObjectWrapper(sceneInstance);
		if(scriptObjectWrappersceneInstance != nullptr)
			tmpsceneInstance = std::static_pointer_cast<SceneInstance>(scriptObjectWrappersceneInstance->GetBaseNativeObjectAsShared());
		tmp__output = static_cast<Prefab*>(self->GetNativeObject())->Instantiate(tmpsceneInstance);

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptSceneObject::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}

	void ScriptPrefab::InternalCreate(MonoObject* scriptObject, MonoObject* sceneObject)
	{
		TGameObjectHandle<SceneObject> tmpsceneObject;
		ScriptSceneObject* scriptObjectWrappersceneObject;
		scriptObjectWrappersceneObject = ScriptSceneObject::GetScriptObjectWrapper(sceneObject);
		if(scriptObjectWrappersceneObject != nullptr)
			tmpsceneObject = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrappersceneObject->GetBaseNativeObjectAsHandle());
		TResourceHandle<Prefab> nativeObject = Prefab::Create(tmpsceneObject);
		ScriptObjectWrapper::Create<ScriptPrefab>(nativeObject, scriptObject);
	}
}
