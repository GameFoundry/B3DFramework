//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"

namespace b3d { class ParticleEvolver; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEvolverWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleEvolver : public TScriptReflectableWrapper<ParticleEvolver, ScriptParticleEvolver, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleEvolver")

		ScriptParticleEvolver(const TShared<ParticleEvolver>& nativeObject);
		~ScriptParticleEvolver();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
