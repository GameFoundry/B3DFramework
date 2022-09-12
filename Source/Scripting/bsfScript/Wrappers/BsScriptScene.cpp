//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptScene.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Scene/BsSceneManager.h"
#include "Resources/BsResources.h"
#include "Scene/BsPrefab.h"
#include "BsApplication.h"
#include "Scene/BsSceneObject.h"
#include "Renderer/BsCamera.h"
#include "BsScriptGameObjectManager.h"
#include "Resources/BsGameResourceManager.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptPrefab.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "BsScriptObjectManager.h"

namespace bs
{
	HEvent ScriptScene::OnRefreshDomainLoadedConn;
	HEvent ScriptScene::OnRefreshStartedConn;

	UUID ScriptScene::sActiveSceneUUID;
	String ScriptScene::sActiveSceneName;
	bool ScriptScene::sIsGenericPrefab;

#if BS_IS_BANSHEE3D
	ScriptScene::OnUpdateThunkDef ScriptScene::onUpdateThunk;
#endif

	ScriptScene::ScriptScene(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptScene::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_GetRoot", (void*)&ScriptScene::internal_GetRoot);
		metaData.scriptClass->AddInternalCall("Internal_GetMainCameraSO", (void*)&ScriptScene::internal_GetMainCameraSO);

#if BS_IS_BANSHEE3D
		metaData.scriptClass->AddInternalCall("Internal_SetActiveScene", (void*)&ScriptScene::internal_SetActiveScene);
		metaData.scriptClass->AddInternalCall("Internal_ClearScene", (void*)&ScriptScene::internal_ClearScene);

		MonoMethod* updateMethod = metaData.scriptClass->GetMethod("OnUpdate");
		onUpdateThunk = (OnUpdateThunkDef)updateMethod->GetThunk();
#endif
	}

	void ScriptScene::StartUp()
	{
		OnRefreshStartedConn = ScriptObjectManager::instance().onRefreshStarted.Connect(&onRefreshStarted);
		OnRefreshDomainLoadedConn = ScriptObjectManager::instance().onRefreshDomainLoaded.Connect(&onRefreshDomainLoaded);
	}

	void ScriptScene::ShutDown()
	{
		OnRefreshStartedConn.Disconnect();
		OnRefreshDomainLoadedConn.Disconnect();
	}

	void ScriptScene::Update()
	{
#if BS_IS_BANSHEE3D
		MonoUtil::invokeThunk(onUpdateThunk);
#endif
	}

	void ScriptScene::SetActiveScene(const HPrefab& prefab)
	{
		if (prefab.IsLoaded(false))
		{
			// If scene replace current root node, otherwise just append to the current root node
			if (prefab->IsScene())
				gSceneManager().LoadScene(prefab);
			else
			{
				gSceneManager().ClearScene();
				prefab->Instantiate();
			}
		}
		else
		{
			BS_LOG(Error, Scene, "Attempting to activate a scene that hasn't finished loading yet.");
		}
	}

	void ScriptScene::OnRefreshStarted()
	{
		MonoMethod* uuidMethod = metaData.scriptClass->GetMethod("GetSceneUUID");
		if (uuidMethod != nullptr)
			sActiveSceneUUID = ScriptUUID::unbox(uuidMethod->Invoke(nullptr, nullptr));

		MonoMethod* nameMethod = metaData.scriptClass->GetMethod("GetSceneName");
		if (nameMethod != nullptr)
			sActiveSceneName = MonoUtil::monoToString((MonoString*)nameMethod->Invoke(nullptr, nullptr));

		MonoMethod* genericPrefabMethod = metaData.scriptClass->GetMethod("GetIsGenericPrefab");
		if (genericPrefabMethod != nullptr)
			sIsGenericPrefab = *(bool*)MonoUtil::unbox(genericPrefabMethod->Invoke(nullptr, nullptr));
	}

	void ScriptScene::OnRefreshDomainLoaded()
	{
		MonoMethod* uuidMethod = metaData.scriptClass->GetMethod("SetSceneUUID", 1);
		if (uuidMethod != nullptr)
		{
			void* params[1];
			params[0] = ScriptUUID::box(sActiveSceneUUID);

			uuidMethod->Invoke(nullptr, params);
		}
			
		MonoMethod* nameMethod = metaData.scriptClass->GetMethod("SetSceneName", 1);
		if (nameMethod != nullptr)
		{
			void* params[1];
			params[0] = MonoUtil::stringToMono(sActiveSceneName);

			nameMethod->Invoke(nullptr, params);
		}

		MonoMethod* genericPrefabMethod = metaData.scriptClass->GetMethod("SetIsGenericPrefab", 1);
		if (genericPrefabMethod != nullptr)
		{
			void* params[1] = { &sIsGenericPrefab };
			genericPrefabMethod->Invoke(nullptr, params);
		}
	}

	MonoObject* ScriptScene::internal_GetRoot()
	{
		HSceneObject root = SceneManager::instance().GetMainScene()->GetRoot();

		ScriptSceneObject* scriptRoot = ScriptGameObjectManager::instance().GetOrCreateScriptSceneObject(root);
		return scriptRoot->GetManagedInstance();
	}

	MonoObject* ScriptScene::internal_GetMainCameraSO()
	{
		SPtr<Camera> camera = gSceneManager().GetMainCamera();
		HSceneObject so = gSceneManager()._getActorSO(camera);
		if (so == nullptr)
			return nullptr;

		ScriptSceneObject* cameraSo = ScriptGameObjectManager::instance().GetOrCreateScriptSceneObject(so);
		return cameraSo->GetManagedInstance();
	}

#if BS_IS_BANSHEE3D
	void ScriptScene::internal_SetActiveScene(ScriptPrefab* scriptPrefab)
	{
		HPrefab prefab = scriptPrefab->GetHandle();
		setActiveScene(prefab);
	}

	void ScriptScene::internal_ClearScene()
	{
		gSceneManager().ClearScene();
	}
#endif
}
