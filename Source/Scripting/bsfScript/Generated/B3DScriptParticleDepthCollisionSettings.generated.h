//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Components/BsParticleSystem.h"

namespace b3d { struct ParticleDepthCollisionSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleDepthCollisionSettings : public TScriptReflectableWrapper<ParticleDepthCollisionSettings, ScriptParticleDepthCollisionSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleDepthCollisionSettings")

		ScriptParticleDepthCollisionSettings(const SPtr<ParticleDepthCollisionSettings>& nativeObject);
		~ScriptParticleDepthCollisionSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalParticleDepthCollisionSettings(MonoObject* scriptObject);
		static bool InternalGetEnabled(ScriptParticleDepthCollisionSettings* self);
		static void InternalSetEnabled(ScriptParticleDepthCollisionSettings* self, bool value);
		static float InternalGetRestitution(ScriptParticleDepthCollisionSettings* self);
		static void InternalSetRestitution(ScriptParticleDepthCollisionSettings* self, float value);
		static float InternalGetDampening(ScriptParticleDepthCollisionSettings* self);
		static void InternalSetDampening(ScriptParticleDepthCollisionSettings* self, float value);
		static float InternalGetRadiusScale(ScriptParticleDepthCollisionSettings* self);
		static void InternalSetRadiusScale(ScriptParticleDepthCollisionSettings* self, float value);
	};
}
