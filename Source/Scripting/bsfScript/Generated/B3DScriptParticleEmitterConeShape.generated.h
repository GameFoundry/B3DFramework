//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEmitterShape.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"

namespace b3d { class ParticleEmitterConeShape; }
namespace b3d { struct __ParticleConeShapeSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterConeShape : public TScriptReflectableWrapper<ParticleEmitterConeShape, ScriptParticleEmitterConeShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterConeShape")

		ScriptParticleEmitterConeShape(const TShared<ParticleEmitterConeShape>& nativeObject);
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
