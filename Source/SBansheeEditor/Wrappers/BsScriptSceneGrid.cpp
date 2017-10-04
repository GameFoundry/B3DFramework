//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Wrappers/BsScriptSceneGrid.h"
#include "BsEditorApplication.h"
#include "Scene/BsSceneGrid.h"
#include "Components/BsCCamera.h"

#include "BsScriptCCamera.generated.h"

namespace bs
{
	ScriptSceneGrid::ScriptSceneGrid(MonoObject* object, const SPtr<Camera>& camera)
		:ScriptObject(object), mSceneGrid(nullptr)
	{
		mSceneGrid = bs_new<SceneGrid>(camera);
		mSceneGrid->setSettings(gEditorApplication().getEditorSettings());
	}

	ScriptSceneGrid::~ScriptSceneGrid()
	{
		bs_delete(mSceneGrid);
	}

	void ScriptSceneGrid::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Create", (void*)&ScriptSceneGrid::internal_Create);
		metaData.scriptClass->addInternalCall("Internal_Draw", (void*)&ScriptSceneGrid::internal_Draw);
		metaData.scriptClass->addInternalCall("Internal_SetMode", (void*)&ScriptSceneGrid::internal_SetMode);
	}

	void ScriptSceneGrid::internal_Create(MonoObject* managedInstance, ScriptCCamera* camera)
	{
		new (bs_alloc<ScriptSceneGrid>()) ScriptSceneGrid(managedInstance, camera->getHandle()->_getCamera());
	}

	void ScriptSceneGrid::internal_Draw(ScriptSceneGrid* thisPtr)
	{
		thisPtr->mSceneGrid->_update();
	}

	void ScriptSceneGrid::internal_SetMode(ScriptSceneGrid* thisPtr, UINT32 mode)
	{
		thisPtr->mSceneGrid->setMode((GridMode)mode);
	}
}