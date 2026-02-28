//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/B3DAnimationCurve.h"
#include "RTTI/B3DAnimationCurveRTTI.h"
#include "Math/B3DVector3.h"
#include "Math/B3DVector2.h"
#include "Math/B3DQuaternion.h"
#include "Math/B3DMath.h"
#include "Animation/B3DAnimationUtility.h"

using namespace b3d;

namespace b3d
{
/**
 * Checks if any components of the keyframes are constant (step) functions and updates the hermite curve coefficients
 * accordingly.
 */
static void SetStepCoefficients(const TKeyframe<float>& lhs, const TKeyframe<float>& rhs, float (&outCoefficients)[4])
{
	if(lhs.OutTangent != std::numeric_limits<float>::infinity() &&
	   rhs.InTangent != std::numeric_limits<float>::infinity())
		return;

	outCoefficients[0] = 0.0f;
	outCoefficients[1] = 0.0f;
	outCoefficients[2] = 0.0f;
	outCoefficients[3] = lhs.Value;
}

static void SetStepCoefficients(const TKeyframe<Vector3>& lhs, const TKeyframe<Vector3>& rhs, Vector3 (&outCoefficients)[4])
{
	for(u32 componentIndex = 0; componentIndex < 3; componentIndex++)
	{
		if(lhs.OutTangent[componentIndex] != std::numeric_limits<float>::infinity() &&
		   rhs.InTangent[componentIndex] != std::numeric_limits<float>::infinity())
			continue;

		outCoefficients[0][componentIndex] = 0.0f;
		outCoefficients[1][componentIndex] = 0.0f;
		outCoefficients[2][componentIndex] = 0.0f;
		outCoefficients[3][componentIndex] = lhs.Value[componentIndex];
	}
}

static void SetStepCoefficients(const TKeyframe<Vector2>& lhs, const TKeyframe<Vector2>& rhs, Vector2 (&outCoefficients)[4])
{
	for(u32 componentIndex = 0; componentIndex < 2; componentIndex++)
	{
		if(lhs.OutTangent[componentIndex] != std::numeric_limits<float>::infinity() &&
		   rhs.InTangent[componentIndex] != std::numeric_limits<float>::infinity())
			continue;

		outCoefficients[0][componentIndex] = 0.0f;
		outCoefficients[1][componentIndex] = 0.0f;
		outCoefficients[2][componentIndex] = 0.0f;
		outCoefficients[3][componentIndex] = lhs.Value[componentIndex];
	}
}

static void SetStepCoefficients(const TKeyframe<Quaternion>& lhs, const TKeyframe<Quaternion>& rhs, Quaternion (&outCoefficients)[4])
{
	for(u32 componentIndex = 0; componentIndex < 4; componentIndex++)
	{
		if(lhs.OutTangent[componentIndex] != std::numeric_limits<float>::infinity() &&
		   rhs.InTangent[componentIndex] != std::numeric_limits<float>::infinity())
			continue;

		outCoefficients[0][componentIndex] = 0.0f;
		outCoefficients[1][componentIndex] = 0.0f;
		outCoefficients[2][componentIndex] = 0.0f;
		outCoefficients[3][componentIndex] = lhs.Value[componentIndex];
	}
}

/** Checks if any components of the keyframes are constant (step) functions and updates the key value. */
static void SetStepValue(const TKeyframe<float>& lhs, const TKeyframe<float>& rhs, float& value)
{
	if(lhs.OutTangent != std::numeric_limits<float>::infinity() &&
	   rhs.InTangent != std::numeric_limits<float>::infinity())
		return;

	value = lhs.Value;
}

static void SetStepValue(const TKeyframe<Vector3>& lhs, const TKeyframe<Vector3>& rhs, Vector3& value)
{
	for(u32 i = 0; i < 3; i++)
	{
		if(lhs.OutTangent[i] != std::numeric_limits<float>::infinity() &&
		   rhs.InTangent[i] != std::numeric_limits<float>::infinity())
			continue;

		value[i] = lhs.Value[i];
	}
}

static void SetStepValue(const TKeyframe<Vector2>& lhs, const TKeyframe<Vector2>& rhs, Vector2& value)
{
	for(u32 i = 0; i < 2; i++)
	{
		if(lhs.OutTangent[i] != std::numeric_limits<float>::infinity() &&
		   rhs.InTangent[i] != std::numeric_limits<float>::infinity())
			continue;

		value[i] = lhs.Value[i];
	}
}

static void SetStepValue(const TKeyframe<Quaternion>& lhs, const TKeyframe<Quaternion>& rhs, Quaternion& value)
{
	for(u32 i = 0; i < 4; i++)
	{
		if(lhs.OutTangent[i] != std::numeric_limits<float>::infinity() &&
		   rhs.InTangent[i] != std::numeric_limits<float>::infinity())
			continue;

		value[i] = lhs.Value[i];
	}
}

/** Checks if any components of the keyframes are constant (step) functions and updates the key tangent. */
static void SetStepTangent(const TKeyframe<float>& lhs, const TKeyframe<float>& rhs, float& tangent)
{
	if(lhs.OutTangent != std::numeric_limits<float>::infinity() &&
	   rhs.InTangent != std::numeric_limits<float>::infinity())
		return;

	tangent = std::numeric_limits<float>::infinity();
}

static void SetStepTangent(const TKeyframe<Vector3>& lhs, const TKeyframe<Vector3>& rhs, Vector3& tangent)
{
	for(u32 i = 0; i < 3; i++)
	{
		if(lhs.OutTangent[i] != std::numeric_limits<float>::infinity() &&
		   rhs.InTangent[i] != std::numeric_limits<float>::infinity())
			continue;

		tangent[i] = std::numeric_limits<float>::infinity();
	}
}

static void SetStepTangent(const TKeyframe<Vector2>& lhs, const TKeyframe<Vector2>& rhs, Vector2& tangent)
{
	for(u32 i = 0; i < 2; i++)
	{
		if(lhs.OutTangent[i] != std::numeric_limits<float>::infinity() &&
		   rhs.InTangent[i] != std::numeric_limits<float>::infinity())
			continue;

		tangent[i] = std::numeric_limits<float>::infinity();
	}
}

static void SetStepTangent(const TKeyframe<Quaternion>& lhs, const TKeyframe<Quaternion>& rhs, Quaternion& tangent)
{
	for(u32 i = 0; i < 4; i++)
	{
		if(lhs.OutTangent[i] != std::numeric_limits<float>::infinity() &&
		   rhs.InTangent[i] != std::numeric_limits<float>::infinity())
			continue;

		tangent[i] = std::numeric_limits<float>::infinity();
	}
}

/** Calculates the difference between two values. */
static float GetDiff(float lhs, float rhs)
{
	return lhs - rhs;
}

static Vector3 GetDiff(const Vector3& lhs, const Vector3& rhs)
{
	return lhs - rhs;
}

static Vector2 GetDiff(const Vector2& lhs, const Vector2& rhs)
{
	return lhs - rhs;
}

static Quaternion GetDiff(const Quaternion& lhs, const Quaternion& rhs)
{
	return rhs.Inverse() * lhs;
}

static i32 GetDiff(i32 lhs, i32 rhs)
{
	return lhs - rhs;
}

template <class T>
static T GetZero()
{
	return 0.0f;
}

template <>
static float GetZero<float>()
{
	return 0.0f;
}

template <>
static Vector3 GetZero<Vector3>()
{
	return Vector3(kZeroTag);
}

template <>
static Vector2 GetZero<Vector2>()
{
	return Vector2(kZeroTag);
}

template <>
static Quaternion GetZero<Quaternion>()
{
	return Quaternion(kZeroTag);
}

template <>
static i32 GetZero<i32>()
{
	return 0;
}

template <class T>
static constexpr u32 GetNumComponents()
{
	return 1;
}

template <>
static constexpr u32 GetNumComponents<Vector3>()
{
	return 3;
}

template <>
static constexpr u32 GetNumComponents<Vector2>()
{
	return 2;
}

template <>
static constexpr u32 GetNumComponents<Quaternion>()
{
	return 4;
}

template <class T>
static float& GetComponent(T& val, u32 idx)
{
	return val;
}

template <>
static float& GetComponent(Vector3& val, u32 idx)
{
	return val[idx];
}

template <>
static float& GetComponent(Vector2& val, u32 idx)
{
	return val[idx];
}

template <>
static float& GetComponent(Quaternion& val, u32 idx)
{
	return val[idx];
}

template <class T>
static float GetComponent(const T& val, u32 idx)
{
	return val;
}

template <>
static float GetComponent(const Vector3& val, u32 idx)
{
	return val[idx];
}

template <>
static float GetComponent(const Vector2& val, u32 idx)
{
	return val[idx];
}

template <>
static float GetComponent(const Quaternion& val, u32 idx)
{
	return val[idx];
}

template <class T>
static void GetMinMax(std::pair<T, T>& minmax, const T& value)
{
	minmax.first = std::min(minmax.first, value);
	minmax.second = std::max(minmax.second, value);
}

template <>
static void GetMinMax(std::pair<Vector3, Vector3>& minmax, const Vector3& value)
{
	minmax.first = Vector3::Min(minmax.first, value);
	minmax.second = Vector3::Max(minmax.second, value);
}

template <>
static void GetMinMax(std::pair<Vector2, Vector2>& minmax, const Vector2& value)
{
	minmax.first = Vector2::Min(minmax.first, value);
	minmax.second = Vector2::Max(minmax.second, value);
}

template <>
static void GetMinMax(std::pair<Quaternion, Quaternion>& minmax, const Quaternion& value)
{
	minmax.first = Quaternion::Min(minmax.first, value);
	minmax.second = Quaternion::Max(minmax.second, value);
}

template <class T>
static TKeyframe<T> EvaluateKey(const TKeyframe<T>& lhs, const TKeyframe<T>& rhs, float time)
{
	float length = rhs.Time - lhs.Time;

	if(Math::ApproxEquals(length, 0.0f))
		return lhs;

	// Resize tangents since we're not evaluating the curve over unit range
	float invLength = 1.0f / length;
	float t = (time - lhs.Time) * invLength;
	T leftTangent = lhs.OutTangent * length;
	T rightTangent = rhs.InTangent * length;

	TKeyframe<T> output;
	output.Time = time;
	output.Value = Math::CubicHermite(t, lhs.Value, rhs.Value, leftTangent, rightTangent);
	output.InTangent = Math::CubicHermiteD1(t, lhs.Value, rhs.Value, leftTangent, rightTangent) * invLength;

	SetStepValue(lhs, rhs, output.Value);
	SetStepTangent(lhs, rhs, output.InTangent);

	output.OutTangent = output.InTangent;

	return output;
}

template <>
static TKeyframe<i32> EvaluateKey(const TKeyframe<i32>& lhs, const TKeyframe<i32>& rhs, float time)
{
	TKeyframe<i32> output;
	output.Time = time;
	output.Value = time >= rhs.Time ? rhs.Value : lhs.Value;

	return output;
}

template <class T>
static T EvaluateCubic(float time, float start, float end, T (&coeffs)[4])
{
	float t = time - start;
	return t * (t * (t * coeffs[0] + coeffs[1]) + coeffs[2]) + coeffs[3];
}

template <>
static i32 EvaluateCubic(float time, float start, float end, i32 (&coeffs)[4])
{
	return time >= end ? coeffs[1] : coeffs[0];
}

template <class T>
static void CalculateCoeffs(const TKeyframe<T>& lhs, const TKeyframe<T>& rhs, float time, T (&coeffs)[4])
{
	float length = rhs.Time - lhs.Time;

	// Handle the case where both keys are identical, or close enough to cause precision issues
	if(length < 0.000001f)
	{
		coeffs[0] = GetZero<T>();
		coeffs[1] = GetZero<T>();
		coeffs[2] = GetZero<T>();
		coeffs[3] = lhs.Value;
	}
	else
		Math::CubicHermiteCoefficients(lhs.Value, rhs.Value, lhs.OutTangent, rhs.InTangent, length, coeffs);

	SetStepCoefficients(lhs, rhs, coeffs);
}

template <>
static void CalculateCoeffs(const TKeyframe<i32>& lhs, const TKeyframe<i32>& rhs, float time, i32 (&coeffs)[4])
{
	coeffs[0] = lhs.Value;
	coeffs[1] = rhs.Value;
}

template <class T>
static T EvaluateAndUpdateCache(const TKeyframe<T>& lhs, const TKeyframe<T>& rhs, float time, T (&coeffs)[4])
{
	CalculateCoeffs(lhs, rhs, time, coeffs);

	return EvaluateCubic(time, lhs.Time, rhs.Time, coeffs);
}

template <>
static i32 EvaluateAndUpdateCache(const TKeyframe<i32>& lhs, const TKeyframe<i32>& rhs, float time, i32 (&coeffs)[4])
{
	coeffs[0] = lhs.Value;
	coeffs[1] = rhs.Value;

	return time >= rhs.Time ? rhs.Value : lhs.Value;
}

template <class T>
static T Evaluate(const TKeyframe<T>& lhs, const TKeyframe<T>& rhs, float time)
{
	float length = rhs.Time - lhs.Time;
	B3D_ASSERT(length > 0.0f);

	float t;
	T leftTangent;
	T rightTangent;

	if(Math::ApproxEquals(length, 0.0f))
	{
		t = 0.0f;
		leftTangent = GetZero<T>();
		rightTangent = GetZero<T>();
	}
	else
	{
		// Scale from arbitrary range to [0, 1]
		t = (time - lhs.Time) / length;
		leftTangent = lhs.OutTangent * length;
		rightTangent = rhs.InTangent * length;
	}

	T output = Math::CubicHermite(t, lhs.Value, rhs.Value, leftTangent, rightTangent);
	SetStepValue(lhs, rhs, output);

	return output;
}

template <>
static i32 Evaluate(const TKeyframe<i32>& lhs, const TKeyframe<i32>& rhs, float time)
{
	return time >= rhs.Time ? rhs.Value : lhs.Value;
}

template <class T>
static void Integrate(T (&coeffs)[4])
{
	coeffs[0] = (T)(coeffs[0] / 4.0f);
	coeffs[1] = (T)(coeffs[1] / 3.0f);
	coeffs[2] = (T)(coeffs[2] / 2.0f);
}

template <class T>
static void CalcMinMax(std::pair<T, T>& minmax, float start, float end, T (&coeffs)[4])
{
	// Differentiate
	T a = (T)(3.0f * coeffs[0]);
	T b = (T)(2.0f * coeffs[1]);
	T c = (T)(1.0f * coeffs[2]);

	const u32 numComponents = GetNumComponents<T>();

	for(u32 i = 0; i < numComponents; i++)
	{
		float roots[2];
		const u32 numRoots = Math::SolveQuadratic(
			GetComponent(a, i),
			GetComponent(b, i),
			GetComponent(c, i),
			roots);

		for(u32 j = 0; j < numRoots; j++)
		{
			if((roots[j] >= 0.0f) && ((start + roots[j]) < end))
			{
				float fltCoeffs[4] = {
					GetComponent(coeffs[0], i),
					GetComponent(coeffs[1], i),
					GetComponent(coeffs[2], i),
					GetComponent(coeffs[3], i)
				};

				float value = EvaluateCubic(roots[j], 0.0f, 0.0f, fltCoeffs);

				GetComponent(minmax.first, i) = std::min(GetComponent(minmax.first, i), value);
				GetComponent(minmax.second, i) = std::max(GetComponent(minmax.second, i), value);
			}
		}
	}
}

template <>
static void CalcMinMax(std::pair<i32, i32>& minmax, float start, float end, i32 (&coeffs)[4])
{
	GetMinMax(minmax, coeffs[0]);
	GetMinMax(minmax, coeffs[1]);
}

template <class T>
static void CalcMinMaxIntegrated(std::pair<T, T>& minmax, float start, float end, const T& sum, T (&coeffs)[4])
{
	// Differentiate
	T a = 4.0f * coeffs[0];
	T b = 3.0f * coeffs[1];
	T c = 2.0f * coeffs[2];
	T d = 1.0f * coeffs[3];

	const u32 numComponents = GetNumComponents<T>();

	for(u32 i = 0; i < numComponents; i++)
	{
		float roots[3];
		const u32 numRoots = Math::SolveCubic(
			GetComponent(a, i),
			GetComponent(b, i),
			GetComponent(c, i),
			GetComponent(d, i),
			roots);

		for(u32 j = 0; j < numRoots; j++)
		{
			if((roots[j] >= 0.0f) && ((start + roots[j]) < end))
			{
				float fltCoeffs[4] = {
					GetComponent(coeffs[0], i),
					GetComponent(coeffs[1], i),
					GetComponent(coeffs[2], i),
					GetComponent(coeffs[3], i)
				};

				float value = GetComponent(sum, i) + EvaluateCubic(roots[j], 0.0f, 0.0f, fltCoeffs) * roots[j];

				GetComponent(minmax.first, i) = std::min(GetComponent(minmax.first, i), value);
				GetComponent(minmax.second, i) = std::max(GetComponent(minmax.second, i), value);
			}
		}
	}
}

template <>
static void CalcMinMaxIntegrated(std::pair<i32, i32>& minmax, float start, float end, const i32& sum, i32 (&coeffs)[4])
{
	B3D_ASSERT(false && "Not implemented");
}

template <class T>
static void CalcMinMaxIntegratedDouble(std::pair<T, T>& minmax, float start, float end, const T& doubleSum, const T& sum, T (&coeffs)[4])
{
	// Differentiate
	T a = 5.0f * coeffs[0];
	T b = 4.0f * coeffs[1];
	T c = 3.0f * coeffs[2];
	T d = 2.0f * coeffs[3];

	const u32 numComponents = GetNumComponents<T>();

	for(u32 i = 0; i < numComponents; i++)
	{
		float roots[4];
		const u32 numRoots = Math::SolveQuartic(
			GetComponent(a, i),
			GetComponent(b, i),
			GetComponent(c, i),
			GetComponent(d, i),
			0.0f,
			roots);

		for(u32 j = 0; j < numRoots; j++)
		{
			if((roots[j] >= 0.0f) && ((start + roots[j]) < end))
			{
				float fltCoeffs[4] = {
					GetComponent(coeffs[0], i),
					GetComponent(coeffs[1], i),
					GetComponent(coeffs[2], i),
					GetComponent(coeffs[3], i)
				};

				float root = roots[j];
				float value = GetComponent(doubleSum, i) + GetComponent(sum, i) * root +
					EvaluateCubic(root, 0.0f, 0.0f, fltCoeffs) * root * root;

				GetComponent(minmax.first, i) = std::min(GetComponent(minmax.first, i), value);
				GetComponent(minmax.second, i) = std::max(GetComponent(minmax.second, i), value);
			}
		}
	}
}

template <>
static void CalcMinMaxIntegratedDouble(std::pair<i32, i32>& minmax, float start, float end, const i32& doubleSum, const i32& sum, i32 (&coeffs)[4])
{
	B3D_ASSERT(false && "Not implemented");
}
} // namespace b3d

template <class T>
const u32 TAnimationCurve<T>::kCacheLookahead = 3;

template <class T>
TAnimationCurve<T>::TAnimationCurve(const Vector<KeyFrame>& keyframes)
	: mKeyframes(keyframes)
{
#if B3D_DEBUG
	// Ensure keyframes are sorted
	if(!keyframes.empty())
	{
		float previousTime = keyframes[0].Time;
		for(u32 i = 1; i < (u32)keyframes.size(); i++)
		{
			B3D_ASSERT(keyframes[i].Time >= previousTime);
			previousTime = keyframes[i].Time;
		}
	}
#endif

	if(!keyframes.empty())
		mEnd = keyframes.back().Time;
	else
		mEnd = 0.0f;

	mStart = 0.0f;
	mLength = mEnd;
}

template <class T>
T TAnimationCurve<T>::Evaluate(float time, const TCurveCache<T>& cache, bool loop) const
{
	if(mKeyframes.empty())
		return GetZero<T>();

	if(Math::ApproxEquals(mLength, 0.0f))
		time = 0.0f;

	// Wrap time if looping
	if(loop && mLength > 0.0f)
	{
		if(time < mStart)
			time = time + (std::floor(mEnd - time) / mLength) * mLength;
		else if(time > mEnd)
			time = time - std::floor((time - mStart) / mLength) * mLength;
	}

	// If time is within cache, evaluate it directly
	if(time >= cache.cachedCurveStart && time < cache.cachedCurveEnd)
		return EvaluateCubic(time, cache.cachedCurveStart, cache.cachedCurveEnd, cache.cachedCubicCoefficients);

	// Clamp to start, cache constant of the first key and return
	if(time < mStart)
	{
		cache.cachedCurveStart = -std::numeric_limits<float>::infinity();
		cache.cachedCurveEnd = mStart;
		cache.cachedKey = 0;
		cache.cachedCubicCoefficients[0] = GetZero<T>();
		cache.cachedCubicCoefficients[1] = GetZero<T>();
		cache.cachedCubicCoefficients[2] = GetZero<T>();
		cache.cachedCubicCoefficients[3] = mKeyframes[0].Value;

		return mKeyframes[0].Value;
	}

	if(time >= mEnd) // Clamp to end, cache constant of the final key and return
	{
		u32 lastKey = (u32)mKeyframes.size() - 1;

		cache.cachedCurveStart = mEnd;
		cache.cachedCurveEnd = std::numeric_limits<float>::infinity();
		cache.cachedKey = lastKey;
		cache.cachedCubicCoefficients[0] = GetZero<T>();
		cache.cachedCubicCoefficients[1] = GetZero<T>();
		cache.cachedCubicCoefficients[2] = GetZero<T>();
		cache.cachedCubicCoefficients[3] = mKeyframes[lastKey].Value;

		return mKeyframes[lastKey].Value;
	}

	// Since our value is not in cache, search for the valid pair of keys of interpolate
	u32 leftKeyIdx;
	u32 rightKeyIdx;

	FindKeys(time, cache, leftKeyIdx, rightKeyIdx);

	// Calculate cubic hermite curve coefficients so we can store them in cache
	const KeyFrame& leftKey = mKeyframes[leftKeyIdx];
	const KeyFrame& rightKey = mKeyframes[rightKeyIdx];

	cache.cachedCurveStart = leftKey.Time;
	cache.cachedCurveEnd = rightKey.Time;

	return EvaluateAndUpdateCache(leftKey, rightKey, time, cache.cachedCubicCoefficients);
}

template <class T>
T TAnimationCurve<T>::Evaluate(float time, bool loop) const
{
	if(mKeyframes.empty())
		return GetZero<T>();

	AnimationUtility::WrapTime(time, mStart, mEnd, loop);

	u32 leftKeyIdx;
	u32 rightKeyIdx;

	FindKeys(time, leftKeyIdx, rightKeyIdx);

	// Evaluate curve as hermite cubic spline
	const KeyFrame& leftKey = mKeyframes[leftKeyIdx];
	const KeyFrame& rightKey = mKeyframes[rightKeyIdx];

	if(leftKeyIdx == rightKeyIdx)
		return leftKey.Value;

	return ::Evaluate(leftKey, rightKey, time);
}

template <class T>
T TAnimationCurve<T>::EvaluateIntegrated(float time, const TCurveIntegrationCache<T>& integrationCache) const
{
	const auto numKeyframes = (u32)mKeyframes.size();
	if(numKeyframes == 0)
		return GetZero<T>();

	if(time < mStart)
		time = mStart;

	// Generate integration cache if required
	if(!integrationCache.segmentSums)
		BuildIntegrationCache(integrationCache);

	if(numKeyframes == 1)
		return (T)(mKeyframes[0].Value * (time - mKeyframes[0].Time));

	u32 leftKeyIdx;
	u32 rightKeyIdx;

	FindKeys(time, leftKeyIdx, rightKeyIdx);

	if(leftKeyIdx == rightKeyIdx)
		return integrationCache.segmentSums[leftKeyIdx];

	const KeyFrame& lhs = mKeyframes[leftKeyIdx];
	T(&coeffs)
	[4] = integrationCache.coeffs[leftKeyIdx];

	const float t = time - lhs.Time;
	return integrationCache.segmentSums[leftKeyIdx] + (T)(EvaluateCubic(t, 0.0f, 0.0f, coeffs) * t);
}

template <class T>
T TAnimationCurve<T>::EvaluateIntegratedDouble(float time, const TCurveIntegrationCache<T>& integrationCache) const
{
	const auto numKeyframes = (u32)mKeyframes.size();
	if(numKeyframes == 0)
		return GetZero<T>();

	if(time < mStart)
		time = mStart;

	// Generate integration cache if required
	if(!integrationCache.segmentSums)
		BuildDoubleIntegrationCache(integrationCache);

	if(numKeyframes == 1)
	{
		float t = time - mKeyframes[0].Time;
		return (T)(mKeyframes[0].Value * t * t * 0.5f);
	}

	u32 leftKeyIdx;
	u32 rightKeyIdx;

	FindKeys(time, leftKeyIdx, rightKeyIdx);

	const KeyFrame& lhs = mKeyframes[leftKeyIdx];
	const float t = time - lhs.Time;

	const T sum = (T)(integrationCache.doubleSegmentSums[leftKeyIdx] + integrationCache.segmentSums[leftKeyIdx] * t);
	if(leftKeyIdx == rightKeyIdx)
		return sum;

	T(&coeffs)
	[4] = integrationCache.coeffs[leftKeyIdx];
	return sum + (T)(EvaluateCubic(t, 0.0f, 0.0f, coeffs) * t * t);
}

template <class T>
TKeyframe<T> TAnimationCurve<T>::EvaluateKey(float time, bool loop) const
{
	if(mKeyframes.empty())
		return TKeyframe<T>();

	AnimationUtility::WrapTime(time, mStart, mEnd, loop);

	u32 leftKeyIdx;
	u32 rightKeyIdx;

	FindKeys(time, leftKeyIdx, rightKeyIdx);

	const KeyFrame& leftKey = mKeyframes[leftKeyIdx];
	const KeyFrame& rightKey = mKeyframes[rightKeyIdx];

	if(leftKeyIdx == rightKeyIdx)
		return leftKey;

	return EvaluateKey(leftKey, rightKey, time);
}

template <class T>
void TAnimationCurve<T>::FindKeys(float time, const TCurveCache<T>& animInstance, u32& outLeftKey, u32& outRightKey) const
{
	// Check nearby keys first if there is cached data
	if(animInstance.cachedKey != (u32)-1)
	{
		const KeyFrame& curKey = mKeyframes[animInstance.cachedKey];
		if(time >= curKey.Time)
		{
			const u32 end = std::min((u32)mKeyframes.size(), animInstance.cachedKey + kCacheLookahead + 1);
			for(u32 keyIndex = animInstance.cachedKey + 1; keyIndex < end; keyIndex++)
			{
				const KeyFrame& nextKey = mKeyframes[keyIndex];

				if(time < nextKey.Time)
				{
					outLeftKey = keyIndex - 1;
					outRightKey = keyIndex;

					animInstance.cachedKey = outLeftKey;
					return;
				}
			}
		}
		else
		{
			const u32 start = (u32)std::max(0, (i32)animInstance.cachedKey - (i32)kCacheLookahead);
			for(u32 keyIndex = start; keyIndex < animInstance.cachedKey; keyIndex++)
			{
				const KeyFrame& prevKey = mKeyframes[keyIndex];

				if(time >= prevKey.Time)
				{
					outLeftKey = keyIndex;
					outRightKey = keyIndex + 1;

					animInstance.cachedKey = outLeftKey;
					return;
				}
			}
		}
	}

	// Cannot find nearby ones, search all keys
	FindKeys(time, outLeftKey, outRightKey);
	animInstance.cachedKey = outLeftKey;
}

template <class T>
void TAnimationCurve<T>::FindKeys(float time, u32& outLeftKey, u32& outRightKey) const
{
	i32 start = 0;
	auto searchLength = (i32)mKeyframes.size();

	while(searchLength > 0)
	{
		i32 half = searchLength >> 1;
		i32 mid = start + half;

		if(time < mKeyframes[mid].Time)
		{
			searchLength = half;
		}
		else
		{
			start = mid + 1;
			searchLength -= (half + 1);
		}
	}

	outLeftKey = std::max(0, start - 1);
	outRightKey = std::min(start, (i32)mKeyframes.size() - 1);
}

template <class T>
u32 TAnimationCurve<T>::FindKey(float time)
{
	u32 leftKeyIdx;
	u32 rightKeyIdx;

	FindKeys(time, leftKeyIdx, rightKeyIdx);

	const KeyFrame& leftKey = mKeyframes[leftKeyIdx];
	const KeyFrame& rightKey = mKeyframes[rightKeyIdx];

	if(Math::Abs(leftKey.Time - time) <= Math::Abs(rightKey.Time - time))
		return leftKeyIdx;

	return rightKeyIdx;
}

template <class T>
TKeyframe<T> TAnimationCurve<T>::EvaluateKey(const KeyFrame& lhs, const KeyFrame& rhs, float time) const
{
	return b3d::EvaluateKey(lhs, rhs, time);
}

template <class T>
TAnimationCurve<T> TAnimationCurve<T>::Split(float start, float end)
{
	Vector<TKeyframe<T>> keyFrames;

	start = Math::Clamp(start, mStart, mEnd);
	end = Math::Clamp(end, mStart, mEnd);

	u32 startKeyIdx = FindKey(start);
	u32 endKeyIdx = FindKey(end);

	keyFrames.reserve(endKeyIdx - startKeyIdx + 2);

	const KeyFrame& startKey = mKeyframes[startKeyIdx];

	if(!Math::ApproxEquals(startKey.Time, start))
	{
		if(start > startKey.Time)
		{
			if(mKeyframes.size() > (startKeyIdx + 1))
				keyFrames.push_back(EvaluateKey(startKey, mKeyframes[startKeyIdx + 1], start));
			else
			{
				TKeyframe<T> keyCopy = startKey;
				keyCopy.Time = start;

				keyFrames.push_back(keyCopy);
			}

			startKeyIdx++;
		}
		else
		{

			if(startKeyIdx > 0)
				keyFrames.push_back(EvaluateKey(mKeyframes[startKeyIdx - 1], startKey, start));
			else
			{
				TKeyframe<T> keyCopy = startKey;
				keyCopy.Time = start;

				keyFrames.push_back(keyCopy);
			}
		}
	}
	else
	{
		keyFrames.push_back(startKey);
		startKeyIdx++;
	}

	if(!Math::ApproxEquals(end - start, 0.0f))
	{
		const KeyFrame& endKey = mKeyframes[endKeyIdx];
		if(!Math::ApproxEquals(endKey.Time, end))
		{
			if(end > endKey.Time)
			{
				if(mKeyframes.size() > (endKeyIdx + 1))
					keyFrames.push_back(EvaluateKey(endKey, mKeyframes[endKeyIdx + 1], end));
				else
				{
					TKeyframe<T> keyCopy = endKey;
					keyCopy.Time = end;

					keyFrames.push_back(keyCopy);
				}
			}
			else
			{
				if(endKeyIdx > 0)
				{
					keyFrames.push_back(EvaluateKey(mKeyframes[endKeyIdx - 1], endKey, end));
					endKeyIdx--;
				}
				else
				{
					TKeyframe<T> keyCopy = endKey;
					keyCopy.Time = end;

					keyFrames.push_back(keyCopy);
				}
			}
		}

		if(startKeyIdx < (u32)mKeyframes.size() && endKeyIdx > startKeyIdx)
			keyFrames.insert(keyFrames.begin() + 1, mKeyframes.begin() + startKeyIdx, mKeyframes.begin() + endKeyIdx + 1);
	}

	for(auto& entry : keyFrames)
		entry.Time -= start;

	return TAnimationCurve<T>(keyFrames);
}

template <class T>
void TAnimationCurve<T>::MakeAdditive()
{
	if(mKeyframes.size() < 2)
		return;

	const KeyFrame& refKey = mKeyframes[0];
	const auto numKeys = (u32)mKeyframes.size();

	for(u32 i = 1; i < numKeys; i++)
		mKeyframes[i].Value = GetDiff(mKeyframes[i].Value, refKey.Value);
}

template <class T>
std::pair<float, float> TAnimationCurve<T>::GetTimeRange() const
{
	if(mKeyframes.empty())
		return std::make_pair(0.0f, 0.0f);

	if(mKeyframes.size() == 1)
		return std::make_pair(mKeyframes[0].Time, mKeyframes[0].Time);

	return std::make_pair(mKeyframes[0].Time, mKeyframes[mKeyframes.size() - 1].Time);
}

template <class T>
std::pair<T, T> TAnimationCurve<T>::CalculateRange() const
{
	const auto numKeys = (u32)mKeyframes.size();
	if(numKeys == 0)
		return std::make_pair(GetZero<T>(), GetZero<T>());

	std::pair<T, T> output = { std::numeric_limits<T>::infinity(), -std::numeric_limits<T>::infinity() };
	GetMinMax(output, mKeyframes[0].Value);

	for(u32 i = 1; i < numKeys; i++)
	{
		const KeyFrame& lhs = mKeyframes[i - 1];
		const KeyFrame& rhs = mKeyframes[i];

		T coeffs[4];
		CalculateCoeffs(lhs, rhs, lhs.Time, coeffs);
		CalcMinMax(output, lhs.Time, rhs.Time, coeffs);

		T endVal = EvaluateCubic(rhs.Time, lhs.Time, 0.0f, coeffs);
		GetMinMax(output, endVal);
	}

	return output;
}

template <class T>
std::pair<T, T> TAnimationCurve<T>::CalculateRangeIntegrated(const TCurveIntegrationCache<T>& cache) const
{
	std::pair<T, T> output = std::make_pair(GetZero<T>(), GetZero<T>());

	const auto numKeys = (u32)mKeyframes.size();
	if(numKeys == 0)
		return output;

	if(!cache.segmentSums)
		BuildIntegrationCache(cache);

	for(u32 i = 1; i < numKeys; i++)
	{
		const KeyFrame& lhs = mKeyframes[i - 1];
		const KeyFrame& rhs = mKeyframes[i];

		T(&coeffs)
		[4] = cache.coeffs[i - 1];
		CalcMinMaxIntegrated(output, lhs.Time, rhs.Time, cache.segmentSums[i - 1], coeffs);

		float t = rhs.Time - lhs.Time;
		T endVal = (T)(cache.segmentSums[i - 1] + EvaluateCubic(t, 0.0f, 0.0f, coeffs) * t);
		GetMinMax(output, endVal);
	}

	return output;
}

template <class T>
std::pair<T, T> TAnimationCurve<T>::CalculateRangeIntegratedDouble(const TCurveIntegrationCache<T>& cache) const
{
	std::pair<T, T> output = std::make_pair(GetZero<T>(), GetZero<T>());

	const auto numKeys = (u32)mKeyframes.size();
	if(numKeys == 0)
		return output;

	if(!cache.segmentSums)
		BuildDoubleIntegrationCache(cache);

	for(u32 i = 1; i < numKeys; i++)
	{
		const KeyFrame& lhs = mKeyframes[i - 1];
		const KeyFrame& rhs = mKeyframes[i];

		T(&coeffs)
		[4] = cache.coeffs[i - 1];
		CalcMinMaxIntegratedDouble(output, lhs.Time, rhs.Time, cache.doubleSegmentSums[i - 1], cache.segmentSums[i - 1], coeffs);

		float t = rhs.Time - lhs.Time;
		T endVal = (T)(cache.doubleSegmentSums[i - 1] + cache.segmentSums[i - 1] * t + EvaluateCubic(t, 0.0f, 0.0f, coeffs) * t * t);
		GetMinMax(output, endVal);
	}

	return output;
}

template <class T>
void TAnimationCurve<T>::BuildIntegrationCache(const TCurveIntegrationCache<T>& cache) const
{
	B3D_ASSERT(!cache.segmentSums);

	const auto numKeyframes = (u32)mKeyframes.size();
	if(numKeyframes <= 1)
		return;

	cache.Init(numKeyframes);
	cache.segmentSums[0] = GetZero<T>();

	for(u32 i = 1; i < numKeyframes; i++)
	{
		const TKeyframe<T>& lhs = mKeyframes[i - 1];
		const TKeyframe<T>& rhs = mKeyframes[i];

		T(&coeffs)
		[4] = cache.coeffs[i - 1];
		CalculateCoeffs(lhs, rhs, lhs.Time, coeffs);
		Integrate(coeffs);

		// Evaluate value at the end of the segment and add to the cache (this value is the total area under
		// the segment)
		const float t = rhs.Time - lhs.Time;
		const T value = (T)(EvaluateCubic(t, 0.0f, 0.0f, coeffs) * t);
		cache.segmentSums[i] = cache.segmentSums[i - 1] + value;
	}
}

template <class T>
void TAnimationCurve<T>::BuildDoubleIntegrationCache(const TCurveIntegrationCache<T>& cache) const
{
	B3D_ASSERT(!cache.segmentSums);

	const auto numKeyframes = (u32)mKeyframes.size();
	if(numKeyframes <= 1)
		return;

	cache.InitDouble(numKeyframes);
	cache.segmentSums[0] = GetZero<T>();
	cache.doubleSegmentSums[0] = GetZero<T>();

	for(u32 i = 1; i < numKeyframes; i++)
	{
		const TKeyframe<T>& lhs = mKeyframes[i - 1];
		const TKeyframe<T>& rhs = mKeyframes[i];

		T(&coeffs)
		[4] = cache.coeffs[i - 1];
		CalculateCoeffs(lhs, rhs, lhs.Time, coeffs);
		Integrate(coeffs);

		// Evaluate value at the end of the segment and add to the cache (this value is the total area under
		// the segment)
		const float t = rhs.Time - lhs.Time;
		T value = (T)(EvaluateCubic(t, 0.0f, 0.0f, coeffs) * t);
		cache.segmentSums[i] = cache.segmentSums[i - 1] + value;

		// Double integrate the already integrated coeffs
		coeffs[0] = (T)(coeffs[0] / 5.0f);
		coeffs[1] = (T)(coeffs[1] / 4.0f);
		coeffs[2] = (T)(coeffs[2] / 3.0f);
		coeffs[3] = (T)(coeffs[3] / 2.0f);

		value = (T)(EvaluateCubic(t, 0.0f, 0.0f, coeffs) * t * t + cache.segmentSums[i - 1] * t);
		cache.doubleSegmentSums[i] = cache.doubleSegmentSums[i - 1] + value;
	}
}

template <class T>
bool TAnimationCurve<T>::operator==(const TAnimationCurve<T>& rhs) const
{
	if(mLength != rhs.mLength || mStart != rhs.mStart || mEnd != rhs.mEnd)
		return false;

	return mKeyframes == rhs.mKeyframes;
}

template class TAnimationCurve<Vector3>;
template class TAnimationCurve<Vector2>;
template class TAnimationCurve<Quaternion>;
template class TAnimationCurve<float>;
template class TAnimationCurve<i32>;
