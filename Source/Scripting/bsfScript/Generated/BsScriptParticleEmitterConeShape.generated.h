//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { struct __PARTICLE_CONE_SHAPE_DESCInterop; }
namespace bs { class ParticleEmitterConeShape; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterConeShape : public TScriptReflectableWrapper<ParticleEmitterConeShape, ScriptParticleEmitterConeShape>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ParticleEmitterConeShape")

		ScriptParticleEmitterConeShape(const SPtr<ParticleEmitterConeShape>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleEmitterConeShape* self, __PARTICLE_CONE_SHAPE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleEmitterConeShape* self, __PARTICLE_CONE_SHAPE_DESCInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __PARTICLE_CONE_SHAPE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
