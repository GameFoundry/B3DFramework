//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"

namespace b3d { class ParticleEmitterCircleShape; }
namespace b3d { struct __ParticleCircleShapeSettingsInterop; }
namespace b3d
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
		static void InternalSetSettings(ScriptParticleEmitterCircleShape* self, __ParticleCircleShapeSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleEmitterCircleShape* self, __ParticleCircleShapeSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleCircleShapeSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
