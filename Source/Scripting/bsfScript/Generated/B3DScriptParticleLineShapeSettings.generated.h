//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"

namespace b3d
{
	struct __ParticleLineShapeSettingsInterop
	{
		float Length;
		ParticleEmissionMode Mode;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleLineShapeSettings : public TScriptTypeDefinition<ScriptParticleLineShapeSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleLineShapeSettings")

		static MonoObject* Box(const __ParticleLineShapeSettingsInterop& value);
		static __ParticleLineShapeSettingsInterop Unbox(MonoObject* value);
		static ParticleLineShapeSettings FromInterop(const __ParticleLineShapeSettingsInterop& value);
		static __ParticleLineShapeSettingsInterop ToInterop(const ParticleLineShapeSettings& value);

	private:
		ScriptParticleLineShapeSettings();

	};
}
