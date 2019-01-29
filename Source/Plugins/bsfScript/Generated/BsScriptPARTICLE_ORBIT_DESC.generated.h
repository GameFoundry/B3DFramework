//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs
{
	struct __PARTICLE_ORBIT_DESCInterop
	{
		MonoObject* center;
		MonoObject* velocity;
		MonoObject* radial;
		bool worldSpace;
	};

	class BS_SCR_BE_EXPORT ScriptPARTICLE_ORBIT_DESC : public ScriptObject<ScriptPARTICLE_ORBIT_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleOrbitOptions")

		static MonoObject* box(const __PARTICLE_ORBIT_DESCInterop& value);
		static __PARTICLE_ORBIT_DESCInterop unbox(MonoObject* value);
		static PARTICLE_ORBIT_DESC fromInterop(const __PARTICLE_ORBIT_DESCInterop& value);
		static __PARTICLE_ORBIT_DESCInterop toInterop(const PARTICLE_ORBIT_DESC& value);

	private:
		ScriptPARTICLE_ORBIT_DESC(MonoObject* managedInstance);

	};
}
