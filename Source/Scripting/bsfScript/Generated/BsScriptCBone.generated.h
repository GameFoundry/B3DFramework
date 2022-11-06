//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"

namespace bs { class CBone; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCBone : public TScriptComponent<ScriptCBone, CBone>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Bone")

		ScriptCBone(MonoObject* managedInstance, const GameObjectHandle<CBone>& value);

	private:
		static void InternalSetBoneName(ScriptCBone* thisPtr, MonoString* name);
		static MonoString* InternalGetBoneName(ScriptCBone* thisPtr);
	};
}
