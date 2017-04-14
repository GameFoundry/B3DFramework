//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPrefab.h"
#include "BsScriptResourceManager.h"
#include "BsScriptGameObjectManager.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsScriptSceneObject.h"

namespace bs
{
	ScriptPrefab::ScriptPrefab(MonoObject* instance, const HPrefab& prefab)
		:TScriptResource(instance, prefab)
	{

	}

	void ScriptPrefab::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptPrefab::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_Instantiate", &ScriptPrefab::internal_Instantiate);
		metaData.scriptClass->addInternalCall("Internal_IsScene", &ScriptPrefab::internal_IsScene);
	}

	void ScriptPrefab::internal_CreateInstance(MonoObject* instance, ScriptSceneObject* so, bool isScene)
	{
		HPrefab prefab = Prefab::create(so->getNativeSceneObject(), isScene);
		ScriptResourceBase* scriptInstance = ScriptResourceManager::instance().createBuiltinScriptResource(prefab, instance);
	}

	MonoObject* ScriptPrefab::internal_Instantiate(ScriptPrefab* thisPtr)
	{
		HPrefab prefab = thisPtr->getHandle();

		HSceneObject instance = prefab->instantiate();
		ScriptSceneObject* scriptInstance = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(instance);

		return scriptInstance->getManagedInstance();
	}

	bool ScriptPrefab::internal_IsScene(ScriptPrefab* thisPtr)
	{
		HPrefab prefab = thisPtr->getHandle();
		return prefab->isScene();
	}

	MonoObject* ScriptPrefab::createInstance()
	{
		return metaData.scriptClass->createInstance();
	}
}