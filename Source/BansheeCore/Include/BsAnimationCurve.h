//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsCurveCache.h"

namespace bs
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
	class BS_CORE_EXPORT TAnimationCurve // Note: Curves are expected to be immutable for threading purposes
	{
	public:
		typedef TKeyframe<T> KeyFrame;

		TAnimationCurve();
		TAnimationCurve(const Vector<KeyFrame>& keyframes);

		/**
		 * Evaluate the animation curve using caching. Caching can significantly speed of evaluation if the evaluation
		 * happens sequential order (which should be true for most curves). If evaluation is not happening in sequential
		 * order using the non-caching version of evaluate() might yield better performance.
		 *
		 * @param[in]	time			%Time to evaluate the curve at.
		 * @param[in]	cache			Cached data from previous requests that can be used for speeding up sequential calls
		 *								to this method. Caller should ensure to maintain a persistent instance of this data
		 *								for every animation using this curve in order to ensure cache is maintained.
		 * @param[in]	loop			If true the curve will loop when it goes past the end or beggining. Otherwise the
		 *								curve value will be clamped.
		 * @return						Interpolated value from the curve at provided time.
		 */
		T evaluate(float time, const TCurveCache<T>& cache, bool loop = true) const;

		/**
		 * Evaluate the animation curve at the specified time. If evaluating multiple values in a sequential order consider
		 * using the cached version of evaluate() for better performance.
		 *
		 * @param[in]	time	%Time to evaluate the curve at.		
		 * @param[in]	loop	If true the curve will loop when it goes past the end or beggining. Otherwise the curve 
		 *						value will be clamped.
		 * @return				Interpolated value from the curve at provided time.
		 */
		T evaluate(float time, bool loop = true) const;

		/**
		 * Evaluate the animation curve at the specified time and returns a new keyframe containing the evaluated value
		 * and tangents.
		 *
		 * @param[in]	time	%Time to evaluate the curve at.		
		 * @param[in]	loop	If true the curve will loop when it goes past the end or beggining. Otherwise the curve 
		 *						value will be clamped.
		 * @return				Keyframe containing the interpolated value and tangents at provided time.
		 */
		KeyFrame evaluateKey(float time, bool loop = true) const;

		/** 
		 * Splits a piece of the animation curve into a separate animation curve. 
		 *
		 * @param[in]	start	Beginning time of the split curve.
		 * @param[in]	end		End time of the split curve.
		 * @return				New curve with data corresponding to the provided split times.
		 */
		TAnimationCurve<T> split(float start, float end);

		/** 
		 * Converts a normal curve into an additive curve. It is assumed the first keyframe in the curve is the reference
		 * key from which to generate the additive curve. Such curves can then be added on top of a curve containing
		 * reference keys.
		 */
		void makeAdditive();

		/** Returns the length of the animation curve, from time zero to last keyframe. */
		float getLength() const { return mEnd; }

		/** Returns the total number of key-frames in the curve. */
		UINT32 getNumKeyFrames() const { return (UINT32)mKeyframes.size(); }

		/** Returns a keyframe at the specified index. */
		const TKeyframe<T>& getKeyFrame(UINT32 idx) const { return mKeyframes[idx]; }

	private:
		friend struct RTTIPlainType<TAnimationCurve<T>>;

		/** 
		 * Returns a pair of keys that can be used for interpolating to field the value at the provided time. This attempts
		 * to find keys using the cache first, and if not possible falls back to a full search.
		 *
		 * @param[in]	time			Time for which to find the relevant keys from. It is expected to be clamped to a
		 *								valid range within the curve.
		 * @param[in]	cache			Animation instance data holding the time to evaluate the curve at, and any cached
		 *								data from previous requests. Time is expected to be clamped to a valid range
		 *								within the curve.
		 * @param[out]	leftKey			Index of the key to interpolate from.
		 * @param[out]	rightKey		Index of the key to interpolate to.
		 */
		void findKeys(float time, const TCurveCache<T>& cache, UINT32& leftKey, UINT32& rightKey) const;

		/** 
		 * Returns a pair of keys that can be used for interpolating to field the value at the provided time. 
		 *
		 * @param[in]	time			Time for which to find the relevant keys from. It is expected to be clamped to a
		 *								valid range within the curve.
		 * @param[out]	leftKey			Index of the key to interpolate from.
		 * @param[out]	rightKey		Index of the key to interpolate to.
		 */
		void findKeys(float time, UINT32& leftKey, UINT32& rightKey) const;

		/** Returns a keyframe index nearest to the provided time. */
		UINT32 findKey(float time);

		/** 
		 * Calculates a key in-between the provided two keys. 
		 *
		 * @param[in]	lhs		Key to interpolate from.
		 * @param[in]	rhs		Key to interpolate to.
		 * @param[in]	time	Curve time to interpolate the keys at.
		 * @return				Interpolated key value.
		 */
		KeyFrame evaluateKey(const KeyFrame& lhs, const KeyFrame& rhs, float time) const;

		/** 
		 * Evaluates a value at the cached curve. Caller must ensure the request time falls within the cached curve range.
		 *
		 * @param[in]	time			Time to evaluate the curve at.	
		 * @param[in]	animInstance	Animation instance data holding the time to evaluate the curve at, and any cached
		 *								data from previous requests.
		 * @return						Interpolated value from the curve at provided time.
		 */
		T evaluateCache(float time, const TCurveCache<T>& animInstance) const;

		static const UINT32 CACHE_LOOKAHEAD;

		Vector<KeyFrame> mKeyframes;
		float mStart;
		float mEnd;
		float mLength;
	};

	/** Flags that described an TAnimationCurve<T>. */
	enum class AnimationCurveFlag
	{
		/** Signifies that the curve was imported from an external file, and not created manually in-engine. */
		ImportedCurve = 1 << 0,
		/** Signifies the curve is used to animate between different frames within a morph channel. In range [0, 1]. */
		MorphFrame = 1 << 1,
		/** Signifies the curve is used to adjust the weight of a morph channel. In range [0, 1]. */
		MorphWeight = 1 << 2
	};

	typedef Flags<AnimationCurveFlag> AnimationCurveFlags;
	BS_FLAGS_OPERATORS(AnimationCurveFlag);

	/** An animation curve and its name. */
	template <class T>
	struct TNamedAnimationCurve
	{
		String name;
		AnimationCurveFlags flags;
		TAnimationCurve<T> curve;
	};

	/** @} */
}