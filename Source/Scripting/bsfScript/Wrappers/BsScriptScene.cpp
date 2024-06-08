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
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptPrefab.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "BsScriptObjectManager.h"

using namespace bs;
HEvent ScriptScene::OnRefreshDomainLoadedConn;
HEvent ScriptScene::OnRefreshStartedConn;

UUID ScriptScene::sActiveSceneUUID;
String ScriptScene::sActiveSceneName;
bool ScriptScene::sIsGenericPrefab;

#if B3D_IS_ENGINE
ScriptScene::OnUpdateThunkDef ScriptScene::onUpdateThunk;
#endif

ScriptScene::ScriptScene(MonoObject* instance)
	: ScriptObject(instance)
{}

void ScriptScene::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_GetRoot", (void*)&ScriptScene::InternalGetRoot);
	metaData.ScriptClass->AddInternalCall("Internal_GetMainCameraSO", (void*)&ScriptScene::InternalGetMainCameraSo);

#if B3D_IS_ENGINE
	metaData.ScriptClass->AddInternalCall("Internal_SetActiveScene", (void*)&ScriptScene::InternalSetActiveScene);
	metaData.ScriptClass->AddInternalCall("Internal_ClearScene", (void*)&ScriptScene::InternalClearScene);

	MonoMethod* updateMethod = metaData.ScriptClass->GetMethod("OnUpdate");
	onUpdateThunk = (OnUpdateThunkDef)updateMethod->GetThunk();
#endif
}

void ScriptScene::StartUp()
{
	OnRefreshStartedConn = ScriptObjectManager::Instance().OnRefreshStarted.Connect(&OnRefreshStarted);
	OnRefreshDomainLoadedConn = ScriptObjectManager::Instance().OnRefreshDomainLoaded.Connect(&OnRefreshDomainLoaded);
}

void ScriptScene::ShutDown()
{
	OnRefreshStartedConn.Disconnect();
	OnRefreshDomainLoadedConn.Disconnect();
}

void ScriptScene::Update()
{
#if B3D_IS_ENGINE
	MonoUtil::InvokeThunk(onUpdateThunk);
#endif
}

void ScriptScene::SetActiveScene(const HPrefab& prefab)
{
	if(prefab.IsLoaded(false))
	{
		// If scene replace current root node, otherwise just append to the current root node
		if(prefab->IsScene())
			GetSceneManager().LoadScene(prefab);
		else
		{
			GetSceneManager().ClearScene();
			prefab->Instantiate();
		}
	}
	else
	{
		B3D_LOG(Error, Scene, "Attempting to activate a scene that hasn't finished loading yet.");
	}
}

void ScriptScene::OnRefreshStarted()
{
	MonoMethod* uuidMethod = metaData.ScriptClass->GetMethod("GetSceneUUID");
	if(uuidMethod != nullptr)
		sActiveSceneUUID = ScriptUUID::Unbox(uuidMethod->Invoke(nullptr, nullptr));

	MonoMethod* nameMethod = metaData.ScriptClass->GetMethod("GetSceneName");
	if(nameMethod != nullptr)
		sActiveSceneName = MonoUtil::MonoToString((MonoString*)nameMethod->Invoke(nullptr, nullptr));

	MonoMethod* genericPrefabMethod = metaData.ScriptClass->GetMethod("GetIsGenericPrefab");
	if(genericPrefabMethod != nullptr)
		sIsGenericPrefab = *(bool*)MonoUtil::Unbox(genericPrefabMethod->Invoke(nullptr, nullptr));
}

void ScriptScene::OnRefreshDomainLoaded()
{
	MonoMethod* uuidMethod = metaData.ScriptClass->GetMethod("SetSceneUUID", 1);
	if(uuidMethod != nullptr)
	{
		void* params[1];
		params[0] = ScriptUUID::Box(sActiveSceneUUID);

		uuidMethod->Invoke(nullptr, params);
	}

	MonoMethod* nameMethod = metaData.ScriptClass->GetMethod("SetSceneName", 1);
	if(nameMethod != nullptr)
	{
		void* params[1];
		params[0] = MonoUtil::StringToMono(sActiveSceneName);

		nameMethod->Invoke(nullptr, params);
	}

	MonoMethod* genericPrefabMethod = metaData.ScriptClass->GetMethod("SetIsGenericPrefab", 1);
	if(genericPrefabMethod != nullptr)
	{
		void* params[1] = { &sIsGenericPrefab };
		genericPrefabMethod->Invoke(nullptr, params);
	}
}

MonoObject* ScriptScene::InternalGetRoot()
{
	HSceneObject root = SceneManager::Instance().GetMainScene()->GetRoot();

	ScriptSceneObject* scriptRoot = ScriptGameObjectManager::Instance().GetOrCreateScriptSceneObject(root);
	return scriptRoot->GetManagedInstance();
}

MonoObject* ScriptScene::InternalGetMainCameraSo()
{
	SPtr<Camera> camera = GetSceneManager().GetMainCamera();
	HSceneObject so = GetSceneManager().GetActorSOInternal(camera);
	if(so == nullptr)
		return nullptr;

	ScriptSceneObject* cameraSo = ScriptGameObjectManager::Instance().GetOrCreateScriptSceneObject(so);
	return cameraSo->GetManagedInstance();
}

#if B3D_IS_ENGINE
void ScriptScene::InternalSetActiveScene(ScriptPrefab* scriptPrefab)
{
	HPrefab prefab = scriptPrefab->GetHandle();
	SetActiveScene(prefab);
}

void ScriptScene::InternalClearScene()
{
	GetSceneManager().ClearScene();
}
#endif
