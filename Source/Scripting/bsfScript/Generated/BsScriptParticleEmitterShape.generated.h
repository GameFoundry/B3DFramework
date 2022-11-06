//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterShape; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterShapeBase : public ScriptReflectableBase
	{
	public:
		ScriptParticleEmitterShapeBase(MonoObject* instance);
		virtual ~ScriptParticleEmitterShapeBase() {}

		SPtr<ParticleEmitterShape> GetInternal() const;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterShape : public TScriptReflectable<ScriptParticleEmitterShape, ParticleEmitterShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleEmitterShape")

		ScriptParticleEmitterShape(MonoObject* managedInstance, const SPtr<ParticleEmitterShape>& value);

		static MonoObject* Create(const SPtr<ParticleEmitterShape>& value);

	private:
	};
}
