//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfUtility/Prerequisites/BsFwdDeclUtil.h"

namespace bs { class AnimationUtility; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationUtility : public ScriptObject<ScriptAnimationUtility>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AnimationUtility")

		ScriptAnimationUtility(MonoObject* managedInstance, const SPtr<AnimationUtility>& value);

		SPtr<AnimationUtility> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<AnimationUtility>& value);

	private:
		SPtr<AnimationUtility> mInternal;

		static MonoObject* InternalEulerToQuaternionCurve(MonoObject* eulerCurve, EulerAngleOrder order);
		static MonoObject* InternalQuaternionToEulerCurve(MonoObject* quatCurve);
		static MonoArray* InternalSplitCurve3D(MonoObject* compoundCurve);
		static MonoObject* InternalCombineCurve3D(MonoArray* curveComponents);
		static MonoArray* InternalSplitCurve2D(MonoObject* compoundCurve);
		static MonoObject* InternalCombineCurve2D(MonoArray* curveComponents);
		static void InternalCalculateRange(MonoArray* curves, float* xMin, float* xMax, float* yMin, float* yMax);
	};
}
