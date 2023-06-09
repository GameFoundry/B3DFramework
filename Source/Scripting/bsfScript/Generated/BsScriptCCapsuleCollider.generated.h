//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs { class CCapsuleCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCapsuleCollider : public TScriptComponent<ScriptCapsuleCollider, CCapsuleCollider, ScriptColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "CapsuleCollider")

		ScriptCapsuleCollider(MonoObject* managedInstance, const GameObjectHandle<CCapsuleCollider>& value);

	private:
		static void InternalSetNormal(ScriptCapsuleCollider* thisPtr, TVector3<float>* normal);
		static void InternalGetNormal(ScriptCapsuleCollider* thisPtr, TVector3<float>* __output);
		static void InternalSetCenter(ScriptCapsuleCollider* thisPtr, TVector3<float>* center);
		static void InternalGetCenter(ScriptCapsuleCollider* thisPtr, TVector3<float>* __output);
		static void InternalSetHalfHeight(ScriptCapsuleCollider* thisPtr, float halfHeight);
		static float InternalGetHalfHeight(ScriptCapsuleCollider* thisPtr);
		static void InternalSetRadius(ScriptCapsuleCollider* thisPtr, float radius);
		static float InternalGetRadius(ScriptCapsuleCollider* thisPtr);
	};
}
