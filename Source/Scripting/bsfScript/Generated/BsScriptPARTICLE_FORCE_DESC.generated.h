//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
		MonoObject* Force;
		bool WorldSpace;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleForceOptions : public ScriptObject<ScriptParticleForceOptions>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleForceOptions")

		static MonoObject* Box(const __PARTICLE_FORCE_DESCInterop& value);
		static __PARTICLE_FORCE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_FORCE_DESC FromInterop(const __PARTICLE_FORCE_DESCInterop& value);
		static __PARTICLE_FORCE_DESCInterop ToInterop(const PARTICLE_FORCE_DESC& value);

	private:
		ScriptParticleForceOptions(MonoObject* managedInstance);

	};
}
