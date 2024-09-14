//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs
{
	struct __PARTICLE_ORBIT_DESCInterop
	{
		MonoObject* Center;
		MonoObject* Velocity;
		MonoObject* Radial;
		bool WorldSpace;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleOrbitOptions : public TScriptStructWrapper<ScriptParticleOrbitOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleOrbitOptions")

		static MonoObject* Box(const __PARTICLE_ORBIT_DESCInterop& value);
		static __PARTICLE_ORBIT_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_ORBIT_DESC FromInterop(const __PARTICLE_ORBIT_DESCInterop& value);
		static __PARTICLE_ORBIT_DESCInterop ToInterop(const PARTICLE_ORBIT_DESC& value);

	private:
		ScriptParticleOrbitOptions();

	};
}
