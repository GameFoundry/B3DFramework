//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"

namespace b3d { class PhysicsMesh; }
namespace b3d { class PhysicsMeshEx; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPhysicsMesh : public TScriptResourceWrapper<PhysicsMesh, ScriptPhysicsMesh>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "PhysicsMesh")

		ScriptPhysicsMesh(const TResourceHandle<PhysicsMesh>& nativeObject);
		~ScriptPhysicsMesh();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptPhysicsMesh* self);

		static PhysicsMeshType InternalGetType(ScriptPhysicsMesh* self);
		static void InternalCreate(MonoObject* scriptObject, MonoObject* meshData, PhysicsMeshType type);
		static MonoObject* InternalGetMeshData(ScriptPhysicsMesh* self);
	};
}
