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
		MonoString* name;
		Flags<AnimationCurveFlag> flags;
		MonoObject* curve;
	};

	class BS_SCR_BE_EXPORT ScriptTNamedAnimationCurvefloat : public ScriptObject<ScriptTNamedAnimationCurvefloat>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NamedFloatCurve")

		static MonoObject* box(const __TNamedAnimationCurvefloatInterop& value);
		static __TNamedAnimationCurvefloatInterop unbox(MonoObject* value);
		static TNamedAnimationCurve<float> fromInterop(const __TNamedAnimationCurvefloatInterop& value);
		static __TNamedAnimationCurvefloatInterop toInterop(const TNamedAnimationCurve<float>& value);

	private:
		ScriptTNamedAnimationCurvefloat(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurveVector3Interop
	{
		MonoString* name;
		Flags<AnimationCurveFlag> flags;
		MonoObject* curve;
	};

	class BS_SCR_BE_EXPORT ScriptTNamedAnimationCurveVector3 : public ScriptObject<ScriptTNamedAnimationCurveVector3>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NamedVector3Curve")

		static MonoObject* box(const __TNamedAnimationCurveVector3Interop& value);
		static __TNamedAnimationCurveVector3Interop unbox(MonoObject* value);
		static TNamedAnimationCurve<Vector3> fromInterop(const __TNamedAnimationCurveVector3Interop& value);
		static __TNamedAnimationCurveVector3Interop toInterop(const TNamedAnimationCurve<Vector3>& value);

	private:
		ScriptTNamedAnimationCurveVector3(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurveVector2Interop
	{
		MonoString* name;
		Flags<AnimationCurveFlag> flags;
		MonoObject* curve;
	};

	class BS_SCR_BE_EXPORT ScriptTNamedAnimationCurveVector2 : public ScriptObject<ScriptTNamedAnimationCurveVector2>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NamedVector2Curve")

		static MonoObject* box(const __TNamedAnimationCurveVector2Interop& value);
		static __TNamedAnimationCurveVector2Interop unbox(MonoObject* value);
		static TNamedAnimationCurve<Vector2> fromInterop(const __TNamedAnimationCurveVector2Interop& value);
		static __TNamedAnimationCurveVector2Interop toInterop(const TNamedAnimationCurve<Vector2>& value);

	private:
		ScriptTNamedAnimationCurveVector2(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurveQuaternionInterop
	{
		MonoString* name;
		Flags<AnimationCurveFlag> flags;
		MonoObject* curve;
	};

	class BS_SCR_BE_EXPORT ScriptTNamedAnimationCurveQuaternion : public ScriptObject<ScriptTNamedAnimationCurveQuaternion>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NamedQuaternionCurve")

		static MonoObject* box(const __TNamedAnimationCurveQuaternionInterop& value);
		static __TNamedAnimationCurveQuaternionInterop unbox(MonoObject* value);
		static TNamedAnimationCurve<Quaternion> fromInterop(const __TNamedAnimationCurveQuaternionInterop& value);
		static __TNamedAnimationCurveQuaternionInterop toInterop(const TNamedAnimationCurve<Quaternion>& value);

	private:
		ScriptTNamedAnimationCurveQuaternion(MonoObject* managedInstance);

	};

	struct __TNamedAnimationCurveint32_tInterop
	{
		MonoString* name;
		Flags<AnimationCurveFlag> flags;
		MonoObject* curve;
	};

	class BS_SCR_BE_EXPORT ScriptTNamedAnimationCurveint32_t : public ScriptObject<ScriptTNamedAnimationCurveint32_t>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "NamedIntegerCurve")

		static MonoObject* box(const __TNamedAnimationCurveint32_tInterop& value);
		static __TNamedAnimationCurveint32_tInterop unbox(MonoObject* value);
		static TNamedAnimationCurve<int32_t> fromInterop(const __TNamedAnimationCurveint32_tInterop& value);
		static __TNamedAnimationCurveint32_tInterop toInterop(const TNamedAnimationCurve<int32_t>& value);

	private:
		ScriptTNamedAnimationCurveint32_t(MonoObject* managedInstance);

	};
}
