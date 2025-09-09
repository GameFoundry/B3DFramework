//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "../../../Foundation/bsfUtility/Math/BsVector2.h"

namespace b3d
{
	struct __ParticleRectangleShapeSettingsInterop
	{
		TVector2<float> Extents;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleRectShapeSettings : public TScriptTypeDefinition<ScriptParticleRectShapeSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleRectShapeSettings")

		static MonoObject* Box(const __ParticleRectangleShapeSettingsInterop& value);
		static __ParticleRectangleShapeSettingsInterop Unbox(MonoObject* value);
		static ParticleRectangleShapeSettings FromInterop(const __ParticleRectangleShapeSettingsInterop& value);
		static __ParticleRectangleShapeSettingsInterop ToInterop(const ParticleRectangleShapeSettings& value);

	private:
		ScriptParticleRectShapeSettings();

	};
}
