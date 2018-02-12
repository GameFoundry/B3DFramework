//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptSelection.h"
#include "BsScriptMeta.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "Scene/BsSelection.h"
#include "Wrappers/BsScriptSceneObject.h"
#include "BsMonoUtil.h"
#include "BsScriptGameObjectManager.h"
#include "Wrappers/BsScriptResource.h"

namespace bs
{
	ScriptSelection::OnSelectionChangedThunkDef ScriptSelection::OnSelectionChangedThunk;
	ScriptSelection::OnPingResourceThunkDef ScriptSelection::OnPingResourceThunk;
	ScriptSelection::OnPingSceneObjectThunkDef ScriptSelection::OnPingSceneObjectThunk;

	HEvent ScriptSelection::OnSelectionChangedConn;
	HEvent ScriptSelection::OnPingResourceConn;
	HEvent ScriptSelection::OnPingSceneObjectConn;

	ScriptSelection::ScriptSelection(MonoObject* instance)
		:ScriptObject(instance)
	{
		
	}

	void ScriptSelection::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetSceneObjectSelection", (void*)&ScriptSelection::internal_GetSceneObjectSelection);
		metaData.scriptClass->addInternalCall("Internal_SetSceneObjectSelection", (void*)&ScriptSelection::internal_SetSceneObjectSelection);
		metaData.scriptClass->addInternalCall("Internal_GetResourceUUIDSelection", (void*)&ScriptSelection::internal_GetResourceUUIDSelection);
		metaData.scriptClass->addInternalCall("Internal_SetResourceUUIDSelection", (void*)&ScriptSelection::internal_SetResourceUUIDSelection);
		metaData.scriptClass->addInternalCall("Internal_GetResourcePathSelection", (void*)&ScriptSelection::internal_GetResourcePathSelection);
		metaData.scriptClass->addInternalCall("Internal_SetResourcePathSelection", (void*)&ScriptSelection::internal_SetResourcePathSelection);
		metaData.scriptClass->addInternalCall("Internal_PingResource", (void*)&ScriptSelection::internal_PingResource);
		metaData.scriptClass->addInternalCall("Internal_PingSceneObject", (void*)&ScriptSelection::internal_PingSceneObject);

		OnSelectionChangedThunk = (OnSelectionChangedThunkDef)metaData.scriptClass->getMethod("Internal_TriggerSelectionChanged", 2)->getThunk();
		OnPingResourceThunk = (OnPingResourceThunkDef)metaData.scriptClass->getMethod("Internal_TriggerResourcePing", 1)->getThunk();
		OnPingSceneObjectThunk = (OnPingSceneObjectThunkDef)metaData.scriptClass->getMethod("Internal_TriggerSceneObjectPing", 1)->getThunk();
	}

	void ScriptSelection::internal_GetSceneObjectSelection(MonoArray** selection)
	{
		Vector<HSceneObject> sceneObjects = Selection::instance().getSceneObjects();

		::MonoClass* sceneObjectMonoClass = ScriptSceneObject::getMetaData()->scriptClass->_getInternalClass();
		ScriptArray sceneObjectArray(sceneObjectMonoClass, (UINT32)sceneObjects.size());

		for (UINT32 i = 0; i < (UINT32)sceneObjects.size(); i++)
		{
			ScriptSceneObject* scriptSceneObject = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(sceneObjects[i]);
			MonoObject* sceneMonoObject = scriptSceneObject->getManagedInstance();

			sceneObjectArray.set(i, sceneMonoObject);
		}

		MonoUtil::referenceCopy(selection, (MonoObject*)sceneObjectArray.getInternal());
	}

	void ScriptSelection::internal_SetSceneObjectSelection(MonoArray* selection)
	{
		Vector<HSceneObject> sceneObjects;

		if (selection != nullptr)
		{
			ScriptArray scriptArray(selection);

			UINT32 arrayLen = scriptArray.size();
			for (UINT32 i = 0; i < arrayLen; i++)
			{
				MonoObject* monoSO = scriptArray.get<MonoObject*>(i);
				ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(monoSO);

				if (scriptSO == nullptr)
					continue;

				HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());
				sceneObjects.push_back(so);
			}
		}

		Selection::instance().setSceneObjects(sceneObjects);
	}

	void ScriptSelection::internal_GetResourceUUIDSelection(MonoArray** selection)
	{
		Vector<UUID> uuids = Selection::instance().getResourceUUIDs();
		ScriptArray uuidArray(MonoUtil::getStringClass(), (UINT32)uuids.size());

		for (UINT32 i = 0; i < (UINT32)uuids.size(); i++)
			uuidArray.set(i, ScriptUUID::box(uuids[i]));

		MonoUtil::referenceCopy(selection, (MonoObject*)uuidArray.getInternal());
	}

	void ScriptSelection::internal_SetResourceUUIDSelection(MonoArray* selection)
	{
		Vector<UUID> uuids;

		ScriptArray uuidArray(selection);
		UINT32 arrayLen = uuidArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
			uuids.push_back(ScriptUUID::unbox(uuidArray.get<MonoObject*>(i)));

		Selection::instance().setResourceUUIDs(uuids);
	}

	void ScriptSelection::internal_GetResourcePathSelection(MonoArray** selection)
	{
		Vector<Path> paths = Selection::instance().getResourcePaths();
		ScriptArray pathArray(MonoUtil::getStringClass(), (UINT32)paths.size());

		for (UINT32 i = 0; i < (UINT32)paths.size(); i++)
		{
			MonoString* monoString = MonoUtil::wstringToMono(paths[i].toWString());
			pathArray.set(i, monoString);
		}

		MonoUtil::referenceCopy(selection, (MonoObject*)pathArray.getInternal());
	}

	void ScriptSelection::internal_SetResourcePathSelection(MonoArray* selection)
	{
		Vector<Path> paths;

		ScriptArray pathArray(selection);
		UINT32 arrayLen = pathArray.size();
		for (UINT32 i = 0; i < arrayLen; i++)
		{
			MonoString* monoString = pathArray.get<MonoString*>(i);
			Path path  = MonoUtil::monoToString(monoString);

			paths.push_back(path);
		}

		Selection::instance().setResourcePaths(paths);
	}

	void ScriptSelection::internal_PingResource(MonoString* resourcePath)
	{
		Path path = MonoUtil::monoToString(resourcePath);

		Selection::instance().ping(path);
	}

	void ScriptSelection::internal_PingSceneObject(MonoObject* so)
	{
		ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(so);
		HSceneObject soHandle = static_object_cast<SceneObject>(scriptSO->getNativeHandle());

		Selection::instance().ping(soHandle);
	}

	void ScriptSelection::startUp()
	{
		OnSelectionChangedConn = Selection::instance().onSelectionChanged.connect(&ScriptSelection::onSelectionChanged);
		OnPingResourceConn = Selection::instance().onResourcePing.connect(&ScriptSelection::onResourcePing);
		OnPingSceneObjectConn = Selection::instance().onSceneObjectPing.connect(&ScriptSelection::onSceneObjectPing);
	}

	void ScriptSelection::shutDown()
	{
		OnSelectionChangedConn.disconnect();
		OnPingResourceConn.disconnect();
		OnPingSceneObjectConn.disconnect();
	}

	void ScriptSelection::onSelectionChanged(const Vector<HSceneObject>& sceneObjects, const Vector<Path>& resPaths)
	{
		UINT32 numObjects = (UINT32)sceneObjects.size();
		ScriptArray scriptObjects = ScriptArray::create<ScriptSceneObject>(numObjects);
		for (UINT32 i = 0; i < numObjects; i++)
		{
			ScriptSceneObject* scriptSceneObject = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(sceneObjects[i]);
			scriptObjects.set(i, scriptSceneObject->getManagedInstance());
		}

		UINT32 numPaths = (UINT32)resPaths.size();
		ScriptArray scriptPaths = ScriptArray::create<String>(numPaths);
		for (UINT32 i = 0; i < numPaths; i++)
			scriptPaths.set(i, resPaths[i].toString());

		MonoArray* monoObjects = scriptObjects.getInternal();
		MonoArray* monoPaths = scriptPaths.getInternal();
		MonoUtil::invokeThunk(OnSelectionChangedThunk, monoObjects, monoPaths);
	}

	void ScriptSelection::onResourcePing(const Path& resPath)
	{
		MonoString* monoResPath = MonoUtil::wstringToMono(resPath.toWString());

		MonoUtil::invokeThunk(OnPingResourceThunk, monoResPath);
	}

	void ScriptSelection::onSceneObjectPing(const HSceneObject& sceneObject)
	{
		ScriptSceneObject* scriptSceneObject = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(sceneObject);
		MonoUtil::invokeThunk(OnPingSceneObjectThunk, scriptSceneObject->getManagedInstance());
	}
}
