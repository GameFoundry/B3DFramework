//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptSceneGizmos.h"
#include "Scene/BsGizmoManager.h"
#include "BsScriptGizmoManager.h"
#include "Components/BsCCamera.h"

#include "BsScriptCCamera.generated.h"

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
		metaData.scriptClass->addInternalCall("Internal_Create", (void*)&ScriptSceneGizmos::internal_Create);
		metaData.scriptClass->addInternalCall("Internal_Draw", (void*)&ScriptSceneGizmos::internal_Draw);
	}

	void ScriptSceneGizmos::internal_Create(MonoObject* managedInstance, ScriptCCamera* camera)
	{
		new (bs_alloc<ScriptSceneGizmos>()) ScriptSceneGizmos(managedInstance, camera->getHandle()->_getCamera());
	}

	void ScriptSceneGizmos::internal_Draw(ScriptSceneGizmos* thisPtr)
	{
		ScriptGizmoManager::instance().update();
		GizmoManager::instance().update(thisPtr->mCamera);
	}
}
