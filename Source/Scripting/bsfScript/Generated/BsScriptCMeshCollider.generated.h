//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"

namespace bs { class CMeshCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMeshCollider : public TScriptComponent<ScriptMeshCollider, CMeshCollider, ScriptColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "MeshCollider")

		ScriptMeshCollider(MonoObject* managedInstance, const GameObjectHandle<CMeshCollider>& value);

	private:
		static void InternalSetMesh(ScriptMeshCollider* thisPtr, MonoObject* mesh);
		static MonoObject* InternalGetMesh(ScriptMeshCollider* thisPtr);
	};
}
