//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"
#include "Math/B3DDegree.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"

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
