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
	struct __TNamedAnimationCurve_float_Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedFloatCurve : public ScriptObject<ScriptNamedFloatCurve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NamedFloatCurve")

		static MonoObject* Box(const __TNamedAnimationCurve_float_Interop& value);
		static __TNamedAnimationCurve_float_Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<float> FromInterop(const __TNamedAnimationCurve_float_Interop& value);
		static __TNamedAnimationCurve_float_Interop ToInterop(const TNamedAnimationCurve<float>& value);

	private:
		ScriptNamedFloatCurve(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurve_TVector3_float__Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedVector3Curve : public ScriptObject<ScriptNamedVector3Curve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NamedVector3Curve")

		static MonoObject* Box(const __TNamedAnimationCurve_TVector3_float__Interop& value);
		static __TNamedAnimationCurve_TVector3_float__Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<TVector3<float>> FromInterop(const __TNamedAnimationCurve_TVector3_float__Interop& value);
		static __TNamedAnimationCurve_TVector3_float__Interop ToInterop(const TNamedAnimationCurve<TVector3<float>>& value);

	private:
		ScriptNamedVector3Curve(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurve_TVector2_float__Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedVector2Curve : public ScriptObject<ScriptNamedVector2Curve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NamedVector2Curve")

		static MonoObject* Box(const __TNamedAnimationCurve_TVector2_float__Interop& value);
		static __TNamedAnimationCurve_TVector2_float__Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<TVector2<float>> FromInterop(const __TNamedAnimationCurve_TVector2_float__Interop& value);
		static __TNamedAnimationCurve_TVector2_float__Interop ToInterop(const TNamedAnimationCurve<TVector2<float>>& value);

	private:
		ScriptNamedVector2Curve(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurve_Quaternion_Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedQuaternionCurve : public ScriptObject<ScriptNamedQuaternionCurve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NamedQuaternionCurve")

		static MonoObject* Box(const __TNamedAnimationCurve_Quaternion_Interop& value);
		static __TNamedAnimationCurve_Quaternion_Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<Quaternion> FromInterop(const __TNamedAnimationCurve_Quaternion_Interop& value);
		static __TNamedAnimationCurve_Quaternion_Interop ToInterop(const TNamedAnimationCurve<Quaternion>& value);

	private:
		ScriptNamedQuaternionCurve(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurve_int32_t_Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedIntegerCurve : public ScriptObject<ScriptNamedIntegerCurve>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "NamedIntegerCurve")

		static MonoObject* Box(const __TNamedAnimationCurve_int32_t_Interop& value);
		static __TNamedAnimationCurve_int32_t_Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<int32_t> FromInterop(const __TNamedAnimationCurve_int32_t_Interop& value);
		static __TNamedAnimationCurve_int32_t_Interop ToInterop(const TNamedAnimationCurve<int32_t>& value);

	private:
		ScriptNamedIntegerCurve(MonoObject* managedInstance);

	};
}
