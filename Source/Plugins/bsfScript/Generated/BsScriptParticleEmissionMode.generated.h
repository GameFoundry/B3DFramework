//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmissionMode : public ScriptObject<ScriptParticleEmissionMode>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmissionMode")

		static MonoObject* box(const ParticleEmissionMode& value);
		static ParticleEmissionMode unbox(MonoObject* value);

	private:
		ScriptParticleEmissionMode(MonoObject* managedInstance);

	};
}
