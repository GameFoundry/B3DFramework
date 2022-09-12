//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptPrefab.h"
#include "BsScriptResourceManager.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "Wrappers/BsScriptSceneObject.h"

namespace bs
{
	ScriptPrefab::ScriptPrefab(MonoObject* instance, const HPrefab& prefab)
		:TScriptResource(instance, prefab)
	{

	}

	void ScriptPrefab::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptPrefab::internal_CreateInstance);
		metaData.scriptClass->AddInternalCall("Internal_Instantiate", (void*)&ScriptPrefab::internal_Instantiate);
		metaData.scriptClass->AddInternalCall("Internal_IsScene", (void*)&ScriptPrefab::internal_IsScene);
	}

	void ScriptPrefab::internal_CreateInstance(MonoObject* instance, ScriptSceneObject* so, bool isScene)
	{
		HPrefab prefab = Prefab::create(so->GetHandle(), isScene);
		ScriptResourceManager::instance().CreateBuiltinScriptResource(prefab, instance);
	}

	MonoObject* ScriptPrefab::internal_Instantiate(ScriptPrefab* thisPtr)
	{
		HPrefab prefab = thisPtr->GetHandle();

		HSceneObject instance = prefab->Instantiate();
		ScriptSceneObject* scriptInstance = ScriptGameObjectManager::instance().GetOrCreateScriptSceneObject(instance);

		return scriptInstance->GetManagedInstance();
	}

	bool ScriptPrefab::internal_IsScene(ScriptPrefab* thisPtr)
	{
		HPrefab prefab = thisPtr->GetHandle();
		return prefab->IsScene();
	}

	MonoObject* ScriptPrefab::createInstance()
	{
		return metaData.scriptClass->CreateInstance();
	}
}
