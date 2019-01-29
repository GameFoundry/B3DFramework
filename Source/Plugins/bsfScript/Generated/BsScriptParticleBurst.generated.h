//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleDistribution.h"

namespace bs
{
	struct __ParticleBurstInterop
	{
		float time;
		MonoObject* count;
		uint32_t cycles;
		float interval;
	};

	class BS_SCR_BE_EXPORT ScriptParticleBurst : public ScriptObject<ScriptParticleBurst>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleBurst")

		static MonoObject* box(const __ParticleBurstInterop& value);
		static __ParticleBurstInterop unbox(MonoObject* value);
		static ParticleBurst fromInterop(const __ParticleBurstInterop& value);
		static __ParticleBurstInterop toInterop(const ParticleBurst& value);

	private:
		ScriptParticleBurst(MonoObject* managedInstance);

	};
}
