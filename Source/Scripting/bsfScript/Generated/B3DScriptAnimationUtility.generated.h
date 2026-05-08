//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Core/Animation/B3DAnimationUtility.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Utility/Prerequisites/B3DFwdDeclUtil.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationUtility : public TScriptNonReflectableWrapper<AnimationUtility, ScriptAnimationUtility>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AnimationUtility")

		ScriptAnimationUtility(const TShared<AnimationUtility>& nativeObject);
		~ScriptAnimationUtility();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalEulerToQuaternionCurve(MonoObject* eulerCurve, EulerAngleOrder order);
		static MonoObject* InternalQuaternionToEulerCurve(MonoObject* quatCurve);
		static MonoArray* InternalSplitCurve3D(MonoObject* compoundCurve);
		static MonoObject* InternalCombineCurve3D(MonoArray* curveComponents);
		static MonoArray* InternalSplitCurve2D(MonoObject* compoundCurve);
		static MonoObject* InternalCombineCurve2D(MonoArray* curveComponents);
		static void InternalCalculateRange(MonoArray* curves, float* outXMin, float* outXMax, float* outYMin, float* outYMax);
	};
}
