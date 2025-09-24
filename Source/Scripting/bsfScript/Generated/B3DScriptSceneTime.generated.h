//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfUtility/Utility/BsTime.h"
#include "BsScriptNonReflectableWrapper.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSceneTime : public TScriptNonReflectableWrapper<SceneTime, ScriptSceneTime>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SceneTime")

		ScriptSceneTime(const SPtr<SceneTime>& nativeObject);
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
