//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSceneManager.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Scene/BsSceneManager.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "BsScriptSceneInstance.generated.h"
#include "Wrappers/BsScriptPrefab.h"
#include "BsScriptResourceWrapper.h"

namespace bs
{
	ScriptSceneManager::OnMainSceneLoadedThunkDefinition ScriptSceneManager::OnMainSceneLoadedThunk; 
	ScriptSceneManager::OnMainSceneUnloadedThunkDefinition ScriptSceneManager::OnMainSceneUnloadedThunk; 

	HEvent ScriptSceneManager::OnMainSceneLoadedConnection;
	HEvent ScriptSceneManager::OnMainSceneUnloadedConnection;

	ScriptSceneManager::ScriptSceneManager()
		:TScriptTypeDefinition()
	{
	}

	void ScriptSceneManager::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMainScene", (void*)&ScriptSceneManager::InternalGetMainScene);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ClearMainScene", (void*)&ScriptSceneManager::InternalClearMainScene);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_LoadMainScene", (void*)&ScriptSceneManager::InternalLoadMainScene);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMainCameraSceneObject", (void*)&ScriptSceneManager::InternalGetMainCameraSceneObject);

		OnMainSceneLoadedThunk = (OnMainSceneLoadedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnMainSceneLoaded", "UUID&")->GetThunk();
		OnMainSceneUnloadedThunk = (OnMainSceneUnloadedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnMainSceneUnloaded", "UUID&")->GetThunk();
	}

	void ScriptSceneManager::StartUp()
	{
		OnMainSceneLoadedConnection = SceneManager::Instance().OnMainSceneLoaded.Connect(&ScriptSceneManager::OnMainSceneLoaded);
		OnMainSceneUnloadedConnection = SceneManager::Instance().OnMainSceneUnloaded.Connect(&ScriptSceneManager::OnMainSceneUnloaded);
	}
	void ScriptSceneManager::ShutDown()
	{
		OnMainSceneLoadedConnection.Disconnect();
		OnMainSceneUnloadedConnection.Disconnect();
	}

	void ScriptSceneManager::OnMainSceneLoaded(UUID p0)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptUUID::Box(p0);
		MonoUtil::InvokeThunk(OnMainSceneLoadedThunk, tmpp0);
	}

	void ScriptSceneManager::OnMainSceneUnloaded(UUID p0)
	{
		MonoObject* tmpp0;
		tmpp0 = ScriptUUID::Box(p0);
		MonoUtil::InvokeThunk(OnMainSceneUnloadedThunk, tmpp0);
	}

	MonoObject* ScriptSceneManager::InternalGetMainScene()
	{
		SPtr<SceneInstance> tmp__output;
		tmp__output = SceneManager::Instance().GetMainScene();

		MonoObject* __output;
		__output = ScriptSceneInstance::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	void ScriptSceneManager::InternalClearMainScene(bool forceAll)
	{
		SceneManager::Instance().ClearMainScene(forceAll);
	}

	void ScriptSceneManager::InternalLoadMainScene(MonoObject* scene)
	{
		TResourceHandle<Prefab> tmpscene;
		ScriptPrefab* scriptObjectWrapperscene;
		scriptObjectWrapperscene = ScriptPrefab::GetScriptObjectWrapper(scene);
		if(scriptObjectWrapperscene != nullptr)
			tmpscene = B3DStaticResourceCast<Prefab>(scriptObjectWrapperscene->GetBaseNativeObjectAsHandle());
		SceneManager::Instance().LoadMainScene(tmpscene);
	}

	MonoObject* ScriptSceneManager::InternalGetMainCameraSceneObject()
	{
		GameObjectHandle<SceneObject> tmp__output;
		tmp__output = SceneManager::Instance().GetMainCameraSceneObject();

		MonoObject* __output;
		MonoObject* temp__output = nullptr;
		if(tmp__output)
		temp__output = ScriptSceneObject::GetOrCreateScriptObject(tmp__output);
		__output = temp__output;

		return __output;
	}
}
