//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace b3d
{
	struct __ParticleSkinnedMeshShapeSettingsInterop
	{
		ParticleEmitterMeshType Type;
		bool Sequential;
		MonoObject* Renderable;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleSkinnedMeshShapeSettings : public TScriptTypeDefinition<ScriptParticleSkinnedMeshShapeSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleSkinnedMeshShapeSettings")

		static MonoObject* Box(const __ParticleSkinnedMeshShapeSettingsInterop& value);
		static __ParticleSkinnedMeshShapeSettingsInterop Unbox(MonoObject* value);
		static ParticleSkinnedMeshShapeSettings FromInterop(const __ParticleSkinnedMeshShapeSettingsInterop& value);
		static __ParticleSkinnedMeshShapeSettingsInterop ToInterop(const ParticleSkinnedMeshShapeSettings& value);

	private:
		ScriptParticleSkinnedMeshShapeSettings();

	};
}
