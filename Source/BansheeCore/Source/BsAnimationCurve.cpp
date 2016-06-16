//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAnimationCurve.h"
#include "BsVector3.h"
#include "BsQuaternion.h"
#include "BsMath.h"

namespace BansheeEngine
{
	template <class T>
	const UINT32 TAnimationCurve<T>::CACHE_LOOKAHEAD = 3;

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
				assert(keyframes[i].time > time);
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
	T TAnimationCurve<T>::evaluate(const AnimationInstanceData<T>& animInstance, bool loop)
	{
		if (mKeyframes.size() == 0)
			return T();

		float time = animInstance.time;

		// Wrap time if looping
		if(loop)
		{
			if (time < mStart)
				time = time - std::floor(time / mLength) * mLength;
			else if (time > mEnd)
				time = time - std::floor(time / mLength) * mLength;
		}

		// If time is within cache, evaluate it directly
		if (time >= animInstance.cachedCurveStart && time < animInstance.cachedCurveEnd)
			return evaluateCache(animInstance);

		// Clamp to start, cache constant of the first key and return
		if(time < mStart)
		{
			animInstance.cachedCurveStart = -std::numeric_limits<float>::infinity();
			animInstance.cachedCurveEnd = mStart;
			animInstance.cachedKey = 0;
			animInstance.cachedCubicCoefficients[0] = 0.0f;
			animInstance.cachedCubicCoefficients[1] = 0.0f;
			animInstance.cachedCubicCoefficients[2] = 0.0f;
			animInstance.cachedCubicCoefficients[3] = mKeyframes[0].value;

			return mKeyframes[0].value;
		}
		
		if(time > mEnd) // Clamp to end, cache constant of the final key and return
		{
			UINT32 lastKey = (UINT32)mKeyframes.size() - 1;

			animInstance.cachedCurveStart = mEnd;
			animInstance.cachedCurveEnd = std::numeric_limits<float>::infinity();
			animInstance.cachedKey = lastKey;
			animInstance.cachedCubicCoefficients[0] = 0.0f;
			animInstance.cachedCubicCoefficients[1] = 0.0f;
			animInstance.cachedCubicCoefficients[2] = 0.0f;
			animInstance.cachedCubicCoefficients[3] = mKeyframes[lastKey].value;

			return mKeyframes[lastKey].value;
		}

		// Since our value is not in cache, search for the valid pair of keys of interpolate
		UINT32 leftKeyIdx;
		UINT32 rightKeyIdx;

		findKeys(time, animInstance, leftKeyIdx, rightKeyIdx);

		// Calculate cubic hermite curve coefficients so we can store them in cache
		const KeyFrame& leftKey = mKeyframes[leftKeyIdx];
		const KeyFrame& rightKey = mKeyframes[rightKeyIdx];

		float length = rightKey.time - leftKey.time;
		
		animInstance.cachedCurveStart = leftKey.time;
		animInstance.cachedCurveEnd = rightKey.time;
		Math::cubicHermiteCoefficients(leftKey.value, rightKey.value, leftKey.outTangent, rightKey.inTangent, length,
			animInstance.cachedCubicCoefficients);
		// TODO - Handle stepped curve - If tangents are infinite assume constant value from left key is used

		T output = evaluateCache(animInstance);

		return output;
	}

	template <class T>
	T TAnimationCurve<T>::evaluate(float time, bool loop)
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
		float t = (time - leftKey.time) / length;
		T leftTangent;
		T rightTangent; // TODO - Remove zero init for vectors/quaternions by default

		if (Math::approxEquals(t, 0.0f))
		{
			t = 0.0f;
			leftTangent = T();
			rightTangent = T();
		}
		else
		{
			// Resize tangents since we're not evaluating the curve over unit range
			leftTangent = leftKey.outTangent * length;
			rightTangent = rightKey.inTangent * length;
		}

		return Math::cubicHermite(t, leftKey.value, rightKey.value, leftTangent, rightTangent);
	}

	template <class T>
	T TAnimationCurve<T>::evaluateCache(const AnimationInstanceData<T>& animInstance)
	{
		float t = animInstance.time - animInstance.cachedCurveStart;

		const T* coeffs = animInstance.cachedCubicCoefficients;
		return t * (t * (t * coeffs[0] + coeffs[1]) + coeffs[2]) + coeffs[3];
	}

	template <class T>
	void TAnimationCurve<T>::findKeys(float time, const AnimationInstanceData<T>& animInstance, UINT32& leftKey, UINT32& rightKey)
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
	void TAnimationCurve<T>::findKeys(float time, UINT32& leftKey, UINT32& rightKey)
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
				searchLength -= half - 1;
			}
		}

		leftKey = start - 1;
		rightKey = std::min(start, (INT32)mKeyframes.size() - 1);
	}

	template class TAnimationCurve<Vector3>;
	template class TAnimationCurve<Quaternion>;
	template class TAnimationCurve<float>;
}