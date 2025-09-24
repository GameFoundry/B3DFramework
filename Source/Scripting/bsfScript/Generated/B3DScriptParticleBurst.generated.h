//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"
#include "../../../Foundation/Core/Particles/B3DParticleDistribution.h"

namespace b3d
{
	struct __ParticleBurstInterop
	{
		float Time;
		MonoObject* Count;
		uint32_t Cycles;
		float Interval;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleBurst : public TScriptTypeDefinition<ScriptParticleBurst>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleBurst")

		static MonoObject* Box(const __ParticleBurstInterop& value);
		static __ParticleBurstInterop Unbox(MonoObject* value);
		static ParticleBurst FromInterop(const __ParticleBurstInterop& value);
		static __ParticleBurstInterop ToInterop(const ParticleBurst& value);

	private:
		ScriptParticleBurst();

	};
}
