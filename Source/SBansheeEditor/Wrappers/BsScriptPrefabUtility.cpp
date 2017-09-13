//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptPrefabUtility.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "Scene/BsPrefabUtility.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "Scene/BsSceneObject.h"
#include "Scene/BsPrefab.h"
#include "Resources/BsResources.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
#include "Wrappers/BsScriptPrefab.h"

namespace bs
{
	ScriptPrefabUtility::ScriptPrefabUtility(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptPrefabUtility::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_BreakPrefab", (void*)&ScriptPrefabUtility::internal_breakPrefab);
		metaData.scriptClass->addInternalCall("Internal_ApplyPrefab", (void*)&ScriptPrefabUtility::internal_applyPrefab);
		metaData.scriptClass->addInternalCall("Internal_RevertPrefab", (void*)&ScriptPrefabUtility::internal_revertPrefab);
		metaData.scriptClass->addInternalCall("Internal_HasPrefabLink", (void*)&ScriptPrefabUtility::internal_hasPrefabLink);
		metaData.scriptClass->addInternalCall("Internal_GetPrefabParent", (void*)&ScriptPrefabUtility::internal_getPrefabParent);
		metaData.scriptClass->addInternalCall("Internal_GetPrefabUUID", (void*)&ScriptPrefabUtility::internal_GetPrefabUUID);
		metaData.scriptClass->addInternalCall("Internal_UpdateFromPrefab", (void*)&ScriptPrefabUtility::internal_UpdateFromPrefab);
		metaData.scriptClass->addInternalCall("Internal_RecordPrefabDiff", (void*)&ScriptPrefabUtility::internal_RecordPrefabDiff);
	}

	void ScriptPrefabUtility::internal_breakPrefab(ScriptSceneObject* soPtr)
	{
		if (ScriptSceneObject::checkIfDestroyed(soPtr))
			return;

		soPtr->getNativeSceneObject()->breakPrefabLink();
	}

	void ScriptPrefabUtility::internal_applyPrefab(ScriptSceneObject* soPtr, ScriptPrefab* prefabPtr)
	{
		if (ScriptSceneObject::checkIfDestroyed(soPtr))
			return;

		prefabPtr->getHandle()->update(soPtr->getNativeSceneObject());
	}

	void ScriptPrefabUtility::internal_revertPrefab(ScriptSceneObject* soPtr)
	{
		if (ScriptSceneObject::checkIfDestroyed(soPtr))
			return;

		PrefabUtility::revertToPrefab(soPtr->getNativeSceneObject());
	}

	bool ScriptPrefabUtility::internal_hasPrefabLink(ScriptSceneObject* soPtr)
	{
		if (ScriptSceneObject::checkIfDestroyed(soPtr))
			return false;

		return !soPtr->getNativeSceneObject()->getPrefabLink(true).empty();
	}

	MonoObject* ScriptPrefabUtility::internal_getPrefabParent(ScriptSceneObject* soPtr)
	{
		if (ScriptSceneObject::checkIfDestroyed(soPtr))
			return nullptr;

		HSceneObject so = soPtr->getNativeSceneObject();
		HSceneObject parent = so->getPrefabParent();

		if (parent != nullptr)
		{
			ScriptSceneObject* scriptParent = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(parent);
			return scriptParent->getManagedInstance();
		}

		return nullptr;
	}

	void ScriptPrefabUtility::internal_GetPrefabUUID(ScriptSceneObject* soPtr, UUID* uuid)
	{
		*uuid = UUID::EMPTY;

		if (ScriptSceneObject::checkIfDestroyed(soPtr))
			return;

		HSceneObject so = soPtr->getNativeSceneObject();
		*uuid = soPtr->getNativeSceneObject()->getPrefabLink(true);
	}

	void ScriptPrefabUtility::internal_UpdateFromPrefab(ScriptSceneObject* soPtr)
	{
		if (ScriptSceneObject::checkIfDestroyed(soPtr))
			return;

		HSceneObject so = soPtr->getNativeSceneObject();
		HSceneObject prefabParent = so->getPrefabParent();

		if(prefabParent != nullptr)
			PrefabUtility::updateFromPrefab(prefabParent);
	}

	void ScriptPrefabUtility::internal_RecordPrefabDiff(ScriptSceneObject* soPtr)
	{
		if (ScriptSceneObject::checkIfDestroyed(soPtr))
			return;

		HSceneObject so = soPtr->getNativeSceneObject();
		PrefabUtility::recordPrefabDiff(so);
	}
}