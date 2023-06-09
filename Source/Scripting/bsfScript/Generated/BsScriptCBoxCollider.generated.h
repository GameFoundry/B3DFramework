//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"
#include "Math/BsVector3.h"

namespace bs { class CBoxCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBoxCollider : public TScriptComponent<ScriptBoxCollider, CBoxCollider, ScriptColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "BoxCollider")

		ScriptBoxCollider(MonoObject* managedInstance, const GameObjectHandle<CBoxCollider>& value);

	private:
		static void InternalSetExtents(ScriptBoxCollider* thisPtr, TVector3<float>* extents);
		static void InternalGetExtents(ScriptBoxCollider* thisPtr, TVector3<float>* __output);
		static void InternalSetCenter(ScriptBoxCollider* thisPtr, TVector3<float>* center);
		static void InternalGetCenter(ScriptBoxCollider* thisPtr, TVector3<float>* __output);
	};
}
