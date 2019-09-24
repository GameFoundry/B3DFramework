//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"

namespace bs { class CMeshCollider; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCMeshCollider : public TScriptComponent<ScriptCMeshCollider, CMeshCollider, ScriptCColliderBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "MeshCollider")

		ScriptCMeshCollider(MonoObject* managedInstance, const GameObjectHandle<CMeshCollider>& value);

	private:
		static void Internal_setMesh(ScriptCMeshCollider* thisPtr, MonoObject* mesh);
		static MonoObject* Internal_getMesh(ScriptCMeshCollider* thisPtr);
	};
}
