//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"

namespace b3d { class ParticleEmitterStaticMeshShape; }
namespace b3d { struct __ParticleStaticMeshShapeSettingsInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterStaticMeshShape : public TScriptReflectableWrapper<ParticleEmitterStaticMeshShape, ScriptParticleEmitterStaticMeshShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterStaticMeshShape")

		ScriptParticleEmitterStaticMeshShape(const SPtr<ParticleEmitterStaticMeshShape>& nativeObject);
		~ScriptParticleEmitterStaticMeshShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetSettings(ScriptParticleEmitterStaticMeshShape* self, __ParticleStaticMeshShapeSettingsInterop* settings);
		static void InternalGetSettings(ScriptParticleEmitterStaticMeshShape* self, __ParticleStaticMeshShapeSettingsInterop* __output);
		static void InternalCreate(MonoObject* scriptObject, __ParticleStaticMeshShapeSettingsInterop* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
