//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfUtility/Prerequisites/BsFwdDeclUtil.h"

namespace bs { class AnimationUtility; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptAnimationUtility : public ScriptObject<ScriptAnimationUtility>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AnimationUtility")

		ScriptAnimationUtility(MonoObject* managedInstance, const SPtr<AnimationUtility>& value);

		SPtr<AnimationUtility> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<AnimationUtility>& value);

	private:
		SPtr<AnimationUtility> mInternal;

		static MonoObject* Internal_eulerToQuaternionCurve(MonoObject* eulerCurve, EulerAngleOrder order);
		static MonoObject* Internal_quaternionToEulerCurve(MonoObject* quatCurve);
		static MonoArray* Internal_splitCurve3D(MonoObject* compoundCurve);
		static MonoObject* Internal_combineCurve3D(MonoArray* curveComponents);
		static MonoArray* Internal_splitCurve2D(MonoObject* compoundCurve);
		static MonoObject* Internal_combineCurve2D(MonoArray* curveComponents);
		static void Internal_calculateRange(MonoArray* curves, float* xMin, float* xMax, float* yMin, float* yMax);
	};
}
