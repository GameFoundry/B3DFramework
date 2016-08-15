//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsAnimationCurve.h"

namespace BansheeEngine
{
	/** @addtogroup Animation
	 *  @{
	 */

	/** Helper class for dealing with animations, animation clips and curves. */
	class BS_CORE_EXPORT AnimationUtility
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
		static TAnimationCurve<Quaternion> eulerToQuaternionCurve(const TAnimationCurve<Vector3>& eulerCurve);

		/** Converts a curve in quaternions into a curve using euler angles (in degrees). */
		static TAnimationCurve<Vector3> quaternionToEulerCurve(const TAnimationCurve<Quaternion>& quatCurve);
	};

	/** @} */
}