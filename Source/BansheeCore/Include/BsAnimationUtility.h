//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsAnimationCurve.h"

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
		BS_SCRIPT_EXPORT(n:EulerToQuaternionCurve)
		static SPtr<TAnimationCurve<Quaternion>> eulerToQuaternionCurve(const SPtr<TAnimationCurve<Vector3>>& eulerCurve);

		/** Converts a curve in quaternions into a curve using euler angles (in degrees). */
		BS_SCRIPT_EXPORT(n:QuaternionToEulerCurve)
		static SPtr<TAnimationCurve<Vector3>> quaternionToEulerCurve(const SPtr<TAnimationCurve<Quaternion>>& quatCurve);

		/** Splits a Vector3 curve into three individual curves, one for each component. */
		BS_SCRIPT_EXPORT(n:SplitCurve)
		static Vector<SPtr<TAnimationCurve<float>>> splitCurve(const SPtr<TAnimationCurve<Vector3>>& compoundCurve);

		/** Combines three single component curves into a Vector3 curve. */
		BS_SCRIPT_EXPORT(n:CombineCurve)
		static SPtr<TAnimationCurve<Vector3>> combineCurve(const Vector<SPtr<TAnimationCurve<float>>>& curveComponents);

		/** Scales all curve values and tangents by the specified scale factor. */
		template<class T> 
		static TAnimationCurve<T> scaleCurve(const TAnimationCurve<T>& curve, float factor);

		/** Adds a time offset to all keyframes in the provided curve. */
		template<class T>
		static TAnimationCurve<T> offsetCurve(const TAnimationCurve<T>& curve, float offset);
	};

	/** @} */
}