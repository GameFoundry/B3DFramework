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
	struct __PARTICLE_SIZE_DESCInterop
	{
		MonoObject* Size;
		MonoObject* Size3D;
		bool Use3DSize;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSizeOptions : public TScriptTypeDefinition<ScriptParticleSizeOptions>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleSizeOptions")

		static MonoObject* Box(const __PARTICLE_SIZE_DESCInterop& value);
		static __PARTICLE_SIZE_DESCInterop Unbox(MonoObject* value);
		static PARTICLE_SIZE_DESC FromInterop(const __PARTICLE_SIZE_DESCInterop& value);
		static __PARTICLE_SIZE_DESCInterop ToInterop(const PARTICLE_SIZE_DESC& value);

	private:
		ScriptParticleSizeOptions();

	};
}
