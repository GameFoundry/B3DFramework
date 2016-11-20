//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSceneGizmos.h"
#include "BsScriptCamera.h"
#include "BsGizmoManager.h"

namespace bs
{
	ScriptSceneGizmos::ScriptSceneGizmos(MonoObject* object, const SPtr<Camera>& camera)
		:ScriptObject(object), mCamera(camera)
	{

	}

	ScriptSceneGizmos::~ScriptSceneGizmos()
	{
		if (GizmoManager::isStarted()) // If not active, we don't care
			GizmoManager::instance().clearRenderData();
	}

	void ScriptSceneGizmos::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Create", &ScriptSceneGizmos::internal_Create);
		metaData.scriptClass->addInternalCall("Internal_Draw", &ScriptSceneGizmos::internal_Draw);
	}

	void ScriptSceneGizmos::internal_Create(MonoObject* managedInstance, ScriptCamera* camera)
	{
		new (bs_alloc<ScriptSceneGizmos>()) ScriptSceneGizmos(managedInstance, camera->getInternal());
	}

	void ScriptSceneGizmos::internal_Draw(ScriptSceneGizmos* thisPtr)
	{
		GizmoManager::instance().update(thisPtr->mCamera);
	}
}