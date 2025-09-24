//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"

namespace b3d
{
	struct __ParticleStaticMeshShapeSettingsInterop
	{
		ParticleEmitterMeshType Type;
		bool Sequential;
		MonoObject* Mesh;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleStaticMeshShapeSettings : public TScriptTypeDefinition<ScriptParticleStaticMeshShapeSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleStaticMeshShapeSettings")

		static MonoObject* Box(const __ParticleStaticMeshShapeSettingsInterop& value);
		static __ParticleStaticMeshShapeSettingsInterop Unbox(MonoObject* value);
		static ParticleStaticMeshShapeSettings FromInterop(const __ParticleStaticMeshShapeSettingsInterop& value);
		static __ParticleStaticMeshShapeSettingsInterop ToInterop(const ParticleStaticMeshShapeSettings& value);

	private:
		ScriptParticleStaticMeshShapeSettings();

	};
}
