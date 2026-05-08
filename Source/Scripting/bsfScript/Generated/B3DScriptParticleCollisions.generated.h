//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptParticleEvolver.generated.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"
#include "../../../Engine/Utility/Math/B3DPlane.h"
#include "../../../Engine/Core/Particles/B3DParticleEvolver.h"

namespace b3d { class ParticleCollisions; }
namespace b3d { struct __TPlane_float_Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptParticleCollisions : public TScriptReflectableWrapper<ParticleCollisions, ScriptParticleCollisions, ScriptParticleEvolverWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ParticleCollisions")

		ScriptParticleCollisions(const TShared<ParticleCollisions>& nativeObject);
		~ScriptParticleCollisions();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetPlanes(ScriptParticleCollisions* self, MonoArray* planes);
		static MonoArray* InternalGetPlanes(ScriptParticleCollisions* self);
		static void InternalSetPlaneObjects(ScriptParticleCollisions* self, MonoArray* objects);
		static MonoArray* InternalGetPlaneObjects(ScriptParticleCollisions* self);
		static void InternalSetSettings(ScriptParticleCollisions* self, ParticleCollisionSettings* settings);
		static void InternalGetSettings(ScriptParticleCollisions* self, ParticleCollisionSettings* __output);
		static void InternalCreate(MonoObject* scriptObject, ParticleCollisionSettings* settings);
		static void InternalCreate0(MonoObject* scriptObject);
	};
}
