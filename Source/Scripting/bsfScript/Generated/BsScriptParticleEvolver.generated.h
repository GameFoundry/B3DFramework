//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"

namespace bs { class ParticleEvolver; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEvolverBase : public ScriptReflectableBase
	{
	public:
		ScriptParticleEvolverBase(MonoObject* instance);
		virtual ~ScriptParticleEvolverBase() {}

		SPtr<ParticleEvolver> GetInternal() const;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEvolver : public TScriptReflectable<ScriptParticleEvolver, ParticleEvolver, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleEvolver")

		ScriptParticleEvolver(MonoObject* managedInstance, const SPtr<ParticleEvolver>& value);

		static MonoObject* Create(const SPtr<ParticleEvolver>& value);

	private:
	};
}
