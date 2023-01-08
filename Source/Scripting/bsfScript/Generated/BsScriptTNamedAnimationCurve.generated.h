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

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedFloatCurve : public ScriptObject<ScriptNamedFloatCurve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NamedFloatCurve")

		static MonoObject* Box(const __TNamedAnimationCurvefloatInterop& value);
		static __TNamedAnimationCurvefloatInterop Unbox(MonoObject* value);
		static TNamedAnimationCurve<float> FromInterop(const __TNamedAnimationCurvefloatInterop& value);
		static __TNamedAnimationCurvefloatInterop ToInterop(const TNamedAnimationCurve<float>& value);

	private:
		ScriptNamedFloatCurve(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurveVector3Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedVector3Curve : public ScriptObject<ScriptNamedVector3Curve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NamedVector3Curve")

		static MonoObject* Box(const __TNamedAnimationCurveVector3Interop& value);
		static __TNamedAnimationCurveVector3Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<Vector3> FromInterop(const __TNamedAnimationCurveVector3Interop& value);
		static __TNamedAnimationCurveVector3Interop ToInterop(const TNamedAnimationCurve<Vector3>& value);

	private:
		ScriptNamedVector3Curve(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurveVector2Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedVector2Curve : public ScriptObject<ScriptNamedVector2Curve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NamedVector2Curve")

		static MonoObject* Box(const __TNamedAnimationCurveVector2Interop& value);
		static __TNamedAnimationCurveVector2Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<Vector2> FromInterop(const __TNamedAnimationCurveVector2Interop& value);
		static __TNamedAnimationCurveVector2Interop ToInterop(const TNamedAnimationCurve<Vector2>& value);

	private:
		ScriptNamedVector2Curve(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurveQuaternionInterop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedQuaternionCurve : public ScriptObject<ScriptNamedQuaternionCurve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NamedQuaternionCurve")

		static MonoObject* Box(const __TNamedAnimationCurveQuaternionInterop& value);
		static __TNamedAnimationCurveQuaternionInterop Unbox(MonoObject* value);
		static TNamedAnimationCurve<Quaternion> FromInterop(const __TNamedAnimationCurveQuaternionInterop& value);
		static __TNamedAnimationCurveQuaternionInterop ToInterop(const TNamedAnimationCurve<Quaternion>& value);

	private:
		ScriptNamedQuaternionCurve(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurveint32_tInterop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedIntegerCurve : public ScriptObject<ScriptNamedIntegerCurve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NamedIntegerCurve")

		static MonoObject* Box(const __TNamedAnimationCurveint32_tInterop& value);
		static __TNamedAnimationCurveint32_tInterop Unbox(MonoObject* value);
		static TNamedAnimationCurve<int32_t> FromInterop(const __TNamedAnimationCurveint32_tInterop& value);
		static __TNamedAnimationCurveint32_tInterop ToInterop(const TNamedAnimationCurve<int32_t>& value);

	private:
		ScriptNamedIntegerCurve(MonoObject* managedInstance);

	};
}
