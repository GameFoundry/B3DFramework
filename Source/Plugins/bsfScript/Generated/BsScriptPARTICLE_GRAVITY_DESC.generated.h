//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptPARTICLE_GRAVITY_DESC : public ScriptObject<ScriptPARTICLE_GRAVITY_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleGravityOptions")

		static MonoObject* box(const PARTICLE_GRAVITY_DESC& value);
		static PARTICLE_GRAVITY_DESC unbox(MonoObject* value);

	private:
		ScriptPARTICLE_GRAVITY_DESC(MonoObject* managedInstance);

	};
}
