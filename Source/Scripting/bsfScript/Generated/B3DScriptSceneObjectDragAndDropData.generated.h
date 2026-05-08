//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptDragAndDropData.generated.h"
#include "../../../Engine/Core/GUI/B3DDragAndDrop.h"

namespace b3d { class SceneObjectDragAndDropData; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSceneObjectDragAndDropData : public TScriptReflectableWrapper<SceneObjectDragAndDropData, ScriptSceneObjectDragAndDropData, ScriptDragAndDropDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SceneObjectDragAndDropData")

		ScriptSceneObjectDragAndDropData(const TShared<SceneObjectDragAndDropData>& nativeObject);
		~ScriptSceneObjectDragAndDropData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSceneObjectDragAndDropData(MonoObject* scriptObject);
		static void InternalSceneObjectDragAndDropData0(MonoObject* scriptObject, MonoObject* sceneObject);
		static void InternalSceneObjectDragAndDropData1(MonoObject* scriptObject, MonoArray* sceneObjects);
		static MonoArray* InternalGetSceneObjects(ScriptSceneObjectDragAndDropData* self);
		static void InternalSetSceneObjects(ScriptSceneObjectDragAndDropData* self, MonoArray* value);
	};
}
