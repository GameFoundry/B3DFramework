//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { struct __PARTICLE_CIRCLE_SHAPE_DESCInterop; }
namespace bs { class ParticleEmitterCircleShape; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterCircleShape : public TScriptReflectableWrapper<ParticleEmitterCircleShape, ScriptParticleEmitterCircleShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterCircleShape")

		ScriptParticleEmitterCircleShape(const SPtr<ParticleEmitterCircleShape>& nativeObject);
		~ScriptParticleEmitterCircleShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleEmitterCircleShape* self, __PARTICLE_CIRCLE_SHAPE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleEmitterCircleShape* self, __PARTICLE_CIRCLE_SHAPE_DESCInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __PARTICLE_CIRCLE_SHAPE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
