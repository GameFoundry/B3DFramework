//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Animation/B3DCurveCache.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector2.h"
#include "Math/B3DQuaternion.h"
#include "Allocators/B3DPoolAlloc.h"
#include "Script/B3DIScriptExportable.h"

namespace b3d
{
	/** @addtogroup Animation-Internal
	 *  @{
	 */

	/** Animation keyframe, represented as an endpoint of a cubic hermite spline. */
	template <class T>
	struct TKeyframe
	{
		T Value; /**< Value of the key. */
		T InTangent; /**< Input tangent (going from the previous key to this one) of the key. */
		T OutTangent; /**< Output tangent (going from this key to next one) of the key. */
		float Time; /**< Position of the key along the animation spline. */

		bool operator==(const TKeyframe<T>& rhs) const
		{
			return (Value == rhs.Value && InTangent == rhs.InTangent && OutTangent == rhs.OutTangent && Time == rhs.Time);
		}

		bool operator!=(const TKeyframe<T>& rhs) const
		{
			return !operator==(rhs);
		}
	};

	/** Keyframe specialization for integers (no tangents). */
	template <>
	struct B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(KeyFrameInt), ExportAsStruct(true)) TKeyframe<i32>
	{
		i32 Value; /**< Value of the key. */
		float Time; /**< Position of the key along the animation spline. */

		bool operator==(const TKeyframe<i32>& rhs) const
		{
			return (Value == rhs.Value && Time == rhs.Time);
		}

		bool operator!=(const TKeyframe<i32>& rhs) const
		{
			return !operator==(rhs);
		}
	};

	template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(KeyFrame), ExportAsStruct(true)) TKeyframe<float>;
	template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(KeyFrameVec3), ExportAsStruct(true)) TKeyframe<Vector3>;
	template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(KeyFrameVec2), ExportAsStruct(true)) TKeyframe<Vector2>;
	template struct B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(KeyFrameQuat), ExportAsStruct(true)) TKeyframe<Quaternion>;

	/**
	 * Animation spline represented by a set of keyframes, each representing an endpoint of a cubic hermite curve. The
	 * spline can be evaluated at any time, and uses caching to speed up multiple sequential evaluations.
	 */
	template <class T>
	class B3D_EXPORT TAnimationCurve : public IScriptExportable // Note: Curves are expected to be immutable for threading purposes
	{
	public:
		typedef TKeyframe<T> KeyFrame;

		TAnimationCurve() = default;

		/**
		 * Creates a new animation curve.
		 *
		 * @param	keyframes	Keyframes to initialize the curve with. They must be sorted by time.
		 */
		B3D_SCRIPT_EXPORT()
		TAnimationCurve(const Vector<KeyFrame>& keyframes);

		/**
		 * Evaluate the animation curve using caching. Caching can significantly speed of evaluation if the evaluation
		 * happens sequential order (which should be true for most curves). If evaluation is not happening in sequential
		 * order using the non-caching version of Evaluate() might yield better performance.
		 *
		 * @param	time		%Time to evaluate the curve at.
		 * @param	cache		Cached data from previous requests that can be used for speeding up sequential calls
		 *						to this method. Caller should ensure to maintain a persistent instance of this data
		 *						for every animation using this curve in order to ensure cache is maintained.
		 * @param	loop		If true the curve will loop when it goes past the end or beggining. Otherwise the
		 *						curve value will be clamped.
		 * @return				Interpolated value from the curve at provided time.
		 */
		T Evaluate(float time, const TCurveCache<T>& cache, bool loop = true) const;

		/**
		 * Evaluate the animation curve at the specified time. If evaluating multiple values in a sequential order consider
		 * using the cached version of Evaluate() for better performance.
		 *
		 * @param	time	%Time to evaluate the curve at.
		 * @param	loop	If true the curve will loop when it goes past the end or beggining. Otherwise the curve
		 *					value will be clamped.
		 * @return			Interpolated value from the curve at provided time.
		 */
		B3D_SCRIPT_EXPORT()
		T Evaluate(float time, bool loop = true) const;

		/**
		 * Evaluates the integrated animation curve. (e.g. evaluating a curve containing velocity values will return
		 * a position).
		 *
		 * @param	time				%Time to evaluate the curve at.
		 * @param	integrationCache	Cache storing the values required for integration. Generated the first time
		 *									this method is called and re-used on subsequent calls. Caller must ensure to
		 *									use the cache only with the curve it was originally used on. Separate caches
		 *									need to be used for single and double integration evaluation.
		 * @return						Interpolated value from the curve at provided time.
		 */
		T EvaluateIntegrated(float time, const TCurveIntegrationCache<T>& integrationCache) const;

		/**
		 * Evaluates the double integrated animation curve. (e.g. evaluating a curve containing acceleration values will
		 * return a position).
		 *
		 * @param	time				%Time to evaluate the curve at.
		 * @param	integrationCache	Cache storing the values required for integration. Generated the first time
		 *									this method is called and re-used on subsequent calls. Caller must ensure to
		 *									use the cache only with the curve it was originally used on. Separate caches
		 *									need to be used for single and double integration evaluation.
		 * @return						Interpolated value from the curve at provided time.
		 */
		T EvaluateIntegratedDouble(float time, const TCurveIntegrationCache<T>& integrationCache) const;

		/**
		 * Evaluate the animation curve at the specified time and returns a new keyframe containing the evaluated value
		 * and tangents.
		 *
		 * @param	time	%Time to evaluate the curve at.
		 * @param	loop	If true the curve will loop when it goes past the end or beginning. Otherwise the curve
		 *					value will be clamped.
		 * @return			Keyframe containing the interpolated value and tangents at provided time.
		 */
		KeyFrame EvaluateKey(float time, bool loop = true) const;

		/**
		 * Splits a piece of the animation curve into a separate animation curve.
		 *
		 * @param	start	Beginning time of the split curve.
		 * @param	end		End time of the split curve.
		 * @return				New curve with data corresponding to the provided split times.
		 */
		TAnimationCurve<T> Split(float start, float end);

		/**
		 * Converts a normal curve into an additive curve. It is assumed the first keyframe in the curve is the reference
		 * key from which to generate the additive curve. Such curves can then be added on top of a curve containing
		 * reference keys.
		 */
		void MakeAdditive();

		/** Returns the time of the first and last keyframe in the curve. */
		std::pair<float, float> GetTimeRange() const;

		/** Calculates the minimal and maximal value of the curve. */
		std::pair<T, T> CalculateRange() const;

		/** Calculates the minimal and maximal value of the integrated curve. */
		std::pair<T, T> CalculateRangeIntegrated(const TCurveIntegrationCache<T>& cache) const;

		/** Calculates the minimal and maximal value of the doubly integrated curve. */
		std::pair<T, T> CalculateRangeIntegratedDouble(const TCurveIntegrationCache<T>& cache) const;

		/** Returns the length of the animation curve, from time zero to last keyframe. */
		float GetLength() const { return mEnd; }

		/** Returns the total number of key-frames in the curve. */
		u32 GetNumKeyFrames() const { return (u32)mKeyframes.size(); }

		/** Returns a keyframe at the specified index. */
		const TKeyframe<T>& GetKeyFrame(u32 index) const { return mKeyframes[index]; }

		/** Returns a list of all keyframes in the curve. */
		B3D_SCRIPT_EXPORT(ExportName(KeyFrames), Property(Getter))

		const Vector<TKeyframe<T>>& GetKeyFrames() const { return mKeyframes; }

		bool operator==(const TAnimationCurve<T>& rhs) const;

		bool operator!=(const TAnimationCurve<T>& rhs) const { return !operator==(rhs); }

	private:
		friend struct RTTIPlainType<TAnimationCurve<T>>;

		/**
		 * Returns a pair of keys that can be used for interpolating to field the value at the provided time. This attempts
		 * to find keys using the cache first, and if not possible falls back to a full search.
		 *
		 * @param	time			Time for which to find the relevant keys from. It is expected to be clamped to a
		 *							valid range within the curve.
		 * @param	cache			Animation instance data holding the time to evaluate the curve at, and any cached
		 *							data from previous requests. Time is expected to be clamped to a valid range
		 *							within the curve.
		 * @param	outLeftKey		Index of the key to interpolate from.
		 * @param	outRightKey		Index of the key to interpolate to.
		 */
		void FindKeys(float time, const TCurveCache<T>& cache, u32& outLeftKey, u32& outRightKey) const;

		/**
		 * Returns a pair of keys that can be used for interpolating to field the value at the provided time.
		 *
		 * @param	time			Time for which to find the relevant keys from. It is expected to be clamped to a
		 *							valid range within the curve.
		 * @param	outLeftKey		Index of the key to interpolate from.
		 * @param	outRightKey		Index of the key to interpolate to.
		 */
		void FindKeys(float time, u32& outLeftKey, u32& outRightKey) const;

		/** Returns a keyframe index nearest to the provided time. */
		u32 FindKey(float time);

		/**
		 * Calculates a key in-between the provided two keys.
		 *
		 * @param	lhs		Key to interpolate from.
		 * @param	rhs		Key to interpolate to.
		 * @param	time	Curve time to interpolate the keys at.
		 * @return				Interpolated key value.
		 */
		KeyFrame EvaluateKey(const KeyFrame& lhs, const KeyFrame& rhs, float time) const;

		/** Creates a cache used for quick evaluation of single integrated curves. */
		void BuildIntegrationCache(const TCurveIntegrationCache<T>& cache) const;

		/** Creates a cache used for quick evaluation of double integrated curves. */
		void BuildDoubleIntegrationCache(const TCurveIntegrationCache<T>& cache) const;

		static const u32 kCacheLookahead;

		Vector<KeyFrame> mKeyframes;
		float mStart = 0.0f;
		float mEnd = 0.0f;
		float mLength = 0.0f;
	};

#ifdef B3D_CODEGEN
	template class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(AnimationCurve)) TAnimationCurve<float>;
	template class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(Vector3Curve)) TAnimationCurve<Vector3>;
	template class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(Vector2Curve)) TAnimationCurve<Vector2>;
	template class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(QuaternionCurve)) TAnimationCurve<Quaternion>;
	template class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(IntegerCurve)) TAnimationCurve<i32>;
#endif

	/** Flags that describe an animation curve. */
	enum class B3D_SCRIPT_EXPORT(ExportName(AnimationCurveFlags)) AnimationCurveFlag
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
	B3D_FLAGS_OPERATORS(AnimationCurveFlag);

	/** An animation curve and its name. */
	template <class T>
	struct TNamedAnimationCurve
	{
		TNamedAnimationCurve() = default;

		/**
		 * Constructs a new named animation curve.
		 *
		 * @param	name	Name of the curve.
		 * @param	curve	Curve containing the animation data.
		 */
		TNamedAnimationCurve(const String& name, const TAnimationCurve<T> curve)
			: Name(name), Curve(curve)
		{}

		/**
		 * Constructs a new named animation curve.
		 *
		 * @param	name	Name of the curve.
		 * @param	flags	Flags that describe the animation curve.
		 * @param	curve	Curve containing the animation data.
		 */
		TNamedAnimationCurve(const String& name, AnimationCurveFlags flags, const TAnimationCurve<T> curve)
			: Name(name), Curve(curve)
		{}

		/** Name of the curve. */
		String Name;

		/** Flags that describe the animation curve. */
		AnimationCurveFlags Flags;

		/** Actual curve containing animation data. */
		TAnimationCurve<T> Curve;
	};

#ifdef B3D_CODEGEN 
	template class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(NamedFloatCurve), ExportAsStruct(true)) TNamedAnimationCurve<float>;
	template class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(NamedVector3Curve), ExportAsStruct(true)) TNamedAnimationCurve<Vector3>;
	template class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(NamedVector2Curve), ExportAsStruct(true)) TNamedAnimationCurve<Vector2>;
	template class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(NamedQuaternionCurve), ExportAsStruct(true)) TNamedAnimationCurve<Quaternion>;
	template class B3D_SCRIPT_EXPORT(DocumentationGroup(Animation), ExportName(NamedIntegerCurve), ExportAsStruct(true)) TNamedAnimationCurve<i32>;
#endif

	/** @} */

	B3D_IMPLEMENT_GLOBAL_POOL(TAnimationCurve<float>, 32)
} // namespace b3d
