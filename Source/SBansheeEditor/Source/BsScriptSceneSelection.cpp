//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSceneSelection.h"
#include "BsScriptCamera.h"
#include "BsSelectionRenderer.h"
#include "BsScriptSceneObject.h"
#include "BsScenePicking.h"
#include "BsSelection.h"
#include <BsMonoArray.h>

namespace BansheeEngine
{
	ScriptSceneSelection::ScriptSceneSelection(MonoObject* object, const SPtr<Camera>& camera)
		:ScriptObject(object), mCamera(camera), mSelectionRenderer(nullptr)
	{
		mSelectionRenderer = bs_new<SelectionRenderer>();
	}

	ScriptSceneSelection::~ScriptSceneSelection()
	{
		bs_delete(mSelectionRenderer);
	}

	void ScriptSceneSelection::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_Create", &ScriptSceneSelection::internal_Create);
		metaData.scriptClass->addInternalCall("Internal_Draw", &ScriptSceneSelection::internal_Draw);
		metaData.scriptClass->addInternalCall("Internal_PickObject", &ScriptSceneSelection::internal_PickObject);
		metaData.scriptClass->addInternalCall("Internal_Snap", &ScriptSceneSelection::internal_Snap);
	}

	void ScriptSceneSelection::internal_Create(MonoObject* managedInstance, ScriptCamera* camera)
	{
		new (bs_alloc<ScriptSceneSelection>()) ScriptSceneSelection(managedInstance, camera->getInternal());
	}

	void ScriptSceneSelection::internal_Draw(ScriptSceneSelection* thisPtr)
	{
		thisPtr->mSelectionRenderer->update(thisPtr->mCamera);
	}

	void ScriptSceneSelection::internal_PickObject(ScriptSceneSelection* thisPtr, Vector2I* inputPos, bool additive, MonoArray* ignoreRenderables)
	{
		// TODO - Handle multi-selection (i.e. selection rectangle when dragging)
		SnapData data;

		Vector<HSceneObject> ignoredSceneObjects;

		if (ignoreRenderables != nullptr)
		{
			ScriptArray scriptArray(ignoreRenderables);

			UINT32 arrayLen = scriptArray.size();
			for (UINT32 i = 0; i < arrayLen; i++)
			{
				MonoObject* monoSO = scriptArray.get<MonoObject*>(i);
				ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(monoSO);

				if (scriptSO == nullptr)
					continue;

				HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());
				ignoredSceneObjects.push_back(so);
			}
		}

		HSceneObject pickedObject = ScenePicking::instance().pickClosestObject(thisPtr->mCamera, *inputPos, Vector2I(1, 1), data, ignoredSceneObjects);

		if (pickedObject)
		{
			if (additive) // Append to existing selection
			{
				Vector<HSceneObject> selectedSOs = Selection::instance().getSceneObjects();

				auto iterFind = std::find_if(selectedSOs.begin(), selectedSOs.end(),
					[&](const HSceneObject& obj) { return obj == pickedObject; }
				);

				if (iterFind == selectedSOs.end())
					selectedSOs.push_back(pickedObject);

				Selection::instance().setSceneObjects(selectedSOs);
			}
			else
			{
				Vector<HSceneObject> selectedSOs = { pickedObject };

				Selection::instance().setSceneObjects(selectedSOs);
			}
		}
		else
			Selection::instance().clearSceneSelection();
	}

	void ScriptSceneSelection::internal_Snap(ScriptSceneSelection* thisPtr, Vector2I* inputPos, SnapData* data, MonoArray* ignoreRenderables)
	{
		Vector<HSceneObject> ignoredSceneObjects;

		if (ignoreRenderables != nullptr)
		{
			ScriptArray scriptArray(ignoreRenderables);

			UINT32 arrayLen = scriptArray.size();
			for (UINT32 i = 0; i < arrayLen; i++)
			{
				MonoObject* monoSO = scriptArray.get<MonoObject*>(i);
				ScriptSceneObject* scriptSO = ScriptSceneObject::toNative(monoSO);

				if (scriptSO == nullptr)
					continue;

				HSceneObject so = static_object_cast<SceneObject>(scriptSO->getNativeHandle());
				ignoredSceneObjects.push_back(so);
			}
		}

  		ScenePicking::instance().pickClosestObject(thisPtr->mCamera, *inputPos, Vector2I(1, 1), *data, ignoredSceneObjects);
	}

}
