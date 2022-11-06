//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmissionMode : public ScriptObject<ScriptParticleEmissionMode>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleEmissionMode")

		static MonoObject* Box(const ParticleEmissionMode& value);
		static ParticleEmissionMode Unbox(MonoObject* value);

	private:
		ScriptParticleEmissionMode(MonoObject* managedInstance);

	};
}
