//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAnimationCurve.h"
#include "BsAnimationCurveRTTI.h"
#include "BsVector3.h"
#include "BsQuaternion.h"
#include "BsMath.h"

namespace BansheeEngine
{
	/** 
	 * Checks if any components of the keyframes are constant (step) functions and updates the hermite curve coefficients
	 * accordingly.
	 */
	void setStepCoefficients(const TKeyframe<float>& lhs, const TKeyframe<float>& rhs, float (&coefficients)[4])
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
		for(UINT32 i = 0; i < 3; i++)
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
		if(keyframes.size() > 0)
		{
			float time = keyframes[0].time;
			for (UINT32 i = 1; i < (UINT32)keyframes.size(); i++)
			{
				assert(keyframes[i].time >= time);
				time = keyframes[i].time;
			}
		}
#endif

		if (keyframes.size() > 0)
		{
			mStart = keyframes[0].time;
			mEnd = keyframes.back().time;
		}
		else
		{
			mStart = 0.0f;
			mEnd = 0.0f;
		}

		mLength = mEnd - mStart;
	}

	template <class T>
	T TAnimationCurve<T>::evaluate(float time, const TCurveCache<T>& cache, bool loop) const
	{
		if (mKeyframes.size() == 0)
			return T();

		// Wrap time if looping
		if(loop)
		{
			if (time < mStart)
				time = time - std::floor(time / mLength) * mLength;
			else if (time > mEnd)
				time = time - std::floor(time / mLength) * mLength;
		}

		// If time is within cache, evaluate it directly
		if (time >= cache.cachedCurveStart && time < cache.cachedCurveEnd)
			return evaluateCache(time, cache);

		// Clamp to start, cache constant of the first key and return
		if(time < mStart)
		{
			cache.cachedCurveStart = -std::numeric_limits<float>::infinity();
			cache.cachedCurveEnd = mStart;
			cache.cachedKey = 0;
			cache.cachedCubicCoefficients[0] = T();
			cache.cachedCubicCoefficients[1] = T();
			cache.cachedCubicCoefficients[2] = T();
			cache.cachedCubicCoefficients[3] = mKeyframes[0].value;

			return mKeyframes[0].value;
		}
		
		if(time > mEnd) // Clamp to end, cache constant of the final key and return
		{
			UINT32 lastKey = (UINT32)mKeyframes.size() - 1;

			cache.cachedCurveStart = mEnd;
			cache.cachedCurveEnd = std::numeric_limits<float>::infinity();
			cache.cachedKey = lastKey;
			cache.cachedCubicCoefficients[0] = T();
			cache.cachedCubicCoefficients[1] = T();
			cache.cachedCubicCoefficients[2] = T();
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

		float length = rightKey.time - leftKey.time;
		Math::cubicHermiteCoefficients(leftKey.value, rightKey.value, leftKey.outTangent, rightKey.inTangent, length,
			cache.cachedCubicCoefficients);

		setStepCoefficients(leftKey, rightKey, cache.cachedCubicCoefficients);

		T output = evaluateCache(time, cache);
		return output;
	}

	template <class T>
	T TAnimationCurve<T>::evaluate(float time, bool loop) const
	{
		if (mKeyframes.size() == 0)
			return T();

		// Clamp to start or loop
		if (time < mStart)
		{
			if (loop)
				time = time - std::floor(time / mLength) * mLength;
			else // Clamping
				time = mStart;
		}

		// Clamp to end or loop
		if (time > mEnd)
		{
			if (loop)
				time = time - std::floor(time / mLength) * mLength;
			else // Clamping
				time = mEnd;
		}

		UINT32 leftKeyIdx;
		UINT32 rightKeyIdx;

		findKeys(time, leftKeyIdx, rightKeyIdx);

		// Evaluate curve as hermite cubic spline
		const KeyFrame& leftKey = mKeyframes[leftKeyIdx];
		const KeyFrame& rightKey = mKeyframes[rightKeyIdx];

		float length = rightKey.time - leftKey.time;
		float t;
		T leftTangent;
		T rightTangent; // TODO - Remove zero init for vectors/quaternions by default

		if (Math::approxEquals(length, 0.0f))
		{
			t = 0.0f;
			leftTangent = T();
			rightTangent = T();
		}
		else
		{
			// Scale from arbitrary range to [0, 1]
			t = (time - leftKey.time) / length;
			leftTangent = leftKey.outTangent * length;
			rightTangent = rightKey.inTangent * length;
		}

		T output = Math::cubicHermite(t, leftKey.value, rightKey.value, leftTangent, rightTangent);
		setStepValue(leftKey, rightKey, output);

		return output;
	}

	template <class T>
	T TAnimationCurve<T>::evaluateCache(float time, const TCurveCache<T>& cache) const
	{
		float t = time - cache.cachedCurveStart;

		const T* coeffs = cache.cachedCubicCoefficients;
		return t * (t * (t * coeffs[0] + coeffs[1]) + coeffs[2]) + coeffs[3];
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
				UINT32 end = std::min((UINT32)mKeyframes.size(), animInstance.cachedKey + CACHE_LOOKAHEAD + 1);
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
				UINT32 start = (UINT32)std::max(0, (INT32)animInstance.cachedKey - (INT32)CACHE_LOOKAHEAD);
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
		INT32 searchLength = (INT32)mKeyframes.size();
		
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
	TKeyframe<T> TAnimationCurve<T>::evaluateKey(const KeyFrame& lhs, const KeyFrame& rhs, float time)
	{
		float length = rhs.time - lhs.time;
		float t;

		T leftTangent;
		T rightTangent; // TODO - Remove zero init for vectors/quaternions by default

		if (Math::approxEquals(length, 0.0f))
		{
			t = 0.0f;
			leftTangent = T();
			rightTangent = T();
		}
		else
		{
			// Resize tangents since we're not evaluating the curve over unit range
			t = (time - lhs.time) / length;
			leftTangent = lhs.outTangent * length;
			rightTangent = rhs.inTangent * length;
		}

		TKeyframe<T> output;
		output.time = time;
		output.value = Math::cubicHermite(t, lhs.value, rhs.value, leftTangent, rightTangent);
		output.inTangent = Math::cubicHermiteD1(t, lhs.value, rhs.value, leftTangent, rightTangent);
		
		setStepValue(lhs, rhs, output.value);
		setStepTangent(lhs, rhs, output.inTangent);

		output.outTangent = output.inTangent;

		return output;
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
			keyFrames.push_back(evaluateKey(startKey, mKeyframes[startKeyIdx + 1], start));

			if (start > startKey.time)
				startKeyIdx++;
		}
		else
		{
			keyFrames.push_back(startKey);
			startKeyIdx++;
		}

		if(!Math::approxEquals(endKey.time, end))
		{
			keyFrames.push_back(evaluateKey(endKey, mKeyframes[endKeyIdx + 1], end));

			if (end < endKey.time)
				endKeyIdx--;
		}

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
		UINT32 numKeys = (UINT32)mKeyframes.size();

		for(UINT32 i = 1; i < numKeys; i++)
			mKeyframes[i].value = getDiff(mKeyframes[i].value, refKey.value);
	}

	template class TAnimationCurve<Vector3>;
	template class TAnimationCurve<Quaternion>;
	template class TAnimationCurve<float>;
}