//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/BsAnimationCurve.h"
#include "Private/RTTI/BsAnimationCurveRTTI.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"
#include "Math/BsMath.h"
#include "Animation/BsAnimationUtility.h"

namespace bs
{
	namespace impl
	{
		/**
		 * Checks if any components of the keyframes are constant (step) functions and updates the hermite curve coefficients
		 * accordingly.
		 */
		void setStepCoefficients(const TKeyframe<float>& lhs, const TKeyframe<float>& rhs, float(&coefficients)[4])
		{
			if (lhs.outTangent != std::numeric_limits<float>::infinity() &&
				rhs.inTangent != std::numeric_limits<float>::infinity())
				return;

			coefficients[0] = 0.0f;
			coefficients[1] = 0.0f;
			coefficients[2] = 0.0f;
			coefficients[3] = lhs.value;
		}

		void setStepCoefficients(const TKeyframe<Vector3>& lhs, const TKeyframe<Vector3>& rhs, Vector3(&coefficients)[4])
		{
			for (UINT32 i = 0; i < 3; i++)
			{
				if (lhs.outTangent[i] != std::numeric_limits<float>::infinity() &&
					rhs.inTangent[i] != std::numeric_limits<float>::infinity())
					continue;

				coefficients[0][i] = 0.0f;
				coefficients[1][i] = 0.0f;
				coefficients[2][i] = 0.0f;
				coefficients[3][i] = lhs.value[i];
			}
		}

		void setStepCoefficients(const TKeyframe<Quaternion>& lhs, const TKeyframe<Quaternion>& rhs, Quaternion(&coefficients)[4])
		{
			for (UINT32 i = 0; i < 4; i++)
			{
				if (lhs.outTangent[i] != std::numeric_limits<float>::infinity() &&
					rhs.inTangent[i] != std::numeric_limits<float>::infinity())
					continue;

				coefficients[0][i] = 0.0f;
				coefficients[1][i] = 0.0f;
				coefficients[2][i] = 0.0f;
				coefficients[3][i] = lhs.value[i];
			}
		}

		/** Checks if any components of the keyframes are constant (step) functions and updates the key value. */
		void setStepValue(const TKeyframe<float>& lhs, const TKeyframe<float>& rhs, float& value)
		{
			if (lhs.outTangent != std::numeric_limits<float>::infinity() &&
				rhs.inTangent != std::numeric_limits<float>::infinity())
				return;

			value = lhs.value;
		}

		void setStepValue(const TKeyframe<Vector3>& lhs, const TKeyframe<Vector3>& rhs, Vector3& value)
		{
			for (UINT32 i = 0; i < 3; i++)
			{
				if (lhs.outTangent[i] != std::numeric_limits<float>::infinity() &&
					rhs.inTangent[i] != std::numeric_limits<float>::infinity())
					continue;

				value[i] = lhs.value[i];
			}
		}

		void setStepValue(const TKeyframe<Quaternion>& lhs, const TKeyframe<Quaternion>& rhs, Quaternion& value)
		{
			for (UINT32 i = 0; i < 4; i++)
			{
				if (lhs.outTangent[i] != std::numeric_limits<float>::infinity() &&
					rhs.inTangent[i] != std::numeric_limits<float>::infinity())
					continue;

				value[i] = lhs.value[i];
			}
		}

		/** Checks if any components of the keyframes are constant (step) functions and updates the key tangent. */
		void setStepTangent(const TKeyframe<float>& lhs, const TKeyframe<float>& rhs, float& tangent)
		{
			if (lhs.outTangent != std::numeric_limits<float>::infinity() &&
				rhs.inTangent != std::numeric_limits<float>::infinity())
				return;

			tangent = std::numeric_limits<float>::infinity();
		}

		void setStepTangent(const TKeyframe<Vector3>& lhs, const TKeyframe<Vector3>& rhs, Vector3& tangent)
		{
			for (UINT32 i = 0; i < 3; i++)
			{
				if (lhs.outTangent[i] != std::numeric_limits<float>::infinity() &&
					rhs.inTangent[i] != std::numeric_limits<float>::infinity())
					continue;

				tangent[i] = std::numeric_limits<float>::infinity();
			}
		}

		void setStepTangent(const TKeyframe<Quaternion>& lhs, const TKeyframe<Quaternion>& rhs, Quaternion& tangent)
		{
			for (UINT32 i = 0; i < 4; i++)
			{
				if (lhs.outTangent[i] != std::numeric_limits<float>::infinity() &&
					rhs.inTangent[i] != std::numeric_limits<float>::infinity())
					continue;

				tangent[i] = std::numeric_limits<float>::infinity();
			}
		}

		/** Calculates the difference between two values. */
		float getDiff(float lhs, float rhs)
		{
			return lhs - rhs;
		}

		Vector3 getDiff(const Vector3& lhs, const Vector3& rhs)
		{
			return lhs - rhs;
		}

		Quaternion getDiff(const Quaternion& lhs, const Quaternion& rhs)
		{
			return rhs.inverse() * lhs;
		}

		INT32 getDiff(INT32 lhs, INT32 rhs)
		{
			return lhs - rhs;
		}

		template <class T>
		T getZero() { return 0.0f; }

		template<>
		float getZero<float>() { return 0.0f; }

		template<>
		Vector3 getZero<Vector3>() { return Vector3(BsZero); }

		template<>
		Quaternion getZero<Quaternion>() { return Quaternion(BsZero); }

		template<>
		INT32 getZero<INT32>() { return 0; }

		template <class T>
		constexpr UINT32 getNumComponents() { return 1; }

		template<>
		constexpr UINT32 getNumComponents<Vector3>() { return 3; }

		template<>
		constexpr UINT32 getNumComponents<Quaternion>() { return 4; }

		template <class T>
		float& getComponent(T& val, UINT32 idx) { return val; }

		template<>
		float& getComponent(Vector3& val, UINT32 idx) { return val[idx]; }

		template<>
		float& getComponent(Quaternion& val, UINT32 idx) { return val[idx]; }

		template <class T>
		float getComponent(const T& val, UINT32 idx) { return val; }

		template<>
		float getComponent(const Vector3& val, UINT32 idx) { return val[idx]; }

		template<>
		float getComponent(const Quaternion& val, UINT32 idx) { return val[idx]; }

		template <class T>
		void getMinMax(std::pair<T, T>& minmax, const T& value)
		{
			minmax.first = std::min(minmax.first, value);
			minmax.second = std::max(minmax.second, value);
		}

		template <>
		void getMinMax(std::pair<Vector3, Vector3>& minmax, const Vector3& value)
		{
			minmax.first = Vector3::min(minmax.first, value);
			minmax.second = Vector3::max(minmax.second, value);
		}

		template <>
		void getMinMax(std::pair<Quaternion, Quaternion>& minmax, const Quaternion& value)
		{
			minmax.first = Quaternion::min(minmax.first, value);
			minmax.second = Quaternion::max(minmax.second, value);
		}

		template<class T>
		TKeyframe<T> evaluateKey(const TKeyframe<T>& lhs, const TKeyframe<T>& rhs, float time)
		{
			float length = rhs.time - lhs.time;

			if (Math::approxEquals(length, 0.0f))
				return lhs;

			// Resize tangents since we're not evaluating the curve over unit range
			float invLength = 1.0f / length;
			float t = (time - lhs.time) * invLength;
			T leftTangent = lhs.outTangent * length;
			T rightTangent = rhs.inTangent * length;

			TKeyframe<T> output;
			output.time = time;
			output.value = Math::cubicHermite(t, lhs.value, rhs.value, leftTangent, rightTangent);
			output.inTangent = Math::cubicHermiteD1(t, lhs.value, rhs.value, leftTangent, rightTangent) * invLength;

			setStepValue(lhs, rhs, output.value);
			setStepTangent(lhs, rhs, output.inTangent);

			output.outTangent = output.inTangent;

			return output;
		}

		template<>
		TKeyframe<INT32> evaluateKey(const TKeyframe<INT32>& lhs, const TKeyframe<INT32>& rhs, float time)
		{
			TKeyframe<INT32> output;
			output.time = time;
			output.value = time >= rhs.time ? rhs.value : lhs.value;

			return output;
		}

		template <class T>
		T evaluateCubic(float time, float start, float end, T (&coeffs)[4])
		{
			float t = time - start;
			return t * (t * (t * coeffs[0] + coeffs[1]) + coeffs[2]) + coeffs[3];
		}

		template <>
		INT32 evaluateCubic(float time, float start, float end, INT32 (&coeffs)[4])
		{
			return time >= end ? coeffs[1] : coeffs[0];
		}

		template<class T>
		void calculateCoeffs(const TKeyframe<T>& lhs, const TKeyframe<T>& rhs, float time, T (&coeffs)[4])
		{
			float length = rhs.time - lhs.time;

			// Handle the case where both keys are identical, or close enough to cause precision issues
			if (length < 0.000001f)
			{
				coeffs[0] = impl::getZero<T>();
				coeffs[1] = impl::getZero<T>();
				coeffs[2] = impl::getZero<T>();
				coeffs[3] = lhs.value;
			}
			else
				Math::cubicHermiteCoefficients(lhs.value, rhs.value, lhs.outTangent, rhs.inTangent, length, coeffs);

			setStepCoefficients(lhs, rhs, coeffs);
		}

		template<>
		void calculateCoeffs(const TKeyframe<INT32>& lhs, const TKeyframe<INT32>& rhs, float time, INT32 (&coeffs)[4])
		{
			coeffs[0] = lhs.value;
			coeffs[1] = rhs.value;
		}

		template<class T>
		T evaluateAndUpdateCache(const TKeyframe<T>& lhs, const TKeyframe<T>& rhs, float time, T (&coeffs)[4])
		{
			calculateCoeffs(lhs, rhs, time, coeffs);

			return impl::evaluateCubic(time, lhs.time, rhs.time, coeffs);
		}

		template<>
		INT32 evaluateAndUpdateCache(const TKeyframe<INT32>& lhs, const TKeyframe<INT32>& rhs, float time, 
			INT32 (&coeffs)[4])
		{
			coeffs[0] = lhs.value;
			coeffs[1] = rhs.value;

			return time >= rhs.time ? rhs.value : lhs.value;
		}

		template<class T>
		T evaluate(const TKeyframe<T>& lhs, const TKeyframe<T>& rhs, float time)
		{
			float length = rhs.time - lhs.time;
			assert(length > 0.0f);

			float t;
			T leftTangent;
			T rightTangent;

			if (Math::approxEquals(length, 0.0f))
			{
				t = 0.0f;
				leftTangent = impl::getZero<T>();
				rightTangent = impl::getZero<T>();
			}
			else
			{
				// Scale from arbitrary range to [0, 1]
				t = (time - lhs.time) / length;
				leftTangent = lhs.outTangent * length;
				rightTangent = rhs.inTangent * length;
			}

			T output = Math::cubicHermite(t, lhs.value, rhs.value, leftTangent, rightTangent);
			setStepValue(lhs, rhs, output);

			return output;
		}

		template<>
		INT32 evaluate(const TKeyframe<INT32>& lhs, const TKeyframe<INT32>& rhs, float time)
		{
			return time >= rhs.time ? rhs.value : lhs.value;
		}

		template <class T>
		void integrate(T (&coeffs)[4])
		{
			coeffs[0] = (T)(coeffs[0] / 4.0f);
			coeffs[1] = (T)(coeffs[1] / 3.0f);
			coeffs[2] = (T)(coeffs[2] / 2.0f);
		}

		template <class T>
		void calcMinMax(std::pair<T, T>& minmax, float start, float end, T(&coeffs)[4])
		{
			// Differentiate
			T a = (T)(3.0f * coeffs[0]);
			T b = (T)(2.0f * coeffs[1]);
			T c = (T)(1.0f * coeffs[2]);

			const UINT32 numComponents = getNumComponents<T>();

			for (UINT32 i = 0; i < numComponents; i++)
			{
				float roots[2];
				const UINT32 numRoots = Math::solveQuadratic(
					getComponent(a, i),
					getComponent(b, i),
					getComponent(c, i),
					roots);

				for (UINT32 j = 0; j < numRoots; j++)
				{
					if ((roots[j] >= 0.0f) && ((start + roots[j]) < end))
					{
						float fltCoeffs[4] =
						{
							getComponent(coeffs[0], i),
							getComponent(coeffs[1], i),
							getComponent(coeffs[2], i),
							getComponent(coeffs[3], i)
						};

						float value = evaluateCubic(roots[j], 0.0f, 0.0f, fltCoeffs);

						getComponent(minmax.first, i) = std::min(getComponent(minmax.first, i), value);
						getComponent(minmax.second, i) = std::max(getComponent(minmax.second, i), value);
					}
				}
			}
		}

		template <>
		void calcMinMax(std::pair<INT32, INT32>& minmax, float start, float end, INT32(&coeffs)[4])
		{
			getMinMax(minmax, coeffs[0]);
			getMinMax(minmax, coeffs[1]);
		}

		template <class T>
		void calcMinMaxIntegrated(std::pair<T, T>& minmax, float start, float end, const T& sum, T(&coeffs)[4])
		{
			// Differentiate
			T a = 4.0f * coeffs[0];
			T b = 3.0f * coeffs[1];
			T c = 2.0f * coeffs[2];
			T d = 1.0f * coeffs[3];

			const UINT32 numComponents = getNumComponents<T>();

			for (UINT32 i = 0; i < numComponents; i++)
			{
				float roots[3];
				const UINT32 numRoots = Math::solveCubic(
					getComponent(a, i),
					getComponent(b, i),
					getComponent(c, i),
					getComponent(d, i),
					roots);

				for (UINT32 j = 0; j < numRoots; j++)
				{
					if ((roots[j] >= 0.0f) && ((start + roots[j]) < end))
					{
						float fltCoeffs[4] =
						{
							getComponent(coeffs[0], i),
							getComponent(coeffs[1], i),
							getComponent(coeffs[2], i),
							getComponent(coeffs[3], i)
						};

						float value = getComponent(sum, i) + evaluateCubic(roots[j], 0.0f, 0.0f, fltCoeffs) * roots[j];

						getComponent(minmax.first, i) = std::min(getComponent(minmax.first, i), value);
						getComponent(minmax.second, i) = std::max(getComponent(minmax.second, i), value);
					}
				}
			}
		}

		template <>
		void calcMinMaxIntegrated(std::pair<INT32, INT32>& minmax, float start, float end, const INT32& sum, 
			INT32(&coeffs)[4])
		{
			assert(false && "Not implemented");
		}

		template <class T>
		void calcMinMaxIntegratedDouble(std::pair<T, T>& minmax, float start, float end, const T& doubleSum, 
			const T& sum, T(&coeffs)[4])
		{
			// Differentiate
			T a = 5.0f * coeffs[0];
			T b = 4.0f * coeffs[1];
			T c = 3.0f * coeffs[2];
			T d = 2.0f * coeffs[3];

			const UINT32 numComponents = getNumComponents<T>();

			for (UINT32 i = 0; i < numComponents; i++)
			{
				float roots[4];
				const UINT32 numRoots = Math::solveQuartic(
					getComponent(a, i),
					getComponent(b, i),
					getComponent(c, i),
					getComponent(d, i),
					0.0f,
					roots);

				for (UINT32 j = 0; j < numRoots; j++)
				{
					if ((roots[j] >= 0.0f) && ((start + roots[j]) < end))
					{
						float fltCoeffs[4] =
						{
							getComponent(coeffs[0], i),
							getComponent(coeffs[1], i),
							getComponent(coeffs[2], i),
							getComponent(coeffs[3], i)
						};

						float root = roots[j];
						float value = getComponent(doubleSum, i) + getComponent(sum, i) * root + 
							evaluateCubic(root, 0.0f, 0.0f, fltCoeffs) * root * root;

						getComponent(minmax.first, i) = std::min(getComponent(minmax.first, i), value);
						getComponent(minmax.second, i) = std::max(getComponent(minmax.second, i), value);
					}
				}
			}
		}

		template <>
		void calcMinMaxIntegratedDouble(std::pair<INT32, INT32>& minmax, float start, float end, 
			const INT32& doubleSum, const INT32& sum, INT32(&coeffs)[4])
		{
			assert(false && "Not implemented");
		}
	}

	template <class T>
	const UINT32 TAnimationCurve<T>::CACHE_LOOKAHEAD = 3;

	template <class T>
	TAnimationCurve<T>::TAnimationCurve()
		:mStart(0.0f), mEnd(0.0f), mLength(0.0f)
	{
	}

	template <class T>
	TAnimationCurve<T>::TAnimationCurve(const Vector<KeyFrame>& keyframes)
		:mKeyframes(keyframes)
	{
#if BS_DEBUG_MODE
		// Ensure keyframes are sorted
		if(!keyframes.empty())
		{
			float time = keyframes[0].time;
			for (UINT32 i = 1; i < (UINT32)keyframes.size(); i++)
			{
				assert(keyframes[i].time >= time);
				time = keyframes[i].time;
			}
		}
#endif

		if (!keyframes.empty())
			mEnd = keyframes.back().time;
		else
			mEnd = 0.0f;

		mStart = 0.0f;
		mLength = mEnd;
	}

	template <class T>
	T TAnimationCurve<T>::evaluate(float time, const TCurveCache<T>& cache, bool loop) const
	{
		if (mKeyframes.empty())
			return impl::getZero<T>();

		if (Math::approxEquals(mLength, 0.0f))
			time = 0.0f;

		// Wrap time if looping
		if(loop && mLength > 0.0f)
		{
			if (time < mStart)
				time = time + (std::floor(mEnd - time) / mLength) * mLength;
			else if (time > mEnd)
				time = time - std::floor((time - mStart) / mLength) * mLength;
		}

		// If time is within cache, evaluate it directly
		if (time >= cache.cachedCurveStart && time < cache.cachedCurveEnd)
			return impl::evaluateCubic(time, cache.cachedCurveStart, cache.cachedCurveEnd, cache.cachedCubicCoefficients);

		// Clamp to start, cache constant of the first key and return
		if(time < mStart)
		{
			cache.cachedCurveStart = -std::numeric_limits<float>::infinity();
			cache.cachedCurveEnd = mStart;
			cache.cachedKey = 0;
			cache.cachedCubicCoefficients[0] = impl::getZero<T>();
			cache.cachedCubicCoefficients[1] = impl::getZero<T>();
			cache.cachedCubicCoefficients[2] = impl::getZero<T>();
			cache.cachedCubicCoefficients[3] = mKeyframes[0].value;

			return mKeyframes[0].value;
		}
		
		if(time >= mEnd) // Clamp to end, cache constant of the final key and return
		{
			UINT32 lastKey = (UINT32)mKeyframes.size() - 1;

			cache.cachedCurveStart = mEnd;
			cache.cachedCurveEnd = std::numeric_limits<float>::infinity();
			cache.cachedKey = lastKey;
			cache.cachedCubicCoefficients[0] = impl::getZero<T>();
			cache.cachedCubicCoefficients[1] = impl::getZero<T>();
			cache.cachedCubicCoefficients[2] = impl::getZero<T>();
			cache.cachedCubicCoefficients[3] = mKeyframes[lastKey].value;

			return mKeyframes[lastKey].value;
		}

		// Since our value is not in cache, search for the valid pair of keys of interpolate
		UINT32 leftKeyIdx;
		UINT32 rightKeyIdx;

		findKeys(time, cache, leftKeyIdx, rightKeyIdx);

		// Calculate cubic hermite curve coefficients so we can store them in cache
		const KeyFrame& leftKey = mKeyframes[leftKeyIdx];
		const KeyFrame& rightKey = mKeyframes[rightKeyIdx];

		cache.cachedCurveStart = leftKey.time;
		cache.cachedCurveEnd = rightKey.time;

		return impl::evaluateAndUpdateCache(leftKey, rightKey, time, cache.cachedCubicCoefficients);
	}

	template <class T>
	T TAnimationCurve<T>::evaluate(float time, bool loop) const
	{
		if (mKeyframes.empty())
			return impl::getZero<T>();

		AnimationUtility::wrapTime(time, mStart, mEnd, loop);

		UINT32 leftKeyIdx;
		UINT32 rightKeyIdx;

		findKeys(time, leftKeyIdx, rightKeyIdx);

		// Evaluate curve as hermite cubic spline
		const KeyFrame& leftKey = mKeyframes[leftKeyIdx];
		const KeyFrame& rightKey = mKeyframes[rightKeyIdx];

		if (leftKeyIdx == rightKeyIdx)
			return leftKey.value;

		return impl::evaluate(leftKey, rightKey, time);
	}

	template <class T>
	T TAnimationCurve<T>::evaluateIntegrated(float time, const TCurveIntegrationCache<T>& integrationCache) const
	{
		const auto numKeyframes = (UINT32)mKeyframes.size();
		if (numKeyframes == 0)
			return impl::getZero<T>();

		if(time < mStart)
			time = mStart;

		// Generate integration cache if required
		if(!integrationCache.segmentSums)
			buildIntegrationCache(integrationCache);

		if(numKeyframes == 1)
			return (T)(mKeyframes[0].value * (time - mKeyframes[0].time));

		UINT32 leftKeyIdx;
		UINT32 rightKeyIdx;

		findKeys(time, leftKeyIdx, rightKeyIdx);

		if(leftKeyIdx == rightKeyIdx)
			return integrationCache.segmentSums[leftKeyIdx];

		const KeyFrame& lhs = mKeyframes[leftKeyIdx];
		T(&coeffs)[4] = integrationCache.coeffs[leftKeyIdx];

		const float t = time - lhs.time;
		return integrationCache.segmentSums[leftKeyIdx] + (T)(impl::evaluateCubic(t, 0.0f, 0.0f, coeffs) * t);
	}

	template <class T>
	T TAnimationCurve<T>::evaluateIntegratedDouble(float time, const TCurveIntegrationCache<T>& integrationCache) const
	{
		const auto numKeyframes = (UINT32)mKeyframes.size();
		if (numKeyframes == 0)
			return impl::getZero<T>();

		if(time < mStart)
			time = mStart;

		// Generate integration cache if required
		if(!integrationCache.segmentSums)
			buildDoubleIntegrationCache(integrationCache);

		if(numKeyframes == 1)
		{
			float t = time - mKeyframes[0].time;
			return (T)(mKeyframes[0].value * t * t * 0.5f);
		}

		UINT32 leftKeyIdx;
		UINT32 rightKeyIdx;

		findKeys(time, leftKeyIdx, rightKeyIdx);

		const KeyFrame& lhs = mKeyframes[leftKeyIdx];
		const float t = time - lhs.time;

		const T sum = (T)(integrationCache.doubleSegmentSums[leftKeyIdx] + integrationCache.segmentSums[leftKeyIdx] * t);
		if(leftKeyIdx == rightKeyIdx)
			return sum;

		T(&coeffs)[4] = integrationCache.coeffs[leftKeyIdx];
		return sum + (T)(impl::evaluateCubic(t, 0.0f, 0.0f, coeffs) * t * t);
	}

	template <class T>
	TKeyframe<T> TAnimationCurve<T>::evaluateKey(float time, bool loop) const
	{
		if (mKeyframes.empty())
			return TKeyframe<T>();

		AnimationUtility::wrapTime(time, mStart, mEnd, loop);

		UINT32 leftKeyIdx;
		UINT32 rightKeyIdx;

		findKeys(time, leftKeyIdx, rightKeyIdx);

		const KeyFrame& leftKey = mKeyframes[leftKeyIdx];
		const KeyFrame& rightKey = mKeyframes[rightKeyIdx];

		if (leftKeyIdx == rightKeyIdx)
			return leftKey;

		return evaluateKey(leftKey, rightKey, time);
	}

	template <class T>
	void TAnimationCurve<T>::findKeys(float time, const TCurveCache<T>& animInstance, UINT32& leftKey, UINT32& rightKey) const
	{
		// Check nearby keys first if there is cached data
		if (animInstance.cachedKey != (UINT32)-1)
		{
			const KeyFrame& curKey = mKeyframes[animInstance.cachedKey];
			if (time >= curKey.time)
			{
				const UINT32 end = std::min((UINT32)mKeyframes.size(), animInstance.cachedKey + CACHE_LOOKAHEAD + 1);
				for (UINT32 i = animInstance.cachedKey + 1; i < end; i++)
				{
					const KeyFrame& nextKey = mKeyframes[i];

					if (time < nextKey.time)
					{
						leftKey = i - 1;
						rightKey = i;

						animInstance.cachedKey = leftKey;
						return;
					}
				}
			}
			else
			{
				const UINT32 start = (UINT32)std::max(0, (INT32)animInstance.cachedKey - (INT32)CACHE_LOOKAHEAD);
				for(UINT32 i = start; i < animInstance.cachedKey; i++)
				{
					const KeyFrame& prevKey = mKeyframes[i];

					if (time >= prevKey.time)
					{
						leftKey = i;
						rightKey = i + 1;

						animInstance.cachedKey = leftKey;
						return;
					}
				}
			}
		}

		// Cannot find nearby ones, search all keys
		findKeys(time, leftKey, rightKey);
		animInstance.cachedKey = leftKey;
	}

	template <class T>
	void TAnimationCurve<T>::findKeys(float time, UINT32& leftKey, UINT32& rightKey) const
	{
		INT32 start = 0;
		auto searchLength = (INT32)mKeyframes.size();
		
		while(searchLength > 0)
		{
			INT32 half = searchLength >> 1;
			INT32 mid = start + half;

			if(time < mKeyframes[mid].time)
			{
				searchLength = half;
			}
			else
			{
				start = mid + 1;
				searchLength -= (half + 1);
			}
		}

		leftKey = std::max(0, start - 1);
		rightKey = std::min(start, (INT32)mKeyframes.size() - 1);
	}

	template <class T>
	UINT32 TAnimationCurve<T>::findKey(float time)
	{
		UINT32 leftKeyIdx;
		UINT32 rightKeyIdx;

		findKeys(time, leftKeyIdx, rightKeyIdx);

		const KeyFrame& leftKey = mKeyframes[leftKeyIdx];
		const KeyFrame& rightKey = mKeyframes[rightKeyIdx];

		if (Math::abs(leftKey.time - time) <= Math::abs(rightKey.time - time))
			return leftKeyIdx;
		
		return rightKeyIdx;
	}

	template <class T>
	TKeyframe<T> TAnimationCurve<T>::evaluateKey(const KeyFrame& lhs, const KeyFrame& rhs, float time) const
	{
		return impl::evaluateKey(lhs, rhs, time);
	}

	template <class T>
	TAnimationCurve<T> TAnimationCurve<T>::split(float start, float end)
	{
		Vector<TKeyframe<T>> keyFrames;

		start = Math::clamp(start, mStart, mEnd);
		end = Math::clamp(end, mStart, mEnd);

		if (Math::approxEquals(end - start, 0.0f))
			return TAnimationCurve<T>();

		UINT32 startKeyIdx = findKey(start);
		UINT32 endKeyIdx = findKey(end);

		keyFrames.reserve(endKeyIdx - startKeyIdx + 2);

		const KeyFrame& startKey = mKeyframes[startKeyIdx];
		const KeyFrame& endKey = mKeyframes[endKeyIdx];

		if (!Math::approxEquals(startKey.time, start))
		{
			if(start > startKey.time)
			{
				if (mKeyframes.size() > (startKeyIdx + 1))
					keyFrames.push_back(evaluateKey(startKey, mKeyframes[startKeyIdx + 1], start));
				else
				{
					TKeyframe<T> keyCopy = startKey;
					keyCopy.time = start;

					keyFrames.push_back(keyCopy);
				}

				startKeyIdx++;
			}
			else
			{
				
				if (startKeyIdx > 0)
					keyFrames.push_back(evaluateKey(mKeyframes[startKeyIdx - 1], startKey , start));
				else
				{
					TKeyframe<T> keyCopy = startKey;
					keyCopy.time = start;

					keyFrames.push_back(keyCopy);
				}
			}
		}
		else
		{
			keyFrames.push_back(startKey);
			startKeyIdx++;
		}

		if(!Math::approxEquals(endKey.time, end))
		{
			if(end > endKey.time)
			{
				if (mKeyframes.size() > (endKeyIdx + 1))
					keyFrames.push_back(evaluateKey(endKey, mKeyframes[endKeyIdx + 1], end));
				else
				{
					TKeyframe<T> keyCopy = endKey;
					keyCopy.time = end;

					keyFrames.push_back(keyCopy);
				}
			}
			else
			{
				if(endKeyIdx > 0)
				{
					keyFrames.push_back(evaluateKey(mKeyframes[endKeyIdx - 1], endKey, end));
					endKeyIdx--;
				}
				else
				{
					TKeyframe<T> keyCopy = endKey;
					keyCopy.time = end;

					keyFrames.push_back(keyCopy);
				}
			}
		}

		if(startKeyIdx < (UINT32)mKeyframes.size() && endKeyIdx > startKeyIdx)
			keyFrames.insert(keyFrames.begin() + 1, mKeyframes.begin() + startKeyIdx, mKeyframes.begin() + endKeyIdx + 1);

		for (auto& entry : keyFrames)
			entry.time -= start;

		return TAnimationCurve<T>(keyFrames);
	}

	template <class T>
	void TAnimationCurve<T>::makeAdditive()
	{
		if (mKeyframes.size() < 2)
			return;

		const KeyFrame& refKey = mKeyframes[0];
		const auto numKeys = (UINT32)mKeyframes.size();

		for(UINT32 i = 1; i < numKeys; i++)
			mKeyframes[i].value = impl::getDiff(mKeyframes[i].value, refKey.value);
	}

	template <class T>
	std::pair<T, T> TAnimationCurve<T>::calculateRange() const
	{
		const auto numKeys = (UINT32)mKeyframes.size();
		if(numKeys == 0)
			return std::make_pair(impl::getZero<T>(), impl::getZero<T>());

		std::pair<T, T> output = { std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity() };
		impl::getMinMax(output, mKeyframes[0].value);

		for(UINT32 i = 1; i < numKeys; i++)
		{
			const KeyFrame& lhs = mKeyframes[i - 1];
			const KeyFrame& rhs = mKeyframes[i];

			T coeffs[4];
			impl::calculateCoeffs(lhs, rhs, lhs.time, coeffs);
			impl::calcMinMax(output, lhs.time, rhs.time, coeffs);

			T endVal = impl::evaluateCubic(rhs.time, lhs.time, 0.0f, coeffs);
			impl::getMinMax(output, endVal);
		}

		return output;
	}

	template <class T>
	std::pair<T, T> TAnimationCurve<T>::calculateRangeIntegrated(const TCurveIntegrationCache<T>& cache) const
	{
		std::pair<T, T> output = std::make_pair(impl::getZero<T>(), impl::getZero<T>());

		const auto numKeys = (UINT32)mKeyframes.size();
		if(numKeys == 0)
			return output;

		if(!cache.segmentSums)
			buildIntegrationCache(cache);

		for(UINT32 i = 1; i < numKeys; i++)
		{
			const KeyFrame& lhs = mKeyframes[i - 1];
			const KeyFrame& rhs = mKeyframes[i];

			T (&coeffs)[4] = cache.coeffs[i - 1];
			impl::calcMinMaxIntegrated(output, lhs.time, rhs.time, cache.segmentSums[i - 1], coeffs);

			float t = rhs.time - lhs.time;
			T endVal = (T)(cache.segmentSums[i - 1] + impl::evaluateCubic(t, 0.0f, 0.0f, coeffs) * t);
			impl::getMinMax(output, endVal);
		}

		return output;
	}

	template <class T>
	std::pair<T, T> TAnimationCurve<T>::calculateRangeIntegratedDouble(const TCurveIntegrationCache<T>& cache) const
	{
		std::pair<T, T> output = std::make_pair(impl::getZero<T>(), impl::getZero<T>());

		const auto numKeys = (UINT32)mKeyframes.size();
		if(numKeys == 0)
			return output;

		if(!cache.segmentSums)
			buildDoubleIntegrationCache(cache);

		for(UINT32 i = 1; i < numKeys; i++)
		{
			const KeyFrame& lhs = mKeyframes[i - 1];
			const KeyFrame& rhs = mKeyframes[i];

			T (&coeffs)[4] = cache.coeffs[i - 1];
			impl::calcMinMaxIntegratedDouble(output, lhs.time, rhs.time, cache.doubleSegmentSums[i - 1], 
				cache.segmentSums[i - 1], coeffs);

			float t = rhs.time - lhs.time;
			T endVal = (T)(cache.doubleSegmentSums[i - 1] + cache.segmentSums[i - 1] * t + 
				impl::evaluateCubic(t, 0.0f, 0.0f, coeffs) * t * t);
			impl::getMinMax(output, endVal);
		}

		return output;
	}

	template <class T>
	void TAnimationCurve<T>::buildIntegrationCache(const TCurveIntegrationCache<T>& cache) const
	{
		assert(!cache.segmentSums);

		const auto numKeyframes = (UINT32)mKeyframes.size();
		if(numKeyframes <= 1)
			return;

		cache.init(numKeyframes);
		cache.segmentSums[0] = impl::getZero<T>();

		for (UINT32 i = 1; i < numKeyframes; i++)
		{
			const TKeyframe<T>& lhs = mKeyframes[i - 1];
			const TKeyframe<T>& rhs = mKeyframes[i];

			T(&coeffs)[4] = cache.coeffs[i - 1];
			impl::calculateCoeffs(lhs, rhs, lhs.time, coeffs);
			impl::integrate(coeffs);

			// Evaluate value at the end of the segment and add to the cache (this value is the total area under
			// the segment)
			const float t = rhs.time - lhs.time;
			const T value = (T)(impl::evaluateCubic(t, 0.0f, 0.0f, coeffs) * t);
			cache.segmentSums[i] = cache.segmentSums[i - 1] + value;
		}
	}

	template <class T>
	void TAnimationCurve<T>::buildDoubleIntegrationCache(const TCurveIntegrationCache<T>& cache) const
	{
		assert(!cache.segmentSums);

		const auto numKeyframes = (UINT32)mKeyframes.size();
		if(numKeyframes <= 1)
			return;

		cache.initDouble(numKeyframes);
		cache.segmentSums[0] = impl::getZero<T>();
		cache.doubleSegmentSums[0] = impl::getZero<T>();

		for (UINT32 i = 1; i < numKeyframes; i++)
		{
			const TKeyframe<T>& lhs = mKeyframes[i - 1];
			const TKeyframe<T>& rhs = mKeyframes[i];

			T(&coeffs)[4] = cache.coeffs[i - 1];
			impl::calculateCoeffs(lhs, rhs, lhs.time, coeffs);
			impl::integrate(coeffs);

			// Evaluate value at the end of the segment and add to the cache (this value is the total area under
			// the segment)
			const float t = rhs.time - lhs.time;
			T value = (T)(impl::evaluateCubic(t, 0.0f, 0.0f, coeffs) * t);
			cache.segmentSums[i] = cache.segmentSums[i - 1] + value;

			// Double integrate the already integrated coeffs
			coeffs[0] = (T)(coeffs[0] / 5.0f);
			coeffs[1] = (T)(coeffs[1] / 4.0f);
			coeffs[2] = (T)(coeffs[2] / 3.0f);
			coeffs[3] = (T)(coeffs[3] / 2.0f);

			value = (T)(impl::evaluateCubic(t, 0.0f, 0.0f, coeffs) * t * t + cache.segmentSums[i - 1] * t);
			cache.doubleSegmentSums[i] = cache.doubleSegmentSums[i - 1] + value;
		}
	}

	template class TAnimationCurve<Vector3>;
	template class TAnimationCurve<Quaternion>;
	template class TAnimationCurve<float>;
	template class TAnimationCurve<INT32>;
}