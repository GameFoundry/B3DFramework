//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Utility/Utility/B3DTime.h"
#include "B3DScriptNonReflectableWrapper.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSceneTime : public TScriptNonReflectableWrapper<SceneTime, ScriptSceneTime>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SceneTime")

		ScriptSceneTime(const TShared<SceneTime>& nativeObject);
		~ScriptSceneTime();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static float InternalGetTimeInSeconds(ScriptSceneTime* self);
		static void InternalSetScale(ScriptSceneTime* self, float scale);
		static float InternalGetScale(ScriptSceneTime* self);
		static void InternalReset(ScriptSceneTime* self);
		static void InternalSetPaused(ScriptSceneTime* self, bool paused);
	};
}
