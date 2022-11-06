//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterRectShape; }
namespace bs { struct __PARTICLE_RECT_SHAPE_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterRectShape : public TScriptReflectable<ScriptParticleEmitterRectShape, ParticleEmitterRectShape, ScriptParticleEmitterShapeBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "ParticleEmitterRectShape")

		ScriptParticleEmitterRectShape(MonoObject* managedInstance, const SPtr<ParticleEmitterRectShape>& value);

		static MonoObject* Create(const SPtr<ParticleEmitterRectShape>& value);

	private:
		static void InternalSetOptions(ScriptParticleEmitterRectShape* thisPtr, __PARTICLE_RECT_SHAPE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleEmitterRectShape* thisPtr, __PARTICLE_RECT_SHAPE_DESCInterop* __output);
		static void InternalCreate(MonoObject* managedInstance, __PARTICLE_RECT_SHAPE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* managedInstance);
	};
}
