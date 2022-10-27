//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class Physics;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPhysics : public ScriptObject<ScriptPhysics>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Physics")

		ScriptPhysics(MonoObject* managedInstance);

	private:
		static void InternalToggleCollision(uint64_t groupA, uint64_t groupB, bool enabled);
		static bool InternalIsCollisionEnabled(uint64_t groupA, uint64_t groupB);
		static bool InternalIsUpdateInProgressInternal();
	};
} // namespace bs
