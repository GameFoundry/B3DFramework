//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptPhysics.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Physics/B3DPhysics.h"

namespace b3d
{
	ScriptPhysics::ScriptPhysics()
		:TScriptTypeDefinition()
	{
	}

	void ScriptPhysics::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_ToggleCollision", (void*)&ScriptPhysics::InternalToggleCollision);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsCollisionEnabled", (void*)&ScriptPhysics::InternalIsCollisionEnabled);

	}

	void ScriptPhysics::InternalToggleCollision(uint64_t groupA, uint64_t groupB, bool enabled)
	{
		Physics::Instance().ToggleCollision(groupA, groupB, enabled);
	}

	bool ScriptPhysics::InternalIsCollisionEnabled(uint64_t groupA, uint64_t groupB)
	{
		bool tmp__output;
		tmp__output = Physics::Instance().IsCollisionEnabled(groupA, groupB);

		bool __output;
		__output = tmp__output;

		return __output;
	}
}
