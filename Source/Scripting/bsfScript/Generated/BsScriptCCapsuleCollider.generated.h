//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs { class CCapsuleCollider; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCCapsuleCollider : public TScriptComponent<ScriptCCapsuleCollider, CCapsuleCollider, ScriptCColliderBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "CapsuleCollider")

		ScriptCCapsuleCollider(MonoObject* managedInstance, const GameObjectHandle<CCapsuleCollider>& value);

	private:
		static void Internal_setNormal(ScriptCCapsuleCollider* thisPtr, Vector3* normal);
		static void Internal_getNormal(ScriptCCapsuleCollider* thisPtr, Vector3* __output);
		static void Internal_setCenter(ScriptCCapsuleCollider* thisPtr, Vector3* center);
		static void Internal_getCenter(ScriptCCapsuleCollider* thisPtr, Vector3* __output);
		static void Internal_setHalfHeight(ScriptCCapsuleCollider* thisPtr, float halfHeight);
		static float Internal_getHalfHeight(ScriptCCapsuleCollider* thisPtr);
		static void Internal_setRadius(ScriptCCapsuleCollider* thisPtr, float radius);
		static float Internal_getRadius(ScriptCCapsuleCollider* thisPtr);
	};
}
