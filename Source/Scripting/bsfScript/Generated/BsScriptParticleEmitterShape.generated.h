//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterShape; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptParticleEmitterShapeBase : public ScriptReflectableBase
	{
	public:
		ScriptParticleEmitterShapeBase(MonoObject* instance);
		virtual ~ScriptParticleEmitterShapeBase() {}

		SPtr<ParticleEmitterShape> getInternal() const;
	};

	class BS_SCR_BE_EXPORT ScriptParticleEmitterShape : public TScriptReflectable<ScriptParticleEmitterShape, ParticleEmitterShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "ParticleEmitterShape")

		ScriptParticleEmitterShape(MonoObject* managedInstance, const SPtr<ParticleEmitterShape>& value);

		static MonoObject* create(const SPtr<ParticleEmitterShape>& value);

	private:
	};
}
