//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"

namespace bs { class ParticleEmitterShape; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterShapeWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEmitterShape : public TScriptReflectableWrapper<ParticleEmitterShape, ScriptParticleEmitterShape, ScriptParticleEmitterShapeWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEmitterShape")

		ScriptParticleEmitterShape(const SPtr<ParticleEmitterShape>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
