//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Physics/BsPhysics.h"
#include "BsScriptObject.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPhysics : public ScriptObject<ScriptPhysics>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Physics")

		ScriptPhysics(MonoObject* managedInstance);

	private:
		static void InternalToggleCollision(uint64_t groupA, uint64_t groupB, bool enabled);
		static bool InternalIsCollisionEnabled(uint64_t groupA, uint64_t groupB);
		static bool InternalIsUpdateInProgressInternal();
	};
}
