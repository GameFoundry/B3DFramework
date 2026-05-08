//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Core/Scene/B3DSceneInstance.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "Utility/B3DUUID.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSceneInstance : public TScriptNonReflectableWrapper<SceneInstance, ScriptSceneInstance>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SceneInstance")

		ScriptSceneInstance(const TShared<SceneInstance>& nativeObject);
		~ScriptSceneInstance();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoString* InternalGetName(ScriptSceneInstance* self);
		static MonoObject* InternalGetRoot(ScriptSceneInstance* self);
		static bool InternalIsActive(ScriptSceneInstance* self);
		static MonoObject* InternalGetPhysicsScene(ScriptSceneInstance* self);
		static void InternalGetAssociatedResourceId(ScriptSceneInstance* self, UUID* __output);
		static MonoObject* InternalGetMainCamera(ScriptSceneInstance* self);
		static MonoObject* InternalGetEditorSceneInstance(ScriptSceneInstance* self);
		static void InternalClear(ScriptSceneInstance* self, bool forceAll);
		static MonoObject* InternalCreateSceneObject(ScriptSceneInstance* self, MonoString* name, uint32_t flags);
		static bool InternalIsRunning(ScriptSceneInstance* self);
		static void InternalCreate(MonoObject* scriptObject, MonoString* name);
		static void InternalCreate0(MonoObject* scriptObject, MonoString* name, MonoObject* root);
	};
}
