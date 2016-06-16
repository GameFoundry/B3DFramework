//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsAnimationInstance.h"

namespace BansheeEngine
{
	/** @addtogroup Animation-Internal
	 *  @{
	 */

	/** Animation keyframe, represented as an endpoint of a cubic hermite spline. */
	template <class T>
	struct TKeyframe
	{
		T value; /**< Value of the key. */
		T inTangent; /**< Input tangent (going from the previous key to this one) of the key. */
		T outTangent; /**< Output tangent (going from this key to next one) of the key. */
		float time; /**< Position of the key along the animation spline. */
	};

	/**
	 * Animation spline represented by a set of keyframes, each representing an endpoint of a cubic hermite curve. The
	 * spline can be evaluated at any time, and uses caching to speed up multiple sequential evaluations.
	 */
	template <class T>
	class BS_CORE_EXPORT TAnimationCurve
	{
	public:
		typedef TKeyframe<T> KeyFrame;

		TAnimationCurve(const Vector<KeyFrame>& keyframes);

		/**
		 * Evaluate the animation curve using caching. Caching can significantly speed of evaluation if the evaluation
		 * happens sequential order (which should be true for most curves). If evaluation is not happening in sequential
		 * order using the non-caching version of evaluate() might yield better performance.
		 *
		 * @param[i]	animInstance	Animation instance data holding the time to evaluate the curve at, and any cached
		 *								data from previous requests. Caller should ensure to maintain a persistent instance
		 *								of this data for every animation using this curve in order to ensure cache is
		 *								maintained.
		 * @param[in]	loop			If true the curve will loop when it goes past the end or beggining. Otherwise the
		 *								curve value will be clamped.
		 @return						Interpolated value from the curve at provided time.
		 */
		T evaluate(const AnimationInstanceData<T>& animInstance, bool loop = true);

		/**
		 * Evaluate the animation curve at the specified time. If evaluating multiple values in a sequential order consider
		 * using the cached version of evaluate() for better performance.
		 *
		 * @param[i]	time	Time to evaluate the curve at.		
		 * @param[in]	loop	If true the curve will loop when it goes past the end or beggining. Otherwise the curve 
		 *						value will be clamped.
		 @return				Interpolated value from the curve at provided time.
		 */
		T evaluate(float time, bool loop = true);

	private:
		/** 
		 * Returns a pair of keys that can be used for interpolating to field the value at the provided time. This attempts
		 * to find keys using the cache first, and if not possible falls back to a full search.
		 *
		 * @param[in]	time			Time for which to find the relevant keys from. It is expected to be clamped to a
		 *								valid range within the curve.
		 * @param[in]	animInstance	Animation instance data holding the time to evaluate the curve at, and any cached
		 *								data from previous requests. Time is expected to be clamped to a valid range
		 *								within the curve.
		 * @param[out]	leftKey			Index of the key to interpolate from.
		 * @param[out]	rightKey		Index of the key to interpolate to.
		 */
		void findKeys(float time, const AnimationInstanceData<T>& animInstance, UINT32& leftKey, UINT32& rightKey);

		/** 
		 * Returns a pair of keys that can be used for interpolating to field the value at the provided time. 
		 *
		 * @param[in]	time			Time for which to find the relevant keys from. It is expected to be clamped to a
		 *								valid range within the curve.
		 * @param[out]	leftKey			Index of the key to interpolate from.
		 * @param[out]	rightKey		Index of the key to interpolate to.
		 */
		void findKeys(float time, UINT32& leftKey, UINT32& rightKey);

		/** 
		 * Evaluates a value at the cached curve. Caller must ensure the request time falls within the cached curve range.
		 *
		 * @param[in]	animInstance	Animation instance data holding the time to evaluate the curve at, and any cached
		 *								data from previous requests.
		 * @return						Interpolated value from the curve at provided time.
		 */
		T evaluateCache(const AnimationInstanceData<T>& animInstance);

		static const UINT32 CACHE_LOOKAHEAD;

		Vector<KeyFrame> mKeyframes;
		float mStart;
		float mEnd;
		float mLength;
	};

	/** @} */
}