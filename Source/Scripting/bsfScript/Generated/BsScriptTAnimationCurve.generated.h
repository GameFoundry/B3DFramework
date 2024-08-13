//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "Math/BsVector2.h"
#include "Math/BsQuaternion.h"

namespace bs { template<class T0> class TAnimationCurve; }
namespace bs { struct __TKeyframe_TVector3_float__Interop; }
namespace bs { struct __TKeyframe_TVector2_float__Interop; }
namespace bs { struct __TKeyframe_Quaternion_Interop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationCurve : public ScriptObject<ScriptAnimationCurve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AnimationCurve")

		ScriptAnimationCurve(MonoObject* managedInstance, const SPtr<TAnimationCurve<float>>& value);

		SPtr<TAnimationCurve<float>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TAnimationCurve<float>>& value);

	private:
		SPtr<TAnimationCurve<float>> mInternal;

		static void InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes);
		static float InternalEvaluate(ScriptAnimationCurve* self, float time, bool loop);
		static MonoArray* InternalGetKeyFrames(ScriptAnimationCurve* self);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector3Curve : public ScriptObject<ScriptVector3Curve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector3Curve")

		ScriptVector3Curve(MonoObject* managedInstance, const SPtr<TAnimationCurve<TVector3<float>>>& value);

		SPtr<TAnimationCurve<TVector3<float>>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TAnimationCurve<TVector3<float>>>& value);

	private:
		SPtr<TAnimationCurve<TVector3<float>>> mInternal;

		static void InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes);
		static void InternalEvaluate(ScriptVector3Curve* self, float time, bool loop, TVector3<float>* __output);
		static MonoArray* InternalGetKeyFrames(ScriptVector3Curve* self);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2Curve : public ScriptObject<ScriptVector2Curve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Vector2Curve")

		ScriptVector2Curve(MonoObject* managedInstance, const SPtr<TAnimationCurve<TVector2<float>>>& value);

		SPtr<TAnimationCurve<TVector2<float>>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TAnimationCurve<TVector2<float>>>& value);

	private:
		SPtr<TAnimationCurve<TVector2<float>>> mInternal;

		static void InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes);
		static void InternalEvaluate(ScriptVector2Curve* self, float time, bool loop, TVector2<float>* __output);
		static MonoArray* InternalGetKeyFrames(ScriptVector2Curve* self);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptQuaternionCurve : public ScriptObject<ScriptQuaternionCurve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "QuaternionCurve")

		ScriptQuaternionCurve(MonoObject* managedInstance, const SPtr<TAnimationCurve<Quaternion>>& value);

		SPtr<TAnimationCurve<Quaternion>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TAnimationCurve<Quaternion>>& value);

	private:
		SPtr<TAnimationCurve<Quaternion>> mInternal;

		static void InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes);
		static void InternalEvaluate(ScriptQuaternionCurve* self, float time, bool loop, Quaternion* __output);
		static MonoArray* InternalGetKeyFrames(ScriptQuaternionCurve* self);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptIntegerCurve : public ScriptObject<ScriptIntegerCurve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "IntegerCurve")

		ScriptIntegerCurve(MonoObject* managedInstance, const SPtr<TAnimationCurve<int32_t>>& value);

		SPtr<TAnimationCurve<int32_t>> GetInternal() const { return mInternal; }
		static MonoObject* Create(const SPtr<TAnimationCurve<int32_t>>& value);

	private:
		SPtr<TAnimationCurve<int32_t>> mInternal;

		static void InternalTAnimationCurve(MonoObject* managedInstance, MonoArray* keyframes);
		static int32_t InternalEvaluate(ScriptIntegerCurve* self, float time, bool loop);
		static MonoArray* InternalGetKeyFrames(ScriptIntegerCurve* self);
	};
}
