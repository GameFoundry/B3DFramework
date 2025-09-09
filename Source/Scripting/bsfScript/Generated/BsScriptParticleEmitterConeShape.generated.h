//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace b3d { class ParticleEmitterConeShape; }
namespace b3d { struct __ParticleConeShapeSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterConeShape : public TScriptReflectableWrapper<ParticleEmitterConeShape, ScriptParticleEmitterConeShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterConeShape")

		ScriptParticleEmitterConeShape(const SPtr<ParticleEmitterConeShape>& nativeObject);
		~ScriptParticleEmitterConeShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleEmitterConeShape* self, __ParticleConeShapeSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleEmitterConeShape* self, __ParticleConeShapeSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleConeShapeSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
