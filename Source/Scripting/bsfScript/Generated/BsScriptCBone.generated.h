//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"

namespace bs { class CBone; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBone : public TScriptComponent<ScriptBone, CBone>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Bone")

		ScriptBone(MonoObject* managedInstance, const GameObjectHandle<CBone>& value);

	private:
		static void InternalSetBoneName(ScriptBone* self, MonoString* name);
		static MonoString* InternalGetBoneName(ScriptBone* self);
	};
}
