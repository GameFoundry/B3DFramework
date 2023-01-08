//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs
{
	struct __PARTICLE_VELOCITY_DESCInterop
	{
		MonoObject* Velocity;
		bool WorldSpace;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleVelocityOptions : public ScriptObject<ScriptParticleVelocityOptions>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleVelocityOptions")

		static MonoObject* Box(const __PARTICLE_VELOCITY_DESCInterop& value);
		static __PARTICLE_VELOCITY_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_VELOCITY_DESC FromInterop(const __PARTICLE_VELOCITY_DESCInterop& value);
		static __PARTICLE_VELOCITY_DESCInterop ToInterop(const PARTICLE_VELOCITY_DESC& value);

	private:
		ScriptParticleVelocityOptions(MonoObject* managedInstance);

	};
}
