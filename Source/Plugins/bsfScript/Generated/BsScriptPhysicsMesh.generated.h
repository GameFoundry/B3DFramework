//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../../../Foundation/bsfCore/Utility/BsCommonTypes.h"

namespace bs
{
	class PhysicsMesh;
	class PhysicsMeshEx;

	class BS_SCR_BE_EXPORT ScriptPhysicsMesh : public TScriptResource<ScriptPhysicsMesh, PhysicsMesh>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "PhysicsMesh")

		ScriptPhysicsMesh(MonoObject* managedInstance, const ResourceHandle<PhysicsMesh>& value);

		static MonoObject* createInstance();

	private:
		static MonoObject* Internal_getRef(ScriptPhysicsMesh* thisPtr);

		static PhysicsMeshType Internal_getType(ScriptPhysicsMesh* thisPtr);
		static void Internal_create(MonoObject* managedInstance, MonoObject* meshData, PhysicsMeshType type);
		static MonoObject* Internal_getMeshData(ScriptPhysicsMesh* thisPtr);
	};
}
