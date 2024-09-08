//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterHemisphereShape; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterHemisphereShape : public TScriptReflectableWrapper<ParticleEmitterHemisphereShape, ScriptParticleEmitterHemisphereShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "ParticleEmitterHemisphereShape")

		ScriptParticleEmitterHemisphereShape(const SPtr<ParticleEmitterHemisphereShape>& nativeObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetOptions(ScriptParticleEmitterHemisphereShape* self, PARTICLE_HEMISPHERE_SHAPE_DESC* options);
		static void InternalGetOptions(ScriptParticleEmitterHemisphereShape* self, PARTICLE_HEMISPHERE_SHAPE_DESC* __output);
		static void InternalCreate(MonoObject* scriptObject, PARTICLE_HEMISPHERE_SHAPE_DESC* desc);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
