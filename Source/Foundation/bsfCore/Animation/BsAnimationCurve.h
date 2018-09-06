//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Animation/BsCurveCache.h"
#include "Math/BsVector3.h"
#include "Math/BsVector2.h"
#include "Math/BsQuaternion.h"
#include "Allocators/BsPoolAlloc.h"

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

	/** Keyframe specialization for integers (no tangents). */
	template <>
	struct BS_SCRIPT_EXPORT(m:Animation,n:KeyFrameInt,pl:true) TKeyframe<INT32>
	{
		INT32 value; /**< Value of the key. */
		float time; /**< Position of the key along the animation spline. */
	};

	template struct BS_SCRIPT_EXPORT(m:Animation,n:KeyFrame,pl:true) TKeyframe<float>;
	template struct BS_SCRIPT_EXPORT(m:Animation,n:KeyFrameVec3,pl:true) TKeyframe<Vector3>;
	template struct BS_SCRIPT_EXPORT(m:Animation,n:KeyFrameVec2,pl:true) TKeyframe<Vector2>;
	template struct BS_SCRIPT_EXPORT(m:Animation,n:KeyFrameQuat,pl:true) TKeyframe<Quaternion>;

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

		/**
		 * Creates a new animation curve.
		 * 
		 * @param[in]	keyframes	Keyframes to initialize the curve with. They must be sorted by time.
		 */
		BS_SCRIPT_EXPORT()
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
		BS_SCRIPT_EXPORT()
		T evaluate(float time, bool loop = true) const;

		/**
		 * Evaluates the integrated animation curve. (e.g. evaluating a curve containing velocity values will return
		 * a position).
		 *
		 * @param[in]	time				%Time to evaluate the curve at.
		 * @param[in]	integrationCache	Cache storing the values required for integration. Generated the first time
		 *									this method is called and re-used on subsequent calls. Caller must ensure to
		 *									use the cache only with the curve it was originally used on. Separate caches
		 *									need to be used for single and double integration evaluation.
		 * @return							Interpolated value from the curve at provided time.
		 */
		T evaluateIntegrated(float time, const TCurveIntegrationCache<T>& integrationCache) const;

		/**
		 * Evaluates the double integrated animation curve. (e.g. evaluating a curve containing acceleration values will
		 * return a position).
		 *
		 * @param[in]	time				%Time to evaluate the curve at.
		 * @param[in]	integrationCache	Cache storing the values required for integration. Generated the first time
		 *									this method is called and re-used on subsequent calls. Caller must ensure to
		 *									use the cache only with the curve it was originally used on. Separate caches
		 *									need to be used for single and double integration evaluation.
		 * @return							Interpolated value from the curve at provided time.
		 */
		T evaluateIntegratedDouble(float time, const TCurveIntegrationCache<T>& integrationCache) const;

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

		/** Returns the time of the first and last keyframe in the curve. */
		std::pair<float, float> getTimeRange() const;

		/** Calculates the minimal and maximal value of the curve. */
		std::pair<T, T> calculateRange() const;

		/** Calculates the minimal and maximal value of the integrated curve. */
		std::pair<T, T> calculateRangeIntegrated(const TCurveIntegrationCache<T>& cache) const;

		/** Calculates the minimal and maximal value of the doubly integrated curve. */
		std::pair<T, T> calculateRangeIntegratedDouble(const TCurveIntegrationCache<T>& cache) const;

		/** Returns the length of the animation curve, from time zero to last keyframe. */
		float getLength() const { return mEnd; }

		/** Returns the total number of key-frames in the curve. */
		UINT32 getNumKeyFrames() const { return (UINT32)mKeyframes.size(); }

		/** Returns a keyframe at the specified index. */
		const TKeyframe<T>& getKeyFrame(UINT32 idx) const { return mKeyframes[idx]; }

		/** Returns a list of all keyframes in the curve. */
		BS_SCRIPT_EXPORT(n:KeyFrames,pr:getter)
		const Vector<TKeyframe<T>>& getKeyFrames() const { return mKeyframes; }

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

		/** Creates a cache used for quick evaluation of single integrated curves. */
		void buildIntegrationCache(const TCurveIntegrationCache<T>& cache) const;

		/** Creates a cache used for quick evaluation of double integrated curves. */
		void buildDoubleIntegrationCache(const TCurveIntegrationCache<T>& cache) const;

		static const UINT32 CACHE_LOOKAHEAD;

		Vector<KeyFrame> mKeyframes;
		float mStart;
		float mEnd;
		float mLength;
	};

#ifdef BS_SBGEN
	template class BS_SCRIPT_EXPORT(m:Animation,n:AnimationCurve) TAnimationCurve<float>;
	template class BS_SCRIPT_EXPORT(m:Animation,n:Vector3Curve) TAnimationCurve<Vector3>;
	template class BS_SCRIPT_EXPORT(m:Animation,n:Vector2Curve) TAnimationCurve<Vector2>;
	template class BS_SCRIPT_EXPORT(m:Animation,n:QuaternionCurve) TAnimationCurve<Quaternion>;
	template class BS_SCRIPT_EXPORT(m:Animation,n:IntegerCurve) TAnimationCurve<INT32>;
#endif

	/** Flags that describe an animation curve. */
	enum BS_SCRIPT_EXPORT(n:AnimationCurveFlags) class AnimationCurveFlag
	{
		/**
		 * If enabled, the curve was imported from an external file and not created within the engine. This will affect
		 * how are animation results applied to scene objects (with imported animations it is assumed the curve is
		 * animating bones and with in-engine curves it is assumed the curve is animating scene objects).
		 */
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
		TNamedAnimationCurve() { }

		/**
		 * Constructs a new named animation curve.
		 * 
		 * @param[in]	name	Name of the curve.
		 * @param[in]	curve	Curve containing the animation data.
		 */
		TNamedAnimationCurve(const String& name, const TAnimationCurve<T> curve)
			:name(name), curve(curve)
		{ }

		/**
		 * Constructs a new named animation curve.
		 * 
		 * @param[in]	name	Name of the curve.
		 * @param[in]	flags	Flags that describe the animation curve.
		 * @param[in]	curve	Curve containing the animation data.
		 */
		TNamedAnimationCurve(const String& name, AnimationCurveFlags flags, const TAnimationCurve<T> curve)
			:name(name), curve(curve)
		{ }

		/** Name of the curve. */
		String name;

		/** Flags that describe the animation curve. */
		AnimationCurveFlags flags;

		/** Actual curve containing animation data. */
		TAnimationCurve<T> curve;
	};

#ifdef BS_SBGEN
	template class BS_SCRIPT_EXPORT(m:Animation,n:NamedFloatCurve,pl:true) TNamedAnimationCurve<float>;
	template class BS_SCRIPT_EXPORT(m:Animation,n:NamedVector3Curve,pl:true) TNamedAnimationCurve<Vector3>;
	template class BS_SCRIPT_EXPORT(m:Animation,n:NamedVector2Curve,pl:true) TNamedAnimationCurve<Vector2>;
	template class BS_SCRIPT_EXPORT(m:Animation,n:NamedQuaternionCurve,pl:true) TNamedAnimationCurve<Quaternion>;
	template class BS_SCRIPT_EXPORT(m:Animation,n:NamedIntegerCurve,pl:true) TNamedAnimationCurve<INT32>;
#endif

	/** @} */

	IMPLEMENT_GLOBAL_POOL(TAnimationCurve<float>, 32)
}
