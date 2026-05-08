//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEmitterShape.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"

namespace b3d { class ParticleEmitterRectShape; }
namespace b3d { struct __ParticleRectangleShapeSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterRectShape : public TScriptReflectableWrapper<ParticleEmitterRectShape, ScriptParticleEmitterRectShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterRectShape")

		ScriptParticleEmitterRectShape(const TShared<ParticleEmitterRectShape>& nativeObject);
		~ScriptParticleEmitterRectShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleEmitterRectShape* self, __ParticleRectangleShapeSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleEmitterRectShape* self, __ParticleRectangleShapeSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleRectangleShapeSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
