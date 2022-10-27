//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs
{
	class CBoxCollider;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCBoxCollider : public TScriptComponent<ScriptCBoxCollider, CBoxCollider, ScriptCColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "BoxCollider")

		ScriptCBoxCollider(MonoObject* managedInstance, const GameObjectHandle<CBoxCollider>& value);

	private:
		static void InternalSetExtents(ScriptCBoxCollider* thisPtr, Vector3* extents);
		static void InternalGetExtents(ScriptCBoxCollider* thisPtr, Vector3* __output);
		static void InternalSetCenter(ScriptCBoxCollider* thisPtr, Vector3* center);
		static void InternalGetCenter(ScriptCBoxCollider* thisPtr, Vector3* __output);
	};
} // namespace bs
