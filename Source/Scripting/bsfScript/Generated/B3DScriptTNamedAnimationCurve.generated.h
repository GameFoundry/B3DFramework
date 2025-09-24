//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "BsScriptObjectWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimationCurve.h"

namespace b3d
{
	struct __TNamedAnimationCurve_float_Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedFloatCurve : public TScriptTypeDefinition<ScriptNamedFloatCurve>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "NamedFloatCurve")

		static MonoObject* Box(const __TNamedAnimationCurve_float_Interop& value);
		static __TNamedAnimationCurve_float_Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<float> FromInterop(const __TNamedAnimationCurve_float_Interop& value);
		static __TNamedAnimationCurve_float_Interop ToInterop(const TNamedAnimationCurve<float>& value);

	private:
		ScriptNamedFloatCurve();

	};

	struct __TNamedAnimationCurve_TVector3_float__Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedVector3Curve : public TScriptTypeDefinition<ScriptNamedVector3Curve>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "NamedVector3Curve")

		static MonoObject* Box(const __TNamedAnimationCurve_TVector3_float__Interop& value);
		static __TNamedAnimationCurve_TVector3_float__Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<TVector3<float>> FromInterop(const __TNamedAnimationCurve_TVector3_float__Interop& value);
		static __TNamedAnimationCurve_TVector3_float__Interop ToInterop(const TNamedAnimationCurve<TVector3<float>>& value);

	private:
		ScriptNamedVector3Curve();

	};

	struct __TNamedAnimationCurve_TVector2_float__Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedVector2Curve : public TScriptTypeDefinition<ScriptNamedVector2Curve>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "NamedVector2Curve")

		static MonoObject* Box(const __TNamedAnimationCurve_TVector2_float__Interop& value);
		static __TNamedAnimationCurve_TVector2_float__Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<TVector2<float>> FromInterop(const __TNamedAnimationCurve_TVector2_float__Interop& value);
		static __TNamedAnimationCurve_TVector2_float__Interop ToInterop(const TNamedAnimationCurve<TVector2<float>>& value);

	private:
		ScriptNamedVector2Curve();

	};

	struct __TNamedAnimationCurve_TQuaternion_float__Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedQuaternionCurve : public TScriptTypeDefinition<ScriptNamedQuaternionCurve>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "NamedQuaternionCurve")

		static MonoObject* Box(const __TNamedAnimationCurve_TQuaternion_float__Interop& value);
		static __TNamedAnimationCurve_TQuaternion_float__Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<TQuaternion<float>> FromInterop(const __TNamedAnimationCurve_TQuaternion_float__Interop& value);
		static __TNamedAnimationCurve_TQuaternion_float__Interop ToInterop(const TNamedAnimationCurve<TQuaternion<float>>& value);

	private:
		ScriptNamedQuaternionCurve();

	};

	struct __TNamedAnimationCurve_int32_t_Interop
	{
		MonoString* Name;
		Flags<AnimationCurveFlag> Flags;
		MonoObject* Curve;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptNamedIntegerCurve : public TScriptTypeDefinition<ScriptNamedIntegerCurve>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "NamedIntegerCurve")

		static MonoObject* Box(const __TNamedAnimationCurve_int32_t_Interop& value);
		static __TNamedAnimationCurve_int32_t_Interop Unbox(MonoObject* value);
		static TNamedAnimationCurve<int32_t> FromInterop(const __TNamedAnimationCurve_int32_t_Interop& value);
		static __TNamedAnimationCurve_int32_t_Interop ToInterop(const TNamedAnimationCurve<int32_t>& value);

	private:
		ScriptNamedIntegerCurve();

	};
}
