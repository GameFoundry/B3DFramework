//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterBoxShape; }
namespace bs { struct __PARTICLE_BOX_SHAPE_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterBoxShape : public TScriptReflectable<ScriptParticleEmitterBoxShape, ParticleEmitterBoxShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleEmitterBoxShape")

		ScriptParticleEmitterBoxShape(MonoObject* managedInstance, const SPtr<ParticleEmitterBoxShape>& value);

		static MonoObject* Create(const SPtr<ParticleEmitterBoxShape>& value);

	private:
		static void InternalSetOptions(ScriptParticleEmitterBoxShape* thisPtr, __PARTICLE_BOX_SHAPE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleEmitterBoxShape* thisPtr, __PARTICLE_BOX_SHAPE_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_BOX_SHAPE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
}
