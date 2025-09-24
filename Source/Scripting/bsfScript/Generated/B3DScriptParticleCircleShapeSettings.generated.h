//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"
#include "Math/B3DDegree.h"
#include "../../../Foundation/Core/Particles/B3DParticleEmitter.h"

namespace b3d
{
	struct __ParticleCircleShapeSettingsInterop
	{
		float Radius;
		float Thickness;
		TDegree<float> Arc;
		ParticleEmissionMode Mode;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleCircleShapeSettings : public TScriptTypeDefinition<ScriptParticleCircleShapeSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleCircleShapeSettings")

		static MonoObject* Box(const __ParticleCircleShapeSettingsInterop& value);
		static __ParticleCircleShapeSettingsInterop Unbox(MonoObject* value);
		static ParticleCircleShapeSettings FromInterop(const __ParticleCircleShapeSettingsInterop& value);
		static __ParticleCircleShapeSettingsInterop ToInterop(const ParticleCircleShapeSettings& value);

	private:
		ScriptParticleCircleShapeSettings();

	};
}
