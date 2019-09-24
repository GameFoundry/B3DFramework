//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"

namespace bs { class CBone; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCBone : public TScriptComponent<ScriptCBone, CBone>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Bone")

		ScriptCBone(MonoObject* managedInstance, const GameObjectHandle<CBone>& value);

	private:
		static void Internal_setBoneName(ScriptCBone* thisPtr, MonoString* name);
		static MonoString* Internal_getBoneName(ScriptCBone* thisPtr);
	};
}
