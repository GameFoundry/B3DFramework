//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs { class CBoxCollider; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCBoxCollider : public TScriptComponent<ScriptCBoxCollider, CBoxCollider, ScriptCColliderBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "BoxCollider")

		ScriptCBoxCollider(MonoObject* managedInstance, const GameObjectHandle<CBoxCollider>& value);

	private:
		static void Internal_setExtents(ScriptCBoxCollider* thisPtr, Vector3* extents);
		static void Internal_getExtents(ScriptCBoxCollider* thisPtr, Vector3* __output);
		static void Internal_setCenter(ScriptCBoxCollider* thisPtr, Vector3* center);
		static void Internal_getCenter(ScriptCBoxCollider* thisPtr, Vector3* __output);
	};
}
