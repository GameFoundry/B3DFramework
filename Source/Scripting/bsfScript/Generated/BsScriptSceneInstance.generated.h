//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs { class SceneInstance; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSceneInstance : public ScriptObject<ScriptSceneInstance>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SceneInstance")

		ScriptSceneInstance(MonoObject* managedInstance, const SPtr<SceneInstance>& value);

		SPtr<SceneInstance> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<SceneInstance>& value);

	private:
		SPtr<SceneInstance> mInternal;

		static MonoString* InternalGetName(ScriptSceneInstance* self);
		static MonoObject* InternalGetRoot(ScriptSceneInstance* self);
		static bool InternalIsActive(ScriptSceneInstance* self);
		static MonoObject* InternalGetPhysicsScene(ScriptSceneInstance* self);
		static MonoObject* InternalCreateSceneObject(ScriptSceneInstance* self, MonoString* name);
		static void InternalCreate(MonoObject* managedInstance, MonoString* name);
		static void InternalCreate0(MonoObject* managedInstance, MonoString* name, MonoObject* root);
	};
}
