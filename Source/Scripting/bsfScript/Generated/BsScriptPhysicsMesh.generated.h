//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfCore/Utility/BsCommonTypes.h"

namespace bs
{
	class PhysicsMesh;
}

namespace bs
{
	class PhysicsMeshEx;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPhysicsMesh : public TScriptResource<ScriptPhysicsMesh, PhysicsMesh>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "PhysicsMesh")

		ScriptPhysicsMesh(MonoObject* managedInstance, const ResourceHandle<PhysicsMesh>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptPhysicsMesh* thisPtr);

		static PhysicsMeshType InternalGetType(ScriptPhysicsMesh* thisPtr);
		static void InternalCreate(MonoObject* managedInstance, MonoObject* meshData, PhysicsMeshType type);
		static MonoObject* InternalGetMeshData(ScriptPhysicsMesh* thisPtr);
	};
} // namespace bs
