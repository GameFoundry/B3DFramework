//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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

	UUID ScriptScene::ActiveSceneUUID;
	WString ScriptScene::ActiveSceneName;
	bool ScriptScene::IsGenericPrefab;

	ScriptScene::ScriptScene(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptScene::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_LoadScene", (void*)&ScriptScene::internal_LoadScene);
		metaData.scriptClass->addInternalCall("Internal_GetRoot", (void*)&ScriptScene::internal_GetRoot);
		metaData.scriptClass->addInternalCall("Internal_ClearScene", (void*)&ScriptScene::internal_ClearScene);
		metaData.scriptClass->addInternalCall("Internal_GetMainCameraSO", (void*)&ScriptScene::internal_GetMainCameraSO);
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

	MonoObject* ScriptScene::internal_LoadScene(MonoString* path)
	{
		Path nativePath = MonoUtil::monoToWString(path);

		HPrefab prefab = GameResourceManager::instance().load<Prefab>(nativePath, true);
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

			ScriptResourceBase* scriptPrefab = ScriptResourceManager::instance().getScriptResource(prefab, true);
			return scriptPrefab->getManagedInstance();
		}
		else
		{
			LOGERR("Failed loading scene at path: \"" + nativePath.toString() + "\"");
			return nullptr;
		}
	}

	void ScriptScene::onRefreshStarted()
	{
		MonoMethod* uuidMethod = metaData.scriptClass->getMethod("GetSceneUUID");
		if (uuidMethod != nullptr)
			ActiveSceneUUID = ScriptUUID::unbox(uuidMethod->invoke(nullptr, nullptr));

		MonoMethod* nameMethod = metaData.scriptClass->getMethod("GetSceneName");
		if (nameMethod != nullptr)
			ActiveSceneName = MonoUtil::monoToWString((MonoString*)nameMethod->invoke(nullptr, nullptr));

		MonoMethod* genericPrefabMethod = metaData.scriptClass->getMethod("GetIsGenericPrefab");
		if (genericPrefabMethod != nullptr)
			IsGenericPrefab = *(bool*)MonoUtil::unbox(genericPrefabMethod->invoke(nullptr, nullptr));
	}

	void ScriptScene::onRefreshDomainLoaded()
	{
		MonoMethod* uuidMethod = metaData.scriptClass->getMethod("SetSceneUUID", 1);
		if (uuidMethod != nullptr)
		{
			void* params[1];
			params[0] = ScriptUUID::box(ActiveSceneUUID);

			uuidMethod->invoke(nullptr, params);
		}
			
		MonoMethod* nameMethod = metaData.scriptClass->getMethod("SetSceneName", 1);
		if (nameMethod != nullptr)
		{
			void* params[1];
			params[0] = MonoUtil::wstringToMono(ActiveSceneName);

			nameMethod->invoke(nullptr, params);
		}

		MonoMethod* genericPrefabMethod = metaData.scriptClass->getMethod("SetIsGenericPrefab", 1);
		if (genericPrefabMethod != nullptr)
		{
			void* params[1] = { &IsGenericPrefab };
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