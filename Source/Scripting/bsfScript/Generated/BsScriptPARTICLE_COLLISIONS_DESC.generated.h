//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPARTICLE_COLLISIONS_DESC : public ScriptObject<ScriptPARTICLE_COLLISIONS_DESC>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleCollisionsOptions")

		static MonoObject* Box(const PARTICLE_COLLISIONS_DESC& value);
		static PARTICLE_COLLISIONS_DESC Unbox(MonoObject* value);

	private:
		ScriptPARTICLE_COLLISIONS_DESC(MonoObject* managedInstance);
	};
} // namespace bs
