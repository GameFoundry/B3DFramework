//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptPrefab.h"
#include "BsScriptResourceManager.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "Scene/BsSceneManager.h"
#include "Wrappers/BsScriptSceneObject.h"

using namespace bs;
ScriptPrefab::ScriptPrefab(MonoObject* instance, const HPrefab& prefab)
	: TScriptResource(instance, prefab)
{
}

void ScriptPrefab::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptPrefab::InternalCreateInstance);
	metaData.ScriptClass->AddInternalCall("Internal_Instantiate", (void*)&ScriptPrefab::InternalInstantiate);
	metaData.ScriptClass->AddInternalCall("Internal_IsScene", (void*)&ScriptPrefab::InternalIsScene);
}

void ScriptPrefab::InternalCreateInstance(MonoObject* instance, ScriptSceneObject* so, bool isScene)
{
	HPrefab prefab = Prefab::Create(so->GetHandle(), isScene);
	ScriptResourceManager::Instance().CreateBuiltinScriptResource(prefab, instance);
}

MonoObject* ScriptPrefab::InternalInstantiate(ScriptPrefab* thisPtr)
{
	HPrefab prefab = thisPtr->GetHandle();

	HSceneObject instance = prefab->Instantiate(GetSceneManager().GetMainScene());
	ScriptSceneObject* scriptInstance = ScriptGameObjectManager::Instance().GetOrCreateScriptSceneObject(instance);

	return scriptInstance->GetManagedInstance();
}

bool ScriptPrefab::InternalIsScene(ScriptPrefab* thisPtr)
{
	HPrefab prefab = thisPtr->GetHandle();
	return prefab->IsScene();
}

MonoObject* ScriptPrefab::CreateInstance()
{
	return metaData.ScriptClass->CreateInstance();
}
