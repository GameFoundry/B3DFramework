//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPARTICLE_GRAVITY_DESC : public ScriptObject<ScriptPARTICLE_GRAVITY_DESC>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleGravityOptions")

		static MonoObject* Box(const PARTICLE_GRAVITY_DESC& value);
		static PARTICLE_GRAVITY_DESC Unbox(MonoObject* value);

	private:
		ScriptPARTICLE_GRAVITY_DESC(MonoObject* managedInstance);

	};
}
