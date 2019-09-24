//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"
#include "Math/BsDegree.h"

namespace bs { class Random; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptRandom : public ScriptObject<ScriptRandom>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Random")

		ScriptRandom(MonoObject* managedInstance, const SPtr<Random>& value);

		SPtr<Random> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<Random>& value);

	private:
		SPtr<Random> mInternal;

		static void Internal_Random(MonoObject* managedInstance, uint32_t seed);
		static void Internal_setSeed(ScriptRandom* thisPtr, uint32_t seed);
		static uint32_t Internal_get(ScriptRandom* thisPtr);
		static int32_t Internal_getRange(ScriptRandom* thisPtr, int32_t min, int32_t max);
		static float Internal_getUNorm(ScriptRandom* thisPtr);
		static float Internal_getSNorm(ScriptRandom* thisPtr);
		static void Internal_getUnitVector(ScriptRandom* thisPtr, Vector3* __output);
		static void Internal_getUnitVector2D(ScriptRandom* thisPtr, Vector2* __output);
		static void Internal_getPointInSphere(ScriptRandom* thisPtr, Vector3* __output);
		static void Internal_getPointInSphereShell(ScriptRandom* thisPtr, float thickness, Vector3* __output);
		static void Internal_getPointInCircle(ScriptRandom* thisPtr, Vector2* __output);
		static void Internal_getPointInCircleShell(ScriptRandom* thisPtr, float thickness, Vector2* __output);
		static void Internal_getPointInArc(ScriptRandom* thisPtr, Degree* angle, Vector2* __output);
		static void Internal_getPointInArcShell(ScriptRandom* thisPtr, Degree* angle, float thickness, Vector2* __output);
		static void Internal_getBarycentric(ScriptRandom* thisPtr, Vector3* __output);
	};
}
