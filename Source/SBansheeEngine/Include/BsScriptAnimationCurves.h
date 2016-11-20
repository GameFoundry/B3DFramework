//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsAnimationCurve.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for AnimationCurves. */
	class BS_SCR_BE_EXPORT ScriptAnimationCurves : public ScriptObject<ScriptAnimationCurves>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "AnimationCurves")

		/** Converts managed animation curves its native counterpart. */
		static SPtr<AnimationCurves> toNative(MonoObject* object);

		/** Converts native animation curves to its managed counterpart. */
		static MonoObject* toManaged(const SPtr<AnimationCurves>& curves);
	private:
		ScriptAnimationCurves(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* sPositionCurvesField;
		static MonoField* sRotationCurvesField;
		static MonoField* sScaleCurvesField;
		static MonoField* sFloatCurvesField;
	};

	/**	Interop class between C++ & CLR for NamedVector3Curve. */
	class BS_SCR_BE_EXPORT ScriptNamedVector3Curve : public ScriptObject<ScriptNamedVector3Curve>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NamedVector3Curve")

		/** Converts managed 3D vector animation curve its native counterpart. */
		static TNamedAnimationCurve<Vector3> toNative(MonoObject* object);

		/** Converts native 3D vector animation curve to its managed counterpart. */
		static MonoObject* toManaged(const TNamedAnimationCurve<Vector3>& curve);
	private:
		ScriptNamedVector3Curve(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* sNameField;
		static MonoField* sFlagsField;
		static MonoField* sXCurveField;
		static MonoField* sYCurveField;
		static MonoField* sZCurveField;
	};

	/**	Interop class between C++ & CLR for NamedFloatCurve. */
	class BS_SCR_BE_EXPORT ScriptNamedFloatCurve : public ScriptObject<ScriptNamedFloatCurve>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NamedFloatCurve")

		/** Converts managed float animation curve its native counterpart. */
		static TNamedAnimationCurve<float> toNative(MonoObject* object);

		/** Converts native float animation curve to its managed counterpart. */
		static MonoObject* toManaged(const TNamedAnimationCurve<float>& curve);
	private:
		ScriptNamedFloatCurve(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* sNameField;
		static MonoField* sFlagsField;
		static MonoField* sCurveField;
	};

	/**	Interop class between C++ & CLR for Vector3Curve. */
	class BS_SCR_BE_EXPORT ScriptVector3Curve : public ScriptObject<ScriptVector3Curve>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Vector3Curve")

		/** Converts native 3D vector animation curve to its managed counterpart. */
		static MonoObject* toManaged(const TAnimationCurve<Vector3>& curve);
	private:
		ScriptVector3Curve(MonoObject* instance);
	};

	/**	Interop class between C++ & CLR for QuaternionCurve. */
	class BS_SCR_BE_EXPORT ScriptQuaternionCurve : public ScriptObject<ScriptQuaternionCurve>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "QuaternionCurve")

		/** Converts native quaternion animation curve to its managed counterpart. */
		static MonoObject* toManaged(const TAnimationCurve<Quaternion>& curve);
	private:
		ScriptQuaternionCurve(MonoObject* instance);
	};

	struct RootMotion;

	/**	Interop class between C++ & CLR for RootMotion. */
	class BS_SCR_BE_EXPORT ScriptRootMotion : public ScriptObject<ScriptRootMotion>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "RootMotion")

		/** Converts native root motion object to its managed counterpart. */
		static MonoObject* toManaged(const SPtr<RootMotion>& rootMotion);
	private:
		ScriptRootMotion(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* sPositionField;
		static MonoField* sRotationField;
	};

	/** @} */
}