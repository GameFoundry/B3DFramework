//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Animation/BsAnimationCurve.h"

namespace bs
{
	/** @addtogroup Animation
	 *  @{
	 */

	/** Helper class for dealing with animations, animation clips and curves. */
	class BS_CORE_EXPORT BS_SCRIPT_EXPORT(m:Animation) AnimationUtility
	{
	public:
		/**
		 * Wraps or clamps the provided time value between the provided range.
		 *
		 * @param[in,out]	time	Time value to wrap/clamp.
		 * @param[in]		start	Start of the range.
		 * @param[in]		end		End of the range.
		 * @param[in]		loop	If true the value will be wrapped, otherwise clamped to range.
		 */
		static void wrapTime(float& time, float start, float end, bool loop);

		/** Converts a curve in euler angles (in degrees) into a curve using quaternions. */
		BS_SCRIPT_EXPORT()
		static SPtr<TAnimationCurve<Quaternion>> eulerToQuaternionCurve(const SPtr<TAnimationCurve<Vector3>>& eulerCurve);

		/** Converts a curve in quaternions into a curve using euler angles (in degrees). */
		BS_SCRIPT_EXPORT()
		static SPtr<TAnimationCurve<Vector3>> quaternionToEulerCurve(const SPtr<TAnimationCurve<Quaternion>>& quatCurve);

		/** Splits a Vector3 curve into three individual curves, one for each component. */
		BS_SCRIPT_EXPORT()
		static Vector<SPtr<TAnimationCurve<float>>> splitCurve(const SPtr<TAnimationCurve<Vector3>>& compoundCurve);

		/** Combines three single component curves into a Vector3 curve. */
		BS_SCRIPT_EXPORT()
		static SPtr<TAnimationCurve<Vector3>> combineCurve(const Vector<SPtr<TAnimationCurve<float>>>& curveComponents);

		/**
		 * Calculates the total range covered by a set of curves.
		 *
		 * @param[in]	curves		Curves to calculate range for.
		 * @param[out]	xMin		Minimum time value present in the curves.
		 * @param[out]	xMax		Maximum time value present in the curves.
		 * @param[out]	yMin		Minimum curve value present in the curves.
		 * @param[out]	yMax		Maximum curve value present in the curves.
		 */
		static void calculateRange(const Vector<TAnimationCurve<float>>& curves, 
			float& xMin, float& xMax, float& yMin, float& yMax);

		/** @copydoc calculateRange(const Vector<TAnimationCurve<float>>&, float&, float&, float&, float&) */
		BS_SCRIPT_EXPORT()
		static void calculateRange(const Vector<SPtr<TAnimationCurve<float>>>& curves, 
			float& xMin, float& xMax, float& yMin, float& yMax);

		/** Scales all curve values and tangents by the specified scale factor. */
		template<class T> 
		static TAnimationCurve<T> scaleCurve(const TAnimationCurve<T>& curve, float factor);

		/** Adds a time offset to all keyframes in the provided curve. */
		template<class T>
		static TAnimationCurve<T> offsetCurve(const TAnimationCurve<T>& curve, float offset);
	};

	/** Type of tangent on a keyframe in an animation curve. */
	enum class BS_SCRIPT_EXPORT(m:Animation) TangentType
	{
		In = 1 << 0,
		Out = 1 << 1
	};

	/**
	 * Flags that are used for describing how are tangents calculated for a specific keyframe in an animation curve. 
	 * Modes for "in" and "out" tangents can be combined.
	 */
	enum class BS_SCRIPT_EXPORT(m:Animation,n:TangentMode) TangentModeBits
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
	BS_FLAGS_OPERATORS(TangentModeBits)

	/* Structure containing a reference to a keyframe as a curve index, and a keyframe index within that curve. */
	struct BS_SCRIPT_EXPORT(pl:true,m:Animation) KeyframeRef
	{
		KeyframeRef() = default;
		KeyframeRef(INT32 curveIdx, INT32 keyIdx)
			:curveIdx(curveIdx), keyIdx(keyIdx)
		{ }

		INT32 curveIdx = 0;
		INT32 keyIdx = 0;
	};

	/** Structure containing a reference to a keyframe tangent, as a keyframe reference and type of the tangent. */
	struct BS_SCRIPT_EXPORT(pl:true,m:Animation) TangentRef
	{
		TangentRef() = default;
		TangentRef(KeyframeRef keyframeRef, TangentType type)
			:keyframeRef(keyframeRef), type(type)
		{ }

		KeyframeRef keyframeRef;
		TangentType type = TangentType::In;
	};

	/** @} */
}