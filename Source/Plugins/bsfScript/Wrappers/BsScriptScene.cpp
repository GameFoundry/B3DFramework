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

	ScriptScene::OnUpdateThunkDef ScriptScene::onUpdateThunk;

	ScriptScene::ScriptScene(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptScene::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_SetActiveScene", (void*)&ScriptScene::internal_SetActiveScene);
		metaData.scriptClass->addInternalCall("Internal_GetRoot", (void*)&ScriptScene::internal_GetRoot);
		metaData.scriptClass->addInternalCall("Internal_ClearScene", (void*)&ScriptScene::internal_ClearScene);
		metaData.scriptClass->addInternalCall("Internal_GetMainCameraSO", (void*)&ScriptScene::internal_GetMainCameraSO);

		MonoMethod* updateMethod = metaData.scriptClass->getMethod("OnUpdate");
		onUpdateThunk = (OnUpdateThunkDef)updateMethod->getThunk();
	}

	void ScriptScene::startUp()
	{
		OnRefreshStartedConn = ScriptObjectManager::instance().onRefreshStarted.connect(&onRefreshStarted);
		OnRefreshDomainLoadedConn = ScriptObjectManager::instance().onRefreshDomainLoaded.connect(&onRefreshDomainLoaded);
	}

	void ScriptScene::shutDown()
	{
		OnRefreshStartedConn.disconnect();
		OnRefreshDomainLoadedConn.disconnect();
	}

	void ScriptScene::update()
	{
		MonoUtil::invokeThunk(onUpdateThunk);
	}

	void ScriptScene::setActiveScene(const HPrefab& prefab)
	{
		if (prefab.isLoaded(false))
		{
			// If scene replace current root node, otherwise just append to the current root node
			if (prefab->isScene())
			{
				HSceneObject root = prefab->instantiate();
				gSceneManager().setRootNode(root);
			}
			else
			{
				gSceneManager().clearScene();
				prefab->instantiate();
			}
		}
		else
		{
			LOGERR("Attempting to activate a scene that hasn't finished loading yet.");
		}
	}

	void ScriptScene::internal_SetActiveScene(ScriptPrefab* scriptPrefab)
	{
		HPrefab prefab = scriptPrefab->getHandle();
		setActiveScene(prefab);
	}

	void ScriptScene::onRefreshStarted()
	{
		MonoMethod* uuidMethod = metaData.scriptClass->getMethod("GetSceneUUID");
		if (uuidMethod != nullptr)
			sActiveSceneUUID = ScriptUUID::unbox(uuidMethod->invoke(nullptr, nullptr));

		MonoMethod* nameMethod = metaData.scriptClass->getMethod("GetSceneName");
		if (nameMethod != nullptr)
			sActiveSceneName = MonoUtil::monoToString((MonoString*)nameMethod->invoke(nullptr, nullptr));

		MonoMethod* genericPrefabMethod = metaData.scriptClass->getMethod("GetIsGenericPrefab");
		if (genericPrefabMethod != nullptr)
			sIsGenericPrefab = *(bool*)MonoUtil::unbox(genericPrefabMethod->invoke(nullptr, nullptr));
	}

	void ScriptScene::onRefreshDomainLoaded()
	{
		MonoMethod* uuidMethod = metaData.scriptClass->getMethod("SetSceneUUID", 1);
		if (uuidMethod != nullptr)
		{
			void* params[1];
			params[0] = ScriptUUID::box(sActiveSceneUUID);

			uuidMethod->invoke(nullptr, params);
		}
			
		MonoMethod* nameMethod = metaData.scriptClass->getMethod("SetSceneName", 1);
		if (nameMethod != nullptr)
		{
			void* params[1];
			params[0] = MonoUtil::stringToMono(sActiveSceneName);

			nameMethod->invoke(nullptr, params);
		}

		MonoMethod* genericPrefabMethod = metaData.scriptClass->getMethod("SetIsGenericPrefab", 1);
		if (genericPrefabMethod != nullptr)
		{
			void* params[1] = { &sIsGenericPrefab };
			genericPrefabMethod->invoke(nullptr, params);
		}
	}

	MonoObject* ScriptScene::internal_GetRoot()
	{
		HSceneObject root = SceneManager::instance().getRootNode();

		ScriptSceneObject* scriptRoot = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(root);
		return scriptRoot->getManagedInstance();
	}

	void ScriptScene::internal_ClearScene()
	{
		gSceneManager().clearScene();
	}

	MonoObject* ScriptScene::internal_GetMainCameraSO()
	{
		SPtr<Camera> camera = gSceneManager().getMainCamera();
		HSceneObject so = gSceneManager()._getActorSO(camera);
		if (so == nullptr)
			return nullptr;

		ScriptSceneObject* cameraSo = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(so);
		return cameraSo->getManagedInstance();
	}
}