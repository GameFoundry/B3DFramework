//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"

namespace b3d { class Skybox; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSkybox : public TScriptGameObjectWrapper<Skybox, ScriptSkybox>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Skybox")

		ScriptSkybox(const GameObjectHandle<Skybox>& nativeObject);
		~ScriptSkybox();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetTexture(ScriptSkybox* self, MonoObject* texture);
		static void InternalSetBrightness(ScriptSkybox* self, float brightness);
		static float InternalGetBrightness(ScriptSkybox* self);
		static MonoObject* InternalGetTexture(ScriptSkybox* self);
	};
}
