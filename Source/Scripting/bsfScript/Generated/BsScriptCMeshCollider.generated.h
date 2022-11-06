//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"

namespace bs { class CMeshCollider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCMeshCollider : public TScriptComponent<ScriptCMeshCollider, CMeshCollider, ScriptCColliderBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "MeshCollider")

		ScriptCMeshCollider(MonoObject* managedInstance, const GameObjectHandle<CMeshCollider>& value);

	private:
		static void InternalSetMesh(ScriptCMeshCollider* thisPtr, MonoObject* mesh);
		static MonoObject* InternalGetMesh(ScriptCMeshCollider* thisPtr);
	};
}
