//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleEvolver; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEvolverBase : public ScriptReflectableBase
	{
	public:
		ScriptParticleEvolverBase(MonoObject* instance);
		virtual ~ScriptParticleEvolverBase() {}

		SPtr<ParticleEvolver> getInternal() const;
	};

	class BS_SCR_BE_EXPORT ScriptParticleEvolver : public TScriptReflectable<ScriptParticleEvolver, ParticleEvolver, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEvolver")

		ScriptParticleEvolver(MonoObject* managedInstance, const SPtr<ParticleEvolver>& value);

		static MonoObject* create(const SPtr<ParticleEvolver>& value);

	private:
	};
}
