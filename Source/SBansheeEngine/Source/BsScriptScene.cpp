//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptScene.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsSceneManager.h"
#include "BsResources.h"
#include "BsPrefab.h"
#include "BsApplication.h"
#include "BsSceneObject.h"
#include "BsScriptGameObjectManager.h"
#include "BsGameResourceManager.h"
#include "BsScriptResourceManager.h"
#include "BsScriptPrefab.h"
#include "BsScriptSceneObject.h"
#include "BsScriptObjectManager.h"

namespace bs
{
	HEvent ScriptScene::OnRefreshDomainLoadedConn;
	HEvent ScriptScene::OnRefreshStartedConn;

	String ScriptScene::ActiveSceneUUID;
	WString ScriptScene::ActiveSceneName;
	bool ScriptScene::IsGenericPrefab;

	ScriptScene::ScriptScene(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptScene::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_LoadScene", &ScriptScene::internal_LoadScene);
		metaData.scriptClass->addInternalCall("Internal_GetRoot", &ScriptScene::internal_GetRoot);
		metaData.scriptClass->addInternalCall("Internal_ClearScene", &ScriptScene::internal_ClearScene);
		metaData.scriptClass->addInternalCall("Internal_GetMainCameraSO", &ScriptScene::internal_GetMainCameraSO);
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
				gSceneManager()._setRootNode(root);
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
			ActiveSceneUUID = MonoUtil::monoToString((MonoString*)uuidMethod->invoke(nullptr, nullptr));

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
			params[0] = MonoUtil::stringToMono(ActiveSceneUUID);

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
		SceneCameraData cameraData = gSceneManager().getMainCamera();
		if (cameraData.sceneObject == nullptr)
			return nullptr;

		ScriptSceneObject* cameraSo = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(cameraData.sceneObject);
		return cameraSo->getManagedInstance();
	}
}