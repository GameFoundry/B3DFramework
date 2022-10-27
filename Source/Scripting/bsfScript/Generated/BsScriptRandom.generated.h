//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"
#include "Math/BsDegree.h"

namespace bs
{
	class Random;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptRandom : public ScriptObject<ScriptRandom>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Random")

		ScriptRandom(MonoObject* managedInstance, const SPtr<Random>& value);

		SPtr<Random> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<Random>& value);

	private:
		SPtr<Random> mInternal;

		static void InternalRandom(MonoObject* managedInstance, uint32_t seed);
		static void InternalSetSeed(ScriptRandom* thisPtr, uint32_t seed);
		static uint32_t InternalGet(ScriptRandom* thisPtr);
		static int32_t InternalGetRange(ScriptRandom* thisPtr, int32_t min, int32_t max);
		static float InternalGetUNorm(ScriptRandom* thisPtr);
		static float InternalGetSNorm(ScriptRandom* thisPtr);
		static void InternalGetUnitVector(ScriptRandom* thisPtr, Vector3* __output);
		static void InternalGetUnitVector2D(ScriptRandom* thisPtr, Vector2* __output);
		static void InternalGetPointInSphere(ScriptRandom* thisPtr, Vector3* __output);
		static void InternalGetPointInSphereShell(ScriptRandom* thisPtr, float thickness, Vector3* __output);
		static void InternalGetPointInCircle(ScriptRandom* thisPtr, Vector2* __output);
		static void InternalGetPointInCircleShell(ScriptRandom* thisPtr, float thickness, Vector2* __output);
		static void InternalGetPointInArc(ScriptRandom* thisPtr, Degree* angle, Vector2* __output);
		static void InternalGetPointInArcShell(ScriptRandom* thisPtr, Degree* angle, float thickness, Vector2* __output);
		static void InternalGetBarycentric(ScriptRandom* thisPtr, Vector3* __output);
	};
} // namespace bs
