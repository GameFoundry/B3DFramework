//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEmitterShape.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"
#include "../../../Engine/Core/Particles/B3DParticleEmitter.h"

namespace b3d { class ParticleEmitterSkinnedMeshShape; }
namespace b3d { struct __ParticleSkinnedMeshShapeSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterSkinnedMeshShape : public TScriptReflectableWrapper<ParticleEmitterSkinnedMeshShape, ScriptParticleEmitterSkinnedMeshShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterSkinnedMeshShape")

		ScriptParticleEmitterSkinnedMeshShape(const TShared<ParticleEmitterSkinnedMeshShape>& nativeObject);
		~ScriptParticleEmitterSkinnedMeshShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleEmitterSkinnedMeshShape* self, __ParticleSkinnedMeshShapeSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleEmitterSkinnedMeshShape* self, __ParticleSkinnedMeshShapeSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleSkinnedMeshShapeSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
