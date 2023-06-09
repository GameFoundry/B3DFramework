//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs { class CSphereCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSphereCollider : public TScriptComponent<ScriptSphereCollider, CSphereCollider, ScriptColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "SphereCollider")

		ScriptSphereCollider(MonoObject* managedInstance, const GameObjectHandle<CSphereCollider>& value);

	private:
		static void InternalSetRadius(ScriptSphereCollider* thisPtr, float radius);
		static float InternalGetRadius(ScriptSphereCollider* thisPtr);
		static void InternalSetCenter(ScriptSphereCollider* thisPtr, TVector3<float>* center);
		static void InternalGetCenter(ScriptSphereCollider* thisPtr, TVector3<float>* __output);
	};
}
