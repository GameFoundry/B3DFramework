//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPrefab.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Scene/BsPrefab.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "BsScriptSceneInstance.generated.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "../../../Foundation/bsfCore/Scene/BsPrefab.h"

namespace bs
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
		GameObjectHandle<SceneObject> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		SPtr<SceneInstance> tmpsceneInstance;
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
		GameObjectHandle<SceneObject> tmpsceneObject;
		ScriptSceneObject* scriptObjectWrappersceneObject;
		scriptObjectWrappersceneObject = ScriptSceneObject::GetScriptObjectWrapper(sceneObject);
		if(scriptObjectWrappersceneObject != nullptr)
			tmpsceneObject = B3DStaticGameObjectCast<SceneObject>(scriptObjectWrappersceneObject->GetBaseNativeObjectAsHandle());
		TResourceHandle<Prefab> nativeObject = Prefab::Create(tmpsceneObject);
		ScriptObjectWrapper::Create<ScriptPrefab>(nativeObject, scriptObject);
	}
}
