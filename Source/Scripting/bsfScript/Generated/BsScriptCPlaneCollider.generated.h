//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs { class CPlaneCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCPlaneCollider : public TScriptComponent<ScriptCPlaneCollider, CPlaneCollider, ScriptCColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "PlaneCollider")

		ScriptCPlaneCollider(MonoObject* managedInstance, const GameObjectHandle<CPlaneCollider>& value);

	private:
		static void InternalSetNormal(ScriptCPlaneCollider* thisPtr, Vector3* normal);
		static void InternalGetNormal(ScriptCPlaneCollider* thisPtr, Vector3* __output);
		static void InternalSetDistance(ScriptCPlaneCollider* thisPtr, float distance);
		static float InternalGetDistance(ScriptCPlaneCollider* thisPtr);
	};
}
