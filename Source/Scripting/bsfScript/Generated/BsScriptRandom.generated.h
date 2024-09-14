//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfUtility/Math/BsRandom.h"
#include "BsScriptObject.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"
#include "Math/BsDegree.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRandom : public ScriptObject<ScriptRandom>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Random")

		ScriptRandom(MonoObject* managedInstance, const SPtr<Random>& value);

		SPtr<Random> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<Random>& value);

	private:
		SPtr<Random> mInternal;

		static void InternalRandom(MonoObject* managedInstance, uint32_t seed);
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
