//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs
{
	struct __PARTICLE_COLOR_DESCInterop
	{
		MonoObject* Color;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleColorOptions : public ScriptObject<ScriptParticleColorOptions>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleColorOptions")

		static MonoObject* Box(const __PARTICLE_COLOR_DESCInterop& value);
		static __PARTICLE_COLOR_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_COLOR_DESC FromInterop(const __PARTICLE_COLOR_DESCInterop& value);
		static __PARTICLE_COLOR_DESCInterop ToInterop(const PARTICLE_COLOR_DESC& value);

	private:
		ScriptParticleColorOptions(MonoObject* managedInstance);

	};
}
