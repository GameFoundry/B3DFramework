//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"

namespace b3d { class Skybox; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSkybox : public TScriptGameObjectWrapper<Skybox, ScriptSkybox>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Skybox")

		ScriptSkybox(const TGameObjectHandle<Skybox>& nativeObject);
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
