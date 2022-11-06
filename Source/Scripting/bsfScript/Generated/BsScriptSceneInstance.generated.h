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

		static MonoString* InternalGetName(ScriptSceneInstance* thisPtr);
		static MonoObject* InternalGetRoot(ScriptSceneInstance* thisPtr);
		static bool InternalIsActive(ScriptSceneInstance* thisPtr);
		static MonoObject* InternalGetPhysicsScene(ScriptSceneInstance* thisPtr);
	};
}
