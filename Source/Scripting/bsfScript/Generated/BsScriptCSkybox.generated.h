//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"

namespace bs
{
	class CSkybox;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCSkybox : public TScriptComponent<ScriptCSkybox, CSkybox>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Skybox")

		ScriptCSkybox(MonoObject* managedInstance, const GameObjectHandle<CSkybox>& value);

	private:
		static MonoObject* InternalGetTexture(ScriptCSkybox* thisPtr);
		static void InternalSetTexture(ScriptCSkybox* thisPtr, MonoObject* texture);
		static void InternalSetBrightness(ScriptCSkybox* thisPtr, float brightness);
		static float InternalGetBrightness(ScriptCSkybox* thisPtr);
	};
} // namespace bs
