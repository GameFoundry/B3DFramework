//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Utility/Math/B3DRandom.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Utility/Math/B3DVector3.h"
#include "../../../Engine/Utility/Math/B3DVector2.h"
#include "Math/B3DDegree.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRandom : public TScriptNonReflectableWrapper<Random, ScriptRandom>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Random")

		ScriptRandom(const TShared<Random>& nativeObject);
		~ScriptRandom();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalRandom(MonoObject* scriptObject, uint32_t seed);
		static void InternalSetSeed(ScriptRandom* self, uint32_t seed);
		static uint32_t InternalGet(ScriptRandom* self);
		static int32_t InternalGetRange(ScriptRandom* self, int32_t min, int32_t max);
		static float InternalGetUNorm(ScriptRandom* self);
		static float InternalGetSNorm(ScriptRandom* self);
		static void InternalGetUnitVector(ScriptRandom* self, TVector3<float>* __output);
		static void InternalGetUnitVector2D(ScriptRandom* self, TVector2<float>* __output);
		static void InternalGetPointInSphere(ScriptRandom* self, TVector3<float>* __output);
		static void InternalGetPointInSphereShell(ScriptRandom* self, float thickness, TVector3<float>* __output);
		static void InternalGetPointInCircle(ScriptRandom* self, TVector2<float>* __output);
		static void InternalGetPointInCircleShell(ScriptRandom* self, float thickness, TVector2<float>* __output);
		static void InternalGetPointInArc(ScriptRandom* self, TDegree<float>* angle, TVector2<float>* __output);
		static void InternalGetPointInArcShell(ScriptRandom* self, TDegree<float>* angle, float thickness, TVector2<float>* __output);
		static void InternalGetBarycentric(ScriptRandom* self, TVector3<float>* __output);
	};
}
