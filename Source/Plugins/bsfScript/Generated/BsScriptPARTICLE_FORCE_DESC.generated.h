//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs
{
	struct __PARTICLE_FORCE_DESCInterop
	{
		MonoObject* force;
		bool worldSpace;
	};

	class BS_SCR_BE_EXPORT ScriptPARTICLE_FORCE_DESC : public ScriptObject<ScriptPARTICLE_FORCE_DESC>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleForceOptions")

		static MonoObject* box(const __PARTICLE_FORCE_DESCInterop& value);
		static __PARTICLE_FORCE_DESCInterop unbox(MonoObject* value);
		static PARTICLE_FORCE_DESC fromInterop(const __PARTICLE_FORCE_DESCInterop& value);
		static __PARTICLE_FORCE_DESCInterop toInterop(const PARTICLE_FORCE_DESC& value);

	private:
		ScriptPARTICLE_FORCE_DESC(MonoObject* managedInstance);

	};
}
