//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptDragAndDropData.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsDragAndDrop.h"

namespace bs { class SceneObjectDragAndDropData; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSceneObjectDragAndDropData : public TScriptReflectableWrapper<SceneObjectDragAndDropData, ScriptSceneObjectDragAndDropData, ScriptDragAndDropDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SceneObjectDragAndDropData")

		ScriptSceneObjectDragAndDropData(const SPtr<SceneObjectDragAndDropData>& nativeObject);
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
