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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPlaneCollider : public TScriptComponent<ScriptPlaneCollider, CPlaneCollider, ScriptColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "PlaneCollider")

		ScriptPlaneCollider(MonoObject* managedInstance, const GameObjectHandle<CPlaneCollider>& value);

	private:
		static void InternalSetNormal(ScriptPlaneCollider* thisPtr, Vector3* normal);
		static void InternalGetNormal(ScriptPlaneCollider* thisPtr, Vector3* __output);
		static void InternalSetDistance(ScriptPlaneCollider* thisPtr, float distance);
		static float InternalGetDistance(ScriptPlaneCollider* thisPtr);
	};
}
