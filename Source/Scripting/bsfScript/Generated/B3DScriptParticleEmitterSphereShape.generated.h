//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEmitterShape.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"

namespace b3d { class ParticleEmitterSphereShape; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterSphereShape : public TScriptReflectableWrapper<ParticleEmitterSphereShape, ScriptParticleEmitterSphereShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterSphereShape")

		ScriptParticleEmitterSphereShape(const TShared<ParticleEmitterSphereShape>& nativeObject);
		~ScriptParticleEmitterSphereShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleEmitterSphereShape* self, ParticleSphereShapeSettings* settings);
		static void InternalGetSettings(ScriptParticleEmitterSphereShape* self, ParticleSphereShapeSettings* __output);
		static void InternalCreate(MonoObject* scriptObject, ParticleSphereShapeSettings* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
