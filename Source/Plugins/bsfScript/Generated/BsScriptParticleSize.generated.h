//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsScriptParticleEvolver.generated.h"

namespace bs
{
	class ParticleSize;
	struct __PARTICLE_SIZE_DESCInterop;

	class BS_SCR_BE_EXPORT ScriptParticleSize : public ScriptObject<ScriptParticleSize, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "ParticleSize")

		ScriptParticleSize(MonoObject* managedInstance, const SPtr<ParticleSize>& value);

		SPtr<ParticleSize> getInternal() const;
		static MonoObject* create(const SPtr<ParticleSize>& value);

	private:
		static void Internal_setOptions(ScriptParticleSize* thisPtr, __PARTICLE_SIZE_DESCInterop* options);
		static void Internal_getOptions(ScriptParticleSize* thisPtr, __PARTICLE_SIZE_DESCInterop* __output);
		static void Internal_create(MonoObject* managedInstance, __PARTICLE_SIZE_DESCInterop* desc);
		static void Internal_create0(MonoObject* managedInstance);
	};
}
