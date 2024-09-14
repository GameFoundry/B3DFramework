//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs
{
	struct __ParticleBurstInterop
	{
		float Time;
		MonoObject* Count;
		uint32_t Cycles;
		float Interval;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleBurst : public TScriptStructWrapper<ScriptParticleBurst>
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
