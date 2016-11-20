//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptDragDropManager.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsScriptSceneObject.h"
#include "BsScriptGameObjectManager.h"
#include "BsGUISceneTreeView.h"
#include "BsGUIResourceTreeView.h"
#include "BsTime.h"

using namespace std::placeholders;

namespace bs
{
	ScriptDragDrop::ScriptDragDrop(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptDragDrop::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_IsDragInProgress", &ScriptDragDrop::internal_IsDragInProgress);
		metaData.scriptClass->addInternalCall("Internal_IsDropInProgress", &ScriptDragDrop::internal_IsDropInProgress);
		metaData.scriptClass->addInternalCall("Internal_GetData", &ScriptDragDrop::internal_GetData);
		metaData.scriptClass->addInternalCall("Internal_GetDragType", &ScriptDragDrop::internal_GetDragType);
		metaData.scriptClass->addInternalCall("Internal_StartSceneObjectDrag", &ScriptDragDrop::internal_StartSceneObjectDrag);
		metaData.scriptClass->addInternalCall("Internal_StartResourceDrag", &ScriptDragDrop::internal_StartResourceDrag);
	}

	bool ScriptDragDrop::internal_IsDragInProgress()
	{
		return DragAndDropManager::instance().isDragInProgress();
	}

	bool ScriptDragDrop::internal_IsDropInProgress()
	{
		return ScriptDragDropManager::instance().isDropInProgress();
	}

	MonoObject* ScriptDragDrop::internal_GetData()
	{
		return ScriptDragDropManager::instance().getDropData();
	}

	ScriptDragDropType ScriptDragDrop::internal_GetDragType()
	{
		return ScriptDragDropManager::instance().getDragType();
	}

	void ScriptDragDrop::internal_StartSceneObjectDrag(ScriptSceneObjectDragDropData* value)
	{
		const Vector<HSceneObject>& sceneObjects = value->getSceneObjects();
		DraggedSceneObjects* draggedSceneObjects = bs_new<DraggedSceneObjects>((UINT32)sceneObjects.size());

		UINT32 idx = 0;
		for (auto& so : sceneObjects)
		{
			draggedSceneObjects->objects[idx] = so;
			idx++;
		}

		DragAndDropManager::instance().startDrag((UINT32)DragAndDropType::SceneObject, draggedSceneObjects, 
			&ScriptDragDrop::sceneObjectDragDropFinalize, false);
	}

	void ScriptDragDrop::internal_StartResourceDrag(ScriptResourceDragDropData* value)
	{
		DraggedResources* draggedResources = bs_new<DraggedResources>();
		draggedResources->resourcePaths = value->getPaths();

		DragAndDropManager::instance().startDrag((UINT32)DragAndDropType::Resources, draggedResources, 
			&ScriptDragDrop::resourceDragDropFinalize, false);
	}

	void ScriptDragDrop::sceneObjectDragDropFinalize(bool processed)
	{
		DraggedSceneObjects* draggedSceneObjects = reinterpret_cast<DraggedSceneObjects*>(DragAndDropManager::instance().getDragData());
		bs_delete(draggedSceneObjects);
	}

	void ScriptDragDrop::resourceDragDropFinalize(bool processed)
	{
		DraggedResources* draggedResources = reinterpret_cast<DraggedResources*>(DragAndDropManager::instance().getDragData());
		bs_delete(draggedResources);
	}

	ScriptSceneObjectDragDropData::ScriptSceneObjectDragDropData(MonoObject* instance, const Vector<HSceneObject>& sceneObjects)
		:ScriptObject(instance), mSceneObjects(sceneObjects)
	{ }

	void ScriptSceneObjectDragDropData::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptSceneObjectDragDropData::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetObjects", &ScriptSceneObjectDragDropData::internal_GetObjects);
	}

	MonoObject* ScriptSceneObjectDragDropData::create(const Vector<HSceneObject>& sceneObjects)
	{
		ScriptArray arrSceneObjects = ScriptArray::create<ScriptSceneObject>((UINT32)sceneObjects.size());

		UINT32 idx = 0;
		for (auto& so : sceneObjects)
		{
			ScriptSceneObject* scriptSceneObject = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(so);

			arrSceneObjects.set(idx, scriptSceneObject->getManagedInstance());
			idx++;
		}

		void* params[1] = { arrSceneObjects.getInternal() };
		MonoObject* instance = metaData.scriptClass->createInstance(params, 1);

		return instance;
	}

	void ScriptSceneObjectDragDropData::internal_CreateInstance(MonoObject* managedInstance, MonoArray* objects)
	{
		ScriptArray objectsArray = ScriptArray(objects);

		Vector<HSceneObject> sceneObjects(objectsArray.size());
		for (UINT32 i = 0; i < objectsArray.size(); i++)
		{
			ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(objectsArray.get<MonoObject*>(i));
			if (scriptSO != nullptr)
				sceneObjects[i] = scriptSO->getNativeSceneObject();
		}

		new (bs_alloc<ScriptSceneObjectDragDropData>()) ScriptSceneObjectDragDropData(managedInstance, sceneObjects);
	}

	MonoArray* ScriptSceneObjectDragDropData::internal_GetObjects(ScriptSceneObjectDragDropData* instance)
	{
		ScriptArray objectsArray = ScriptArray::create<ScriptSceneObject>((UINT32)instance->mSceneObjects.size());
		for (UINT32 i = 0; i < (UINT32)instance->mSceneObjects.size(); i++)
		{
			ScriptSceneObject* scriptSO = ScriptGameObjectManager::instance().getOrCreateScriptSceneObject(instance->mSceneObjects[i]);

			if (scriptSO != nullptr)
				objectsArray.set(i, scriptSO->getManagedInstance());
		}

		return objectsArray.getInternal();
	}

	ScriptResourceDragDropData::ScriptResourceDragDropData(MonoObject* instance, const Vector<Path>& paths)
		:ScriptObject(instance), mPaths(paths)
	{ }

	void ScriptResourceDragDropData::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptResourceDragDropData::internal_CreateInstance);
		metaData.scriptClass->addInternalCall("Internal_GetPaths", &ScriptResourceDragDropData::internal_GetPaths);
	}

	MonoObject* ScriptResourceDragDropData::create(const Vector<Path>& paths)
	{
		ScriptArray arrStrings(MonoUtil::getStringClass(), (UINT32)paths.size());

		UINT32 idx = 0;
		for (auto& path : paths)
		{
			MonoString* managedPath = MonoUtil::wstringToMono(path.toWString());

			arrStrings.set(idx, managedPath);
			idx++;
		}

		void* params[1] = { arrStrings.getInternal() };
		MonoObject* instance = metaData.scriptClass->createInstance(params, 1);

		return instance;
	}

	void ScriptResourceDragDropData::internal_CreateInstance(MonoObject* managedInstance, MonoArray* monoPaths)
	{
		ScriptArray pathsArray = ScriptArray(monoPaths);

		Vector<Path> paths(pathsArray.size());
		for (UINT32 i = 0; i < pathsArray.size(); i++)
		{
			MonoString* monoPath = pathsArray.get<MonoString*>(i);
			paths[i] = MonoUtil::monoToWString(monoPath);
		}

		new (bs_alloc<ScriptResourceDragDropData>()) ScriptResourceDragDropData(managedInstance, paths);
	}

	MonoArray* ScriptResourceDragDropData::internal_GetPaths(ScriptResourceDragDropData* instance)
	{
		ScriptArray arrStrings(MonoUtil::getStringClass(), (UINT32)instance->mPaths.size());

		UINT32 idx = 0;
		for (auto& path : instance->mPaths)
		{
			MonoString* managedPath = MonoUtil::wstringToMono(path.toWString());

			arrStrings.set(idx, managedPath);
			idx++;
		}

		return arrStrings.getInternal();
	}

	ScriptDragDropManager::ScriptDragDropManager()
		:mIsDropInProgress(false), mDroppedFrameIdx(0), mDropType(ScriptDragDropType::None)
	{
		mDragEndedConn = DragAndDropManager::instance().onDragEnded.connect(std::bind(&ScriptDragDropManager::onMouseDragEnded, this, _1, _2));
	}

	ScriptDragDropManager::~ScriptDragDropManager()
	{
		mDragEndedConn.disconnect();
	}

	void ScriptDragDropManager::update()
	{
		// This only stays active for a single frame
		if (mIsDropInProgress && mDroppedFrameIdx < Time::instance().getFrameIdx())
			mIsDropInProgress = false;
	}

	MonoObject* ScriptDragDropManager::getDropData() const
	{
		if (DragAndDropManager::instance().isDragInProgress())
		{
			UINT32 nativeType = DragAndDropManager::instance().getDragTypeId();

			if (nativeType == (UINT32)DragAndDropType::SceneObject)
			{
				DraggedSceneObjects* draggedData = reinterpret_cast<DraggedSceneObjects*>(DragAndDropManager::instance().getDragData());

				Vector<HSceneObject> draggedSceneObjects;
				for (UINT32 i = 0; i < draggedData->numObjects; i++)
					draggedSceneObjects.push_back(draggedData->objects[i]);

				return ScriptSceneObjectDragDropData::create(draggedSceneObjects);
			}
			else if (nativeType == (UINT32)DragAndDropType::Resources)
			{
				DraggedResources* draggedResources = reinterpret_cast<DraggedResources*>(DragAndDropManager::instance().getDragData());

				return ScriptResourceDragDropData::create(draggedResources->resourcePaths);
			}
		}
		else
		{
			if (mDropType == ScriptDragDropType::SceneObject)
				return ScriptSceneObjectDragDropData::create(mDroppedSceneObjects);
			else if (mDropType == ScriptDragDropType::Resource)
				return ScriptResourceDragDropData::create(mDroppedPaths);
		}

		return nullptr;
	}

	ScriptDragDropType ScriptDragDropManager::getDragType() const
	{
		if (DragAndDropManager::instance().isDragInProgress())
		{
			UINT32 nativeType = DragAndDropManager::instance().getDragTypeId();

			if (nativeType == (UINT32)DragAndDropType::SceneObject)
				return ScriptDragDropType::SceneObject;
			else if (nativeType == (UINT32)DragAndDropType::Resources)
				return ScriptDragDropType::Resource;

			return ScriptDragDropType::None;
		}
		else
			return mDropType;
	}

	void ScriptDragDropManager::onMouseDragEnded(const PointerEvent& evt, DragCallbackInfo& callbackInfo)
	{
		if (!DragAndDropManager::instance().isDragInProgress())
			return;

		mDroppedPaths.clear();
		mDroppedSceneObjects.clear();
		mIsDropInProgress = false;
		mDropType = ScriptDragDropType::None;
		
		UINT32 nativeType = DragAndDropManager::instance().getDragTypeId();

		if (nativeType == (UINT32)DragAndDropType::SceneObject)
		{
			DraggedSceneObjects* draggedSceneObjects = reinterpret_cast<DraggedSceneObjects*>(DragAndDropManager::instance().getDragData());

			for (UINT32 i = 0; i < draggedSceneObjects->numObjects; i++)
				mDroppedSceneObjects.push_back(draggedSceneObjects->objects[i]);

			mIsDropInProgress = true;
			mDropType = ScriptDragDropType::SceneObject;
			mDroppedFrameIdx = Time::instance().getFrameIdx();
		}
		else if (nativeType == (UINT32)DragAndDropType::Resources)
		{
			DraggedResources* draggedResources = reinterpret_cast<DraggedResources*>(DragAndDropManager::instance().getDragData());

			mDroppedPaths = draggedResources->resourcePaths;
			mIsDropInProgress = true;
			mDropType = ScriptDragDropType::Resource;
			mDroppedFrameIdx = Time::instance().getFrameIdx();
		}
	}
}
