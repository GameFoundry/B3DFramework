//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"

namespace bs
{
	class CSkybox;

	class BS_SCR_BE_EXPORT ScriptCSkybox : public TScriptComponent<ScriptCSkybox, CSkybox>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "Skybox")

		ScriptCSkybox(MonoObject* managedInstance, const GameObjectHandle<CSkybox>& value);

	private:
		static MonoObject* Internal_getTexture(ScriptCSkybox* thisPtr);
		static void Internal_setTexture(ScriptCSkybox* thisPtr, MonoObject* texture);
		static void Internal_setBrightness(ScriptCSkybox* thisPtr, float brightness);
		static float Internal_getBrightness(ScriptCSkybox* thisPtr);
	};
}
