//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"

namespace bs { class CSkybox; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSkybox : public TScriptComponent<ScriptSkybox, CSkybox>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Skybox")

		ScriptSkybox(MonoObject* managedInstance, const GameObjectHandle<CSkybox>& value);

	private:
		static MonoObject* InternalGetTexture(ScriptSkybox* thisPtr);
		static void InternalSetTexture(ScriptSkybox* thisPtr, MonoObject* texture);
		static void InternalSetBrightness(ScriptSkybox* thisPtr, float brightness);
		static float InternalGetBrightness(ScriptSkybox* thisPtr);
	};
}
