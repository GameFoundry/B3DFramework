//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleCollisionsOptions : public TScriptStructWrapper<ScriptParticleCollisionsOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleCollisionsOptions")

		static MonoObject* Box(const PARTICLE_COLLISIONS_DESC& value);
		static PARTICLE_COLLISIONS_DESC Unbox(MonoObject* value);

	private:
		ScriptParticleCollisionsOptions();

	};
}
