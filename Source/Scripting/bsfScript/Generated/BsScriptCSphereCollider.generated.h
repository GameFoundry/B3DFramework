//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs { class CSphereCollider; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCSphereCollider : public TScriptComponent<ScriptCSphereCollider, CSphereCollider, ScriptCColliderBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "SphereCollider")

		ScriptCSphereCollider(MonoObject* managedInstance, const GameObjectHandle<CSphereCollider>& value);

	private:
		static void Internal_setRadius(ScriptCSphereCollider* thisPtr, float radius);
		static float Internal_getRadius(ScriptCSphereCollider* thisPtr);
		static void Internal_setCenter(ScriptCSphereCollider* thisPtr, Vector3* center);
		static void Internal_getCenter(ScriptCSphereCollider* thisPtr, Vector3* __output);
	};
}
