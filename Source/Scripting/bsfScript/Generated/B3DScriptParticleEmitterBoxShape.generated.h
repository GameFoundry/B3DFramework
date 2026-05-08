//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEmitterShape.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"

namespace b3d { class ParticleEmitterBoxShape; }
namespace b3d { struct __ParticleBoxShapeSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterBoxShape : public TScriptReflectableWrapper<ParticleEmitterBoxShape, ScriptParticleEmitterBoxShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterBoxShape")

		ScriptParticleEmitterBoxShape(const TShared<ParticleEmitterBoxShape>& nativeObject);
		~ScriptParticleEmitterBoxShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleEmitterBoxShape* self, __ParticleBoxShapeSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleEmitterBoxShape* self, __ParticleBoxShapeSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleBoxShapeSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
