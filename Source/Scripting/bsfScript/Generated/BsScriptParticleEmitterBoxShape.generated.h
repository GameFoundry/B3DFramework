//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterBoxShape; }
namespace bs { struct __PARTICLE_BOX_SHAPE_DESCInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterBoxShape : public TScriptReflectableWrapper<ParticleEmitterBoxShape, ScriptParticleEmitterBoxShape>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ParticleEmitterBoxShape")

		ScriptParticleEmitterBoxShape(const SPtr<ParticleEmitterBoxShape>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleEmitterBoxShape* self, __PARTICLE_BOX_SHAPE_DESCInterop* options);
		static void InternalGetOptions(ScriptParticleEmitterBoxShape* self, __PARTICLE_BOX_SHAPE_DESCInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __PARTICLE_BOX_SHAPE_DESCInterop* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
