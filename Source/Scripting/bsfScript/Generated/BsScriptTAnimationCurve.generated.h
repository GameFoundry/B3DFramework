//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "Math/BsVector2.h"
#include "Math/BsQuaternion.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"

namespace bs
{
	template <class T0>
	class TAnimationCurve;
}

namespace bs
{
	struct __TKeyframeVector2Interop;
}

namespace bs
{
	struct __TKeyframeVector3Interop;
}

namespace bs
{
	struct __TKeyframeQuaternionInterop;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptTAnimationCurvefloat : public ScriptObject<ScriptTAnimationCurvefloat>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AnimationCurve")

		ScriptTAnimationCurvefloat(MonoObject* managedInstance, const SPtr<TAnimationCurve<float>>& value);

		SPtr<TAnimationCurve<float>> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<TAnimationCurve<float>>& value);

	private:
		SPtr<TAnimationCurve<float>> mInternal;

		static void InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes);
		static float InternalEvaluate(ScriptTAnimationCurvefloat* thisPtr, float time, bool loop);
		static MonoArray* InternalGetKeyFrames(ScriptTAnimationCurvefloat* thisPtr);
	};

	class BS_SCR_BE_EXPORT ScriptTAnimationCurveVector3 : public ScriptObject<ScriptTAnimationCurveVector3>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector3Curve")

		ScriptTAnimationCurveVector3(MonoObject* managedInstance, const SPtr<TAnimationCurve<Vector3>>& value);

		SPtr<TAnimationCurve<Vector3>> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<TAnimationCurve<Vector3>>& value);

	private:
		SPtr<TAnimationCurve<Vector3>> mInternal;

		static void InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes);
		static void InternalEvaluate(ScriptTAnimationCurveVector3* thisPtr, float time, bool loop, Vector3* __output);
		static MonoArray* InternalGetKeyFrames(ScriptTAnimationCurveVector3* thisPtr);
	};

	class BS_SCR_BE_EXPORT ScriptTAnimationCurveVector2 : public ScriptObject<ScriptTAnimationCurveVector2>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector2Curve")

		ScriptTAnimationCurveVector2(MonoObject* managedInstance, const SPtr<TAnimationCurve<Vector2>>& value);

		SPtr<TAnimationCurve<Vector2>> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<TAnimationCurve<Vector2>>& value);

	private:
		SPtr<TAnimationCurve<Vector2>> mInternal;

		static void InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes);
		static void InternalEvaluate(ScriptTAnimationCurveVector2* thisPtr, float time, bool loop, Vector2* __output);
		static MonoArray* InternalGetKeyFrames(ScriptTAnimationCurveVector2* thisPtr);
	};

	class BS_SCR_BE_EXPORT ScriptTAnimationCurveQuaternion : public ScriptObject<ScriptTAnimationCurveQuaternion>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "QuaternionCurve")

		ScriptTAnimationCurveQuaternion(MonoObject* managedInstance, const SPtr<TAnimationCurve<Quaternion>>& value);

		SPtr<TAnimationCurve<Quaternion>> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<TAnimationCurve<Quaternion>>& value);

	private:
		SPtr<TAnimationCurve<Quaternion>> mInternal;

		static void InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes);
		static void InternalEvaluate(ScriptTAnimationCurveQuaternion* thisPtr, float time, bool loop, Quaternion* __output);
		static MonoArray* InternalGetKeyFrames(ScriptTAnimationCurveQuaternion* thisPtr);
	};

	class BS_SCR_BE_EXPORT ScriptTAnimationCurveint32_t : public ScriptObject<ScriptTAnimationCurveint32_t>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "IntegerCurve")

		ScriptTAnimationCurveint32_t(MonoObject* managedInstance, const SPtr<TAnimationCurve<int32_t>>& value);

		SPtr<TAnimationCurve<int32_t>> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<TAnimationCurve<int32_t>>& value);

	private:
		SPtr<TAnimationCurve<int32_t>> mInternal;

		static void InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes);
		static int32_t InternalEvaluate(ScriptTAnimationCurveint32_t* thisPtr, float time, bool loop);
		static MonoArray* InternalGetKeyFrames(ScriptTAnimationCurveint32_t* thisPtr);
	};
} // namespace bs
