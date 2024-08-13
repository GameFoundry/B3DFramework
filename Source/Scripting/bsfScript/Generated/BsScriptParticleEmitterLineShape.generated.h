//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterLineShape; }
namespace bs { struct __PARTICLE_LINE_SHAPE_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterLineShape : public TScriptReflectable<ScriptParticleEmitterLineShape, ParticleEmitterLineShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleEmitterLineShape")

		ScriptParticleEmitterLineShape(MonoObject* managedInstance, const SPtr<ParticleEmitterLineShape>& value);

		static MonoObject* Create(const SPtr<ParticleEmitterLineShape>& value);

	private:
		static void InternalSetOptions(ScriptParticleEmitterLineShape* self, __PARTICLE_LINE_SHAPE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleEmitterLineShape* self, __PARTICLE_LINE_SHAPE_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_LINE_SHAPE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
}
