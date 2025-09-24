//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Core/Physics/B3DPhysics.h"
#include "B3DScriptTypeDefinition.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPhysics : public TScriptTypeDefinition<ScriptPhysics>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Physics")

		ScriptPhysics();

		static void SetupScriptBindings();

	private:
		static void InternalToggleCollision(uint64_t groupA, uint64_t groupB, bool enabled);
		static bool InternalIsCollisionEnabled(uint64_t groupA, uint64_t groupB);
	};
}
