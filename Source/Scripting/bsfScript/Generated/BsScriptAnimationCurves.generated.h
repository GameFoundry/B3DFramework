//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"

namespace bs
{
	struct __TNamedAnimationCurvefloatInterop;
}

namespace bs
{
	struct __TNamedAnimationCurveQuaternionInterop;
}

namespace bs
{
	struct AnimationCurves;
}

namespace bs
{
	struct __TNamedAnimationCurveVector3Interop;
}

namespace bs
{
	class AnimationCurvesEx;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptAnimationCurves : public ScriptObject<ScriptAnimationCurves>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AnimationCurves")

		ScriptAnimationCurves(MonoObject* managedInstance, const SPtr<AnimationCurves>& value);

		SPtr<AnimationCurves> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<AnimationCurves>& value);

	private:
		SPtr<AnimationCurves> mInternal;

		static void InternalAnimationCurves(MonoObject* managedInstance);
		static void InternalAddPositionCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve);
		static void InternalAddRotationCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve);
		static void InternalAddScaleCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve);
		static void InternalAddGenericCurve(ScriptAnimationCurves* thisPtr, MonoString* name, MonoObject* curve);
		static void InternalRemovePositionCurve(ScriptAnimationCurves* thisPtr, MonoString* name);
		static void InternalRemoveRotationCurve(ScriptAnimationCurves* thisPtr, MonoString* name);
		static void InternalRemoveScaleCurve(ScriptAnimationCurves* thisPtr, MonoString* name);
		static void InternalRemoveGenericCurve(ScriptAnimationCurves* thisPtr, MonoString* name);
		static MonoArray* InternalGetPositionCurves(ScriptAnimationCurves* thisPtr);
		static void InternalSetPositionCurves(ScriptAnimationCurves* thisPtr, MonoArray* value);
		static MonoArray* InternalGetRotationCurves(ScriptAnimationCurves* thisPtr);
		static void InternalSetRotationCurves(ScriptAnimationCurves* thisPtr, MonoArray* value);
		static MonoArray* InternalGetScaleCurves(ScriptAnimationCurves* thisPtr);
		static void InternalSetScaleCurves(ScriptAnimationCurves* thisPtr, MonoArray* value);
		static MonoArray* InternalGetGenericCurves(ScriptAnimationCurves* thisPtr);
		static void InternalSetGenericCurves(ScriptAnimationCurves* thisPtr, MonoArray* value);
	};
} // namespace bs
