//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "BsScriptCCollider.generated.h"

namespace b3d { class MeshCollider; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMeshCollider : public TScriptGameObjectWrapper<MeshCollider, ScriptMeshCollider, ScriptColliderWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MeshCollider")

		ScriptMeshCollider(const GameObjectHandle<MeshCollider>& nativeObject);
		~ScriptMeshCollider();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetMesh(ScriptMeshCollider* self, MonoObject* mesh);
		static MonoObject* InternalGetMesh(ScriptMeshCollider* self);
	};
}
