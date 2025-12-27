//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Animation/B3DAnimationCurve.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** @addtogroup Animation
	 *  @{
	 */

	/** Contains information and helper methods for various curve types. */
	template <class T>
	struct TCurveProperties
	{};

	template <>
	struct TCurveProperties<float>
	{
		enum
		{
			NumComponents = 1
		};

		static float GetZero() { return 0.0f; }

		static float GetComponent(float val, u32 i) { return val; }

		static void SetComponent(float& val, u32 i, float newVal) { val = newVal; }
	};

	template <>
	struct TCurveProperties<i32>
	{
		enum
		{
			NumComponents = 1
		};

		static i32 GetZero() { return 0; }
	};

	template <>
	struct TCurveProperties<Vector2>
	{
		enum
		{
			NumComponents = 2
		};

		static Vector2 GetZero() { return Vector2::kZero; }

		static float GetComponent(const Vector2& val, u32 i) { return val[i]; }

		static void SetComponent(Vector2& val, u32 i, float newVal) { val[i] = newVal; }
	};

	template <>
	struct TCurveProperties<Vector3>
	{
		enum
		{
			NumComponents = 3
		};

		static Vector3 GetZero() { return Vector3::kZero; }

		static float GetComponent(const Vector3& val, u32 i) { return val[i]; }

		static void SetComponent(Vector3& val, u32 i, float newVal) { val[i] = newVal; }
	};

	template <>
	struct TCurveProperties<Quaternion>
	{
		enum
		{
			NumComponents = 4
		};

		static Quaternion GetZero() { return Quaternion::kZero; }

		static float GetComponent(const Quaternion& val, u32 i) { return val[i]; }

		static void SetComponent(Quaternion& val, u32 i, float newVal) { val[i] = newVal; }
	};

	/** Helper class for dealing with animations, animation clips and curves. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) AnimationUtility : public IScriptExportable
	{
	public:
		/**
		 * Wraps or clamps the provided time value between the provided range.
		 *
		 * @param	time	Time value to wrap/clamp.
		 * @param	start	Start of the range.
		 * @param	end		End of the range.
		 * @param	loop	If true the value will be wrapped, otherwise clamped to range.
		 */
		static void WrapTime(float& time, float start, float end, bool loop);

		/** Converts a curve in euler angles (in degrees) into a curve using quaternions. */
		B3D_SCRIPT_EXPORT()
		static SPtr<TAnimationCurve<Quaternion>> EulerToQuaternionCurve(const SPtr<TAnimationCurve<Vector3>>& eulerCurve, EulerAngleOrder order = EulerAngleOrder::YXZ);

		/** Converts a curve in quaternions into a curve using euler angles (in degrees). */
		B3D_SCRIPT_EXPORT()
		static SPtr<TAnimationCurve<Vector3>> QuaternionToEulerCurve(const SPtr<TAnimationCurve<Quaternion>>& quatCurve);

		/** Splits a Vector3 curve into three individual curves, one for each component. */
		B3D_SCRIPT_EXPORT()
		static Vector<SPtr<TAnimationCurve<float>>> SplitCurve3D(const SPtr<TAnimationCurve<Vector3>>& compoundCurve);

		/** Combines three single component curves into a Vector3 curve. */
		B3D_SCRIPT_EXPORT()
		static SPtr<TAnimationCurve<Vector3>> CombineCurve3D(const Vector<SPtr<TAnimationCurve<float>>>& curveComponents);

		/** Splits a Vector2 curve into two individual curves, one for each component. */
		B3D_SCRIPT_EXPORT()
		static Vector<SPtr<TAnimationCurve<float>>> SplitCurve2D(const SPtr<TAnimationCurve<Vector2>>& compoundCurve);

		/** Combines two single component curves into a Vector2 curve. */
		B3D_SCRIPT_EXPORT()
		static SPtr<TAnimationCurve<Vector2>> CombineCurve2D(const Vector<SPtr<TAnimationCurve<float>>>& curveComponents);

		/** Splits a multi-component curve into multiple individual curves, one for each component. */
		template <class T>
		static void SplitCurve(const TAnimationCurve<T>& compoundCurve, TAnimationCurve<float> (&output)[TCurveProperties<T>::NumComponents]);

		/** Combines multiple single component curves into a multi-component curve. */
		template <class T>
		static void CombineCurve(const TAnimationCurve<float> (&curveComponents)[TCurveProperties<T>::NumComponents], TAnimationCurve<T>& output);
		/**
		 * Calculates the total range covered by a set of curves.
		 *
		 * @param	curves		Curves to calculate range for.
		 * @param	outXMin		Minimum time value present in the curves.
		 * @param	outXMax		Maximum time value present in the curves.
		 * @param	outYMin		Minimum curve value present in the curves.
		 * @param	outYMax		Maximum curve value present in the curves.
		 */
		static void CalculateRange(const Vector<TAnimationCurve<float>>& curves, float& outXMin, float& outXMax, float& outYMin, float& outYMax);

		/** @copydoc CalculateRange(const Vector<TAnimationCurve<float>>&, float&, float&, float&, float&) */
		B3D_SCRIPT_EXPORT()
		static void CalculateRange(const Vector<SPtr<TAnimationCurve<float>>>& curves, float& outXMin, float& outXMax, float& outYMin, float& outYMax);

		/** Scales all curve values and tangents by the specified scale factor. */
		template <class T>
		static TAnimationCurve<T> ScaleCurve(const TAnimationCurve<T>& curve, float factor);

		/** Adds a time offset to all keyframes in the provided curve. */
		template <class T>
		static TAnimationCurve<T> OffsetCurve(const TAnimationCurve<T>& curve, float offset);

		/** Updates the provided list of keyframes by automatically calculating their tangents. */
		template <class T>
		static void CalculateTangents(Vector<TKeyframe<T>>& keyframes);
	};

	/** Type of tangent on a keyframe in an animation curve. */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation)) TangentType
	{
		In = 1 << 0,
		Out = 1 << 1
	};

	/**
	 * Flags that are used for describing how are tangents calculated for a specific keyframe in an animation curve.
	 * Modes for "in" and "out" tangents can be combined.
	 */
	enum class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(TangentMode)) TangentModeBits
	{
		/** Both tangents are calculated automatically based on the two surrounding keyframes. */
		Auto = 0,
		/** Left tangent is calculated automatically based on the two surrounding keyframes. */
		InAuto = (int)TangentType::In | 1 << 2,
		/** Left tangent is manually adjusted by the user. */
		InFree = (int)TangentType::In | 1 << 3,
		/** Tangent is calculated automatically based on the previous keyframe. */
		InLinear = (int)TangentType::In | 1 << 4,
		/** Tangent is infinite, ensuring there is a instantaneus jump between previous and current keyframe value. */
		InStep = (int)TangentType::In | 1 << 5,
		/** Right tangents are calculated automatically based on the two surrounding keyframes. */
		OutAuto = (int)TangentType::Out | 1 << 6,
		/** Right tangent is manually adjusted by the user. */
		OutFree = (int)TangentType::Out | 1 << 7,
		/** Tangent is calculated automatically based on the next keyframe. */
		OutLinear = (int)TangentType::Out | 1 << 8,
		/** Tangent is infinite, ensuring there is a instantaneus jump between current and next keyframe value. */
		OutStep = (int)TangentType::Out | 1 << 9,
		/** Both tangents are manually adjusted by the user. */
		Free = 1 << 10,
	};

	typedef Flags<TangentModeBits> TangentMode;
	B3D_FLAGS_OPERATORS(TangentModeBits)

	/* Structure containing a reference to a keyframe as a curve index, and a keyframe index within that curve. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Animation)) KeyframeRef
	{
		KeyframeRef() = default;

		KeyframeRef(i32 curveIdx, i32 keyIdx)
			: CurveIdx(curveIdx), KeyIdx(keyIdx)
		{}

		i32 CurveIdx = 0;
		i32 KeyIdx = 0;
	};

	/** Structure containing a reference to a keyframe tangent, as a keyframe reference and type of the tangent. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(Animation)) TangentRef
	{
		TangentRef() = default;

		TangentRef(KeyframeRef keyframeRef, TangentType type)
			: KeyframeRef(keyframeRef), Type(type)
		{}

		KeyframeRef KeyframeRef;
		TangentType Type = TangentType::In;
	};

	/** @} */
} // namespace b3d
