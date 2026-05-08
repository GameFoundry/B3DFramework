//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Components/B3DParticleSystem.h"
#include "../../../Engine/Utility/Math/B3DVector3.h"
#include "../../../Engine/Utility/Math/B3DQuaternion.h"
#include "../../../Engine/Core/Particles/B3DParticleDistribution.h"

namespace b3d { struct ParticleVectorFieldSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleVectorFieldSettings : public TScriptReflectableWrapper<ParticleVectorFieldSettings, ScriptParticleVectorFieldSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleVectorFieldSettings")

		ScriptParticleVectorFieldSettings(const TShared<ParticleVectorFieldSettings>& nativeObject);
		~ScriptParticleVectorFieldSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetVectorField(ScriptParticleVectorFieldSettings* self);
		static void InternalSetVectorField(ScriptParticleVectorFieldSettings* self, MonoObject* value);
		static float InternalGetIntensity(ScriptParticleVectorFieldSettings* self);
		static void InternalSetIntensity(ScriptParticleVectorFieldSettings* self, float value);
		static float InternalGetTightness(ScriptParticleVectorFieldSettings* self);
		static void InternalSetTightness(ScriptParticleVectorFieldSettings* self, float value);
		static void InternalGetScale(ScriptParticleVectorFieldSettings* self, TVector3<float>* __output);
		static void InternalSetScale(ScriptParticleVectorFieldSettings* self, TVector3<float>* value);
		static void InternalGetOffset(ScriptParticleVectorFieldSettings* self, TVector3<float>* __output);
		static void InternalSetOffset(ScriptParticleVectorFieldSettings* self, TVector3<float>* value);
		static void InternalGetRotation(ScriptParticleVectorFieldSettings* self, TQuaternion<float>* __output);
		static void InternalSetRotation(ScriptParticleVectorFieldSettings* self, TQuaternion<float>* value);
		static MonoObject* InternalGetRotationRate(ScriptParticleVectorFieldSettings* self);
		static void InternalSetRotationRate(ScriptParticleVectorFieldSettings* self, MonoObject* value);
		static bool InternalGetTilingX(ScriptParticleVectorFieldSettings* self);
		static void InternalSetTilingX(ScriptParticleVectorFieldSettings* self, bool value);
		static bool InternalGetTilingY(ScriptParticleVectorFieldSettings* self);
		static void InternalSetTilingY(ScriptParticleVectorFieldSettings* self, bool value);
		static bool InternalGetTilingZ(ScriptParticleVectorFieldSettings* self);
		static void InternalSetTilingZ(ScriptParticleVectorFieldSettings* self, bool value);
	};
}
