//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEmitterShape.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"

namespace b3d { class ParticleEmitterHemisphereShape; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterHemisphereShape : public TScriptReflectableWrapper<ParticleEmitterHemisphereShape, ScriptParticleEmitterHemisphereShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterHemisphereShape")

		ScriptParticleEmitterHemisphereShape(const TShared<ParticleEmitterHemisphereShape>& nativeObject);
		~ScriptParticleEmitterHemisphereShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleEmitterHemisphereShape* self, ParticleHemisphereShapeSettings* settings);
		static void InternalGetSettings(ScriptParticleEmitterHemisphereShape* self, ParticleHemisphereShapeSettings* __output);
		static void InternalCreate(MonoObject* scriptObject, ParticleHemisphereShapeSettings* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
