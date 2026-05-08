//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "B3DScriptNonReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Utility/Math/B3DQuaternion.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Utility/Math/B3DVector3.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"
#include "../../../Engine/Utility/Math/B3DVector2.h"
#include "../../../Engine/Core/Animation/B3DAnimationCurve.h"

namespace b3d { struct __TKeyframe_TVector3_float__Interop; }
namespace b3d { struct __TKeyframe_TVector2_float__Interop; }
namespace b3d { struct __TKeyframe_TQuaternion_float__Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimationCurve : public TScriptNonReflectableWrapper<TAnimationCurve<float>, ScriptAnimationCurve>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AnimationCurve")

		ScriptAnimationCurve(const TShared<TAnimationCurve<float>>& nativeObject);
		~ScriptAnimationCurve();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTAnimationCurve(MonoObject* scriptObject, MonoArray* keyframes);
		static float InternalEvaluate(ScriptAnimationCurve* self, float time, bool loop);
		static MonoArray* InternalGetKeyFrames(ScriptAnimationCurve* self);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector3Curve : public TScriptNonReflectableWrapper<TAnimationCurve<TVector3<float>>, ScriptVector3Curve>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector3Curve")

		ScriptVector3Curve(const TShared<TAnimationCurve<TVector3<float>>>& nativeObject);
		~ScriptVector3Curve();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTAnimationCurve(MonoObject* scriptObject, MonoArray* keyframes);
		static void InternalEvaluate(ScriptVector3Curve* self, float time, bool loop, TVector3<float>* __output);
		static MonoArray* InternalGetKeyFrames(ScriptVector3Curve* self);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptVector2Curve : public TScriptNonReflectableWrapper<TAnimationCurve<TVector2<float>>, ScriptVector2Curve>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Vector2Curve")

		ScriptVector2Curve(const TShared<TAnimationCurve<TVector2<float>>>& nativeObject);
		~ScriptVector2Curve();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTAnimationCurve(MonoObject* scriptObject, MonoArray* keyframes);
		static void InternalEvaluate(ScriptVector2Curve* self, float time, bool loop, TVector2<float>* __output);
		static MonoArray* InternalGetKeyFrames(ScriptVector2Curve* self);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptQuaternionCurve : public TScriptNonReflectableWrapper<TAnimationCurve<TQuaternion<float>>, ScriptQuaternionCurve>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "QuaternionCurve")

		ScriptQuaternionCurve(const TShared<TAnimationCurve<TQuaternion<float>>>& nativeObject);
		~ScriptQuaternionCurve();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTAnimationCurve(MonoObject* scriptObject, MonoArray* keyframes);
		static void InternalEvaluate(ScriptQuaternionCurve* self, float time, bool loop, TQuaternion<float>* __output);
		static MonoArray* InternalGetKeyFrames(ScriptQuaternionCurve* self);
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptIntegerCurve : public TScriptNonReflectableWrapper<TAnimationCurve<int32_t>, ScriptIntegerCurve>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "IntegerCurve")

		ScriptIntegerCurve(const TShared<TAnimationCurve<int32_t>>& nativeObject);
		~ScriptIntegerCurve();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalTAnimationCurve(MonoObject* scriptObject, MonoArray* keyframes);
		static int32_t InternalEvaluate(ScriptIntegerCurve* self, float time, bool loop);
		static MonoArray* InternalGetKeyFrames(ScriptIntegerCurve* self);
	};
}
