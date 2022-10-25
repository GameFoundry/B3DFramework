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
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"

namespace bs
{
	struct __TNamedAnimationCurvefloatInterop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class BS_SCR_BE_EXPORT ScriptTNamedAnimationCurvefloat : public ScriptObject<ScriptTNamedAnimationCurvefloat>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NamedFloatCurve")

		static MonoObject* Box(const __TNamedAnimationCurvefloatInterop& value);
		static __TNamedAnimationCurvefloatInterop Unbox(MonoObject* value);
		static TNamedAnimationCurve<float> FromInterop(const __TNamedAnimationCurvefloatInterop& value);
		static __TNamedAnimationCurvefloatInterop ToInterop(const TNamedAnimationCurve<float>& value);

	private:
		ScriptTNamedAnimationCurvefloat(MonoObject* managedInstance);
	};

	struct __TNamedAnimationCurveVector3Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class BS_SCR_BE_EXPORT ScriptTNamedAnimationCurveVector3 : public ScriptObject<ScriptTNamedAnimationCurveVector3>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NamedVector3Curve")

		static MonoObject* Box(const __TNamedAnimationCurveVector3Interop& value);
		static __TNamedAnimationCurveVector3Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<Vector3> FromInterop(const __TNamedAnimationCurveVector3Interop& value);
		static __TNamedAnimationCurveVector3Interop ToInterop(const TNamedAnimationCurve<Vector3>& value);

	private:
		ScriptTNamedAnimationCurveVector3(MonoObject* managedInstance);
	};

	struct __TNamedAnimationCurveVector2Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class BS_SCR_BE_EXPORT ScriptTNamedAnimationCurveVector2 : public ScriptObject<ScriptTNamedAnimationCurveVector2>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NamedVector2Curve")

		static MonoObject* Box(const __TNamedAnimationCurveVector2Interop& value);
		static __TNamedAnimationCurveVector2Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<Vector2> FromInterop(const __TNamedAnimationCurveVector2Interop& value);
		static __TNamedAnimationCurveVector2Interop ToInterop(const TNamedAnimationCurve<Vector2>& value);

	private:
		ScriptTNamedAnimationCurveVector2(MonoObject* managedInstance);
	};

	struct __TNamedAnimationCurveQuaternionInterop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class BS_SCR_BE_EXPORT ScriptTNamedAnimationCurveQuaternion : public ScriptObject<ScriptTNamedAnimationCurveQuaternion>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NamedQuaternionCurve")

		static MonoObject* Box(const __TNamedAnimationCurveQuaternionInterop& value);
		static __TNamedAnimationCurveQuaternionInterop Unbox(MonoObject* value);
		static TNamedAnimationCurve<Quaternion> FromInterop(const __TNamedAnimationCurveQuaternionInterop& value);
		static __TNamedAnimationCurveQuaternionInterop ToInterop(const TNamedAnimationCurve<Quaternion>& value);

	private:
		ScriptTNamedAnimationCurveQuaternion(MonoObject* managedInstance);
	};

	struct __TNamedAnimationCurveint32_tInterop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class BS_SCR_BE_EXPORT ScriptTNamedAnimationCurveint32_t : public ScriptObject<ScriptTNamedAnimationCurveint32_t>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NamedIntegerCurve")

		static MonoObject* Box(const __TNamedAnimationCurveint32_tInterop& value);
		static __TNamedAnimationCurveint32_tInterop Unbox(MonoObject* value);
		static TNamedAnimationCurve<int32_t> FromInterop(const __TNamedAnimationCurveint32_tInterop& value);
		static __TNamedAnimationCurveint32_tInterop ToInterop(const TNamedAnimationCurve<int32_t>& value);

	private:
		ScriptTNamedAnimationCurveint32_t(MonoObject* managedInstance);
	};
} // namespace bs
