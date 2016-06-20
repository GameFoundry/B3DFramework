//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPrefabUtility.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsPrefabUtility.h"
#include "BsScriptSceneObject.h"
#include "BsSceneObject.h"
#include "BsPrefab.h"
#include "BsResources.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"

namespace BansheeEngine
{
	ScriptPrefabUtility::ScriptPrefabUtility(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptPrefabUtility::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_BreakPrefab", &ScriptPrefabUtility::internal_breakPrefab);
		metaData.scriptClass->addInternalCall("Internal_ApplyPrefab", &ScriptPrefabUtility::internal_applyPrefab);
		metaData.scriptClass->addInternalCall("Internal_RevertPrefab", &ScriptPrefabUtility::internal_revertPrefab);
		metaData.scriptClass->addInternalCall("Internal_HasPrefabLink", &ScriptPrefabUtility::internal_hasPrefabLink);
		metaData.scriptClass->addInternalCall("Internal_GetPrefabParent", &ScriptPrefabUtility::internal_getPrefabParent);
		metaData.scriptClass->addInternalCall("Internal_GetPrefabUUID", &ScriptPrefabUtility::internal_GetPrefabUUID);
	}

	void ScriptPrefabUtility::internal_breakPrefab(ScriptSceneObject* nativeInstance)
	{
		if (ScriptSceneObject::checkIfDestroyed(nativeInstance))
			return;

		nativeInstance->getNativeSceneObject()->breakPrefabLink();
	}

	void ScriptPrefabUtility::internal_applyPrefab(ScriptSceneObject* nativeInstance)
	{
		if (ScriptSceneObject::checkIfDestroyed(nativeInstance))
			return;

		ResourceLoadFlags loadFlags = ResourceLoadFlag::KeepInternalRef | ResourceLoadFlag::KeepSourceData;

		String prefabLinkUUID = nativeInstance->getNativeSceneObject()->getPrefabLink();
		HPrefab prefab = static_resource_cast<Prefab>(gResources().loadFromUUID(prefabLinkUUID, false, loadFlags));

		if (prefab != nullptr)
			prefab->update(nativeInstance->getNativeSceneObject());

		gResources().save(prefab);
	}

	void ScriptPrefabUtility::internal_revertPrefab(ScriptSceneObject* nativeInstance)
	{
		if (ScriptSceneObject::checkIfDestroyed(nativeInstance))
			return;

		PrefabUtility::revertToPrefab(nativeInstance->getNativeSceneObject());
	}

	bool ScriptPrefabUtility::internal_hasPrefabLink(ScriptSceneObject* nativeInstance)
	{
		if (ScriptSceneObject::checkIfDestroyed(nativeInstance))
			return false;

		return !nativeInstance->getNativeSceneObject()->getPrefabLink().empty();
	}

	MonoObject* ScriptPrefabUtility::internal_getPrefabParent(ScriptSceneObject* nativeInstance)
	{
		if (ScriptSceneObject::checkIfDestroyed(nativeInstance))
			return nullptr;

		HSceneObject so = nativeInstance->getNativeSceneObject();
		HSceneObject parent = so->getPrefabParent();

		if(parent != nullptr)
		{
			ScriptSceneObject* scriptParent = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(parent);
			return scriptParent->getManagedInstance();
		}

		return nullptr;
	}

	MonoString* ScriptPrefabUtility::internal_GetPrefabUUID(ScriptSceneObject* nativeInstance)
	{
		if (ScriptSceneObject::checkIfDestroyed(nativeInstance))
			return nullptr;

		HSceneObject so = nativeInstance->getNativeSceneObject();

		String prefabUUID = nativeInstance->getNativeSceneObject()->getPrefabLink();
		if (prefabUUID.empty())
			return nullptr;

		return MonoUtil::stringToMono(prefabUUID);
	}
}