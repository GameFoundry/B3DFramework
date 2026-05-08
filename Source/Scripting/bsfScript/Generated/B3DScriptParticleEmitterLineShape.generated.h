//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEmitterShape.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"

namespace b3d { class ParticleEmitterLineShape; }
namespace b3d { struct __ParticleLineShapeSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterLineShape : public TScriptReflectableWrapper<ParticleEmitterLineShape, ScriptParticleEmitterLineShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterLineShape")

		ScriptParticleEmitterLineShape(const TShared<ParticleEmitterLineShape>& nativeObject);
		~ScriptParticleEmitterLineShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleEmitterLineShape* self, __ParticleLineShapeSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleEmitterLineShape* self, __ParticleLineShapeSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleLineShapeSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
