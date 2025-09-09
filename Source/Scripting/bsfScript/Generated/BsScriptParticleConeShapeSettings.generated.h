//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "Math/BsDegree.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace b3d
{
	struct __ParticleConeShapeSettingsInterop
	{
		ParticleEmitterConeType Type;
		float Radius;
		TDegree<float> Angle;
		float Length;
		float Thickness;
		TDegree<float> Arc;
		ParticleEmissionMode Mode;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleConeShapeSettings : public TScriptTypeDefinition<ScriptParticleConeShapeSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleConeShapeSettings")

		static MonoObject* Box(const __ParticleConeShapeSettingsInterop& value);
		static __ParticleConeShapeSettingsInterop Unbox(MonoObject* value);
		static ParticleConeShapeSettings FromInterop(const __ParticleConeShapeSettingsInterop& value);
		static __ParticleConeShapeSettingsInterop ToInterop(const ParticleConeShapeSettings& value);

	private:
		ScriptParticleConeShapeSettings();

	};
}
