//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationUtility.h"
#include "BsScriptNonReflectableWrapper.h"
#include "../../../Foundation/bsfUtility/Prerequisites/BsFwdDeclUtil.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationUtility : public TScriptNonReflectableWrapper<AnimationUtility, ScriptAnimationUtility>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AnimationUtility")

		ScriptAnimationUtility(const SPtr<AnimationUtility>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalEulerToQuaternionCurve(MonoObject* eulerCurve, EulerAngleOrder order);
		static MonoObject* InternalQuaternionToEulerCurve(MonoObject* quatCurve);
		static MonoArray* InternalSplitCurve3D(MonoObject* compoundCurve);
		static MonoObject* InternalCombineCurve3D(MonoArray* curveComponents);
		static MonoArray* InternalSplitCurve2D(MonoObject* compoundCurve);
		static MonoObject* InternalCombineCurve2D(MonoArray* curveComponents);
		static void InternalCalculateRange(MonoArray* curves, float* xMin, float* xMax, float* yMin, float* yMax);
	};
}
