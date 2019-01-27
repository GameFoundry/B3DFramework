//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class ParticleEvolver;

	class BS_SCR_BE_EXPORT ScriptParticleEvolverBase : public ScriptObjectBase
	{
	public:
		ScriptParticleEvolverBase(MonoObject* instance);
		virtual ~ScriptParticleEvolverBase() {}

		SPtr<ParticleEvolver> getInternal() const { return mInternal; }
	protected:
		SPtr<ParticleEvolver> mInternal;
	};

	class BS_SCR_BE_EXPORT ScriptParticleEvolver : public ScriptObject<ScriptParticleEvolver, ScriptParticleEvolverBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "ParticleEvolver")

		ScriptParticleEvolver(MonoObject* managedInstance, const SPtr<ParticleEvolver>& value);

		SPtr<ParticleEvolver> getInternal() const;
		static MonoObject* create(const SPtr<ParticleEvolver>& value);

	private:
	};
}
