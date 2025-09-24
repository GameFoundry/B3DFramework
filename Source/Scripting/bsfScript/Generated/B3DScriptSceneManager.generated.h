//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Core/Scene/B3DSceneManager.h"
#include "B3DScriptTypeDefinition.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSceneManager : public TScriptTypeDefinition<ScriptSceneManager>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "SceneManager")

		ScriptSceneManager();

		static void SetupScriptBindings();

	private:
		static void InternalSetMainScene(MonoObject* scene);
		static MonoObject* InternalGetMainScene();
	};
}
