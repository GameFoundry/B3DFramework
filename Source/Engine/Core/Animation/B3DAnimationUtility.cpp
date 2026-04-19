//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/B3DAnimationUtility.h"
#include "Math/B3DVector3.h"
#include "Math/B3DQuaternion.h"

using namespace b3d;

void SetStepTangent(const TKeyframe<Vector3>& lhsIn, const TKeyframe<Vector3>& rhsIn, TKeyframe<Quaternion>& lhsOut, TKeyframe<Quaternion>& rhsOut)
{
	for(u32 i = 0; i < 3; i++)
	{
		if(lhsIn.OutTangent[i] != std::numeric_limits<float>::infinity() &&
		   rhsIn.InTangent[i] != std::numeric_limits<float>::infinity())
			continue;

		lhsOut.OutTangent[i] = std::numeric_limits<float>::infinity();
		rhsOut.InTangent[i] = std::numeric_limits<float>::infinity();
	}
}

void SetStepTangent(const TKeyframe<Quaternion>& lhsIn, const TKeyframe<Quaternion>& rhsIn, TKeyframe<Vector3>& lhsOut, TKeyframe<Vector3>& rhsOut)
{
	for(u32 i = 0; i < 4; i++)
	{
		if(lhsIn.OutTangent[i] != std::numeric_limits<float>::infinity() &&
		   rhsIn.InTangent[i] != std::numeric_limits<float>::infinity())
			continue;

		if(i < 3)
		{
			lhsOut.OutTangent[i] = std::numeric_limits<float>::infinity();
			rhsOut.InTangent[i] = std::numeric_limits<float>::infinity();
		}
	}
}

void AnimationUtility::WrapTime(float& time, float start, float end, bool loop)
{
	float length = end - start;

	if(Math::ApproxEquals(length, 0.0f))
	{
		time = 0.0f;
		return;
	}

	// Clamp to start or loop
	if(time < start)
	{
		if(loop)
			time = time + (std::floor(end - time) / length) * length;
		else // Clamping
			time = start;
	}

	// Clamp to end or loop
	if(time > end)
	{
		if(loop)
			time = time - std::floor((time - start) / length) * length;
		else // Clamping
			time = end;
	}
}

SPtr<TAnimationCurve<Quaternion>> AnimationUtility::EulerToQuaternionCurve(
	const SPtr<TAnimationCurve<Vector3>>& eulerCurve, EulerAngleOrder order)
{
	// TODO: We calculate tangents by sampling. There must be an analytical way to calculate tangents when converting
	// a curve.
	const float FIT_TIME = 0.001f;

	auto fnEulerToQuaternion = [&](i32 keyIndex, Vector3& angles, const Quaternion& lastQuat)
	{
		Quaternion quat(
			Degree(angles.X),
			Degree(angles.Y),
			Degree(angles.Z), order);

		// Flip quaternion in case rotation is over 180 degrees (use shortest path)
		if(keyIndex > 0)
		{
			float dot = quat.Dot(lastQuat);
			if(dot < 0.0f)
				quat = -quat;
		}

		return quat;
	};

	i32 numKeys = (i32)eulerCurve->GetNumKeyFrames();
	Vector<TKeyframe<Quaternion>> quatKeyframes;
	quatKeyframes.reserve(numKeys);

	auto fnAddKeyframe = [&quatKeyframes](float time, const Quaternion& quat)
	{
		quatKeyframes.emplace_back();
		TKeyframe<Quaternion>& keyframe = quatKeyframes.back();

		keyframe.Time = time;
		keyframe.Value = quat;
		keyframe.InTangent = Quaternion::kZero;
		keyframe.OutTangent = Quaternion::kZero;
	};

	// Calculate key values
	Quaternion lastQuat(kZeroTag);
	Vector3 lastAngles(kZeroTag);
	float lastTime = 0.0f;
	for(i32 i = 0; i < numKeys; i++)
	{
		float time = eulerCurve->GetKeyFrame(i).Time;
		Vector3 angles = eulerCurve->GetKeyFrame(i).Value;

		Vector3 diff = angles - lastAngles;
		float maxAngle = std::max(std::max(abs(diff.X), abs(diff.Y)), abs(diff.Z));

		// Is the angle greater than 180? In which case we need multiple keyframes to represent it via quaternions
		if(i > 0 && maxAngle > 180.0f)
		{
			constexpr float SPLIT_INTERVAL = 175.0f; // Not exactly 180 to ensure no precision issues
			i32 numSplits = Math::FloorToPosInt(maxAngle / SPLIT_INTERVAL) + 1;

			Vector3 partAngles = diff / (float)numSplits;
			float partTime = (time - lastTime) / numSplits;
			for(i32 j = 0; j < numSplits; j++)
			{
				Quaternion partQuat(
					Degree(partAngles.X),
					Degree(partAngles.Y),
					Degree(partAngles.Z), order);

				float curTime = (j == (numSplits - 1)) ? time : lastTime + partTime;
				Quaternion curQuat = lastQuat * partQuat;

				// Ensure rotation is not over 180 degrees
				B3D_ASSERT(curQuat.Dot(lastQuat) >= 0.0f);

				fnAddKeyframe(curTime, curQuat);
				lastTime = curTime;
				lastQuat = curQuat;
			}
		}
		else
		{
			Quaternion quat = fnEulerToQuaternion(i, angles, lastQuat);
			fnAddKeyframe(time, quat);

			lastTime = time;
			lastQuat = quat;
		}

		lastAngles = angles;
	}

	// Calculate extra values between keys so we can approximate tangents. If we're sampling very close to the key
	// the values should pretty much exactly match the tangent (assuming the curves are cubic hermite)
	i32 numQuatKeys = (i32)quatKeyframes.size();
	for(i32 i = 0; i < numQuatKeys - 1; i++)
	{
		TKeyframe<Quaternion>& currentKey = quatKeyframes[i];
		TKeyframe<Quaternion>& nextKey = quatKeyframes[i + 1];

		float dt = nextKey.Time - currentKey.Time;
		float startFitTime = currentKey.Time + dt * FIT_TIME;
		float endFitTime = currentKey.Time + dt * (1.0f - FIT_TIME);

		Vector3 anglesStart = eulerCurve->Evaluate(startFitTime, false);
		Vector3 anglesEnd = eulerCurve->Evaluate(endFitTime, false);
		Quaternion startFitValue = fnEulerToQuaternion(i, anglesStart, currentKey.Value);
		Quaternion endFitValue = fnEulerToQuaternion(i, anglesEnd, startFitValue);

		float invFitTime = 1.0f / (dt * FIT_TIME);
		currentKey.OutTangent = (startFitValue - currentKey.Value) * invFitTime;
		nextKey.InTangent = (nextKey.Value - endFitValue) * invFitTime;

		const TKeyframe<Vector3>& currentEulerKey = eulerCurve->EvaluateKey(currentKey.Time, false);
		const TKeyframe<Vector3>& nextEulerKey = eulerCurve->EvaluateKey(nextKey.Time, false);
		SetStepTangent(currentEulerKey, nextEulerKey, currentKey, nextKey);
	}

	return B3DMakeShared<TAnimationCurve<Quaternion>>(quatKeyframes);
}

SPtr<TAnimationCurve<Vector3>> AnimationUtility::QuaternionToEulerCurve(const SPtr<TAnimationCurve<Quaternion>>& quatCurve)
{
	// TODO: We calculate tangents by sampling. There must be an analytical way to calculate tangents when converting
	// a curve.
	const float FIT_TIME = 0.001f;

	auto fnQuaternionToEuler = [&](const Quaternion& quat)
	{
		Radian x, y, z;
		quat.ToEulerAngles(x, y, z);

		Vector3 euler(
			x.GetValueInDegrees(),
			y.GetValueInDegrees(),
			z.GetValueInDegrees());

		return euler;
	};

	i32 numKeys = (i32)quatCurve->GetNumKeyFrames();
	Vector<TKeyframe<Vector3>> eulerKeyframes(numKeys);

	// Calculate key values
	for(i32 i = 0; i < numKeys; i++)
	{
		float time = quatCurve->GetKeyFrame(i).Time;
		Quaternion quat = quatCurve->GetKeyFrame(i).Value;
		Vector3 euler = fnQuaternionToEuler(quat);

		eulerKeyframes[i].Time = time;
		eulerKeyframes[i].Value = euler;
		eulerKeyframes[i].InTangent = Vector3::kZero;
		eulerKeyframes[i].OutTangent = Vector3::kZero;
	}

	// Calculate extra values between keys so we can approximate tangents. If we're sampling very close to the key
	// the values should pretty much exactly match the tangent (assuming the curves are cubic hermite)
	for(i32 i = 0; i < numKeys - 1; i++)
	{
		TKeyframe<Vector3>& currentKey = eulerKeyframes[i];
		TKeyframe<Vector3>& nextKey = eulerKeyframes[i + 1];

		const TKeyframe<Quaternion>& currentQuatKey = quatCurve->GetKeyFrame(i);
		const TKeyframe<Quaternion>& nextQuatKey = quatCurve->GetKeyFrame(i + 1);

		float dt = nextKey.Time - currentKey.Time;
		float startFitTime = currentKey.Time + dt * FIT_TIME;
		float endFitTime = currentKey.Time + dt * (1.0f - FIT_TIME);

		Quaternion startQuat = Quaternion::Normalize(quatCurve->Evaluate(startFitTime, false));
		Quaternion endQuat = Quaternion::Normalize(quatCurve->Evaluate(endFitTime, false));
		Vector3 startFitValue = fnQuaternionToEuler(startQuat);
		Vector3 endFitValue = fnQuaternionToEuler(endQuat);

		// If fit values rotate for more than 180 degrees, wrap them so they use the shortest path
		for(int j = 0; j < 3; j++)
		{
			startFitValue[j] = fmod(startFitValue[j] - currentKey.Value[j] + 180.0f, 360.0f) + currentKey.Value[j] - 180.0f;
			endFitValue[j] = nextKey.Value[j] + fmod(nextKey.Value[j] - endFitValue[j] + 180.0f, 360.0f) - 180.0f;
		}

		float invFitTime = 1.0f / (dt * FIT_TIME);
		currentKey.OutTangent = (startFitValue - currentKey.Value) * invFitTime;
		nextKey.InTangent = (nextKey.Value - endFitValue) * invFitTime;

		SetStepTangent(currentQuatKey, nextQuatKey, currentKey, nextKey);
	}

	return B3DMakeShared<TAnimationCurve<Vector3>>(eulerKeyframes);
}

template <class T>
void SplitCurve(
	const TAnimationCurve<T>& compoundCurve,
	Vector<TKeyframe<float>> (&keyFrames)[TCurveProperties<T>::NumComponents])
{
	constexpr u32 NUM_COMPONENTS = TCurveProperties<T>::NumComponents;

	const u32 numKeyFrames = compoundCurve.GetNumKeyFrames();
	for(u32 i = 0; i < numKeyFrames; i++)
	{
		const TKeyframe<T>& key = compoundCurve.GetKeyFrame(i);

		TKeyframe<float> newKey;
		newKey.Time = key.Time;

		for(u32 j = 0; j < NUM_COMPONENTS; j++)
		{
			bool addNew = true;
			if(i > 0)
			{
				const TKeyframe<float>& prevKey = keyFrames[j].back();

				bool isEqual = Math::ApproxEquals(prevKey.Value, TCurveProperties<T>::GetComponent(key.Value, j)) &&
					Math::ApproxEquals(prevKey.OutTangent, TCurveProperties<T>::GetComponent(key.InTangent, j));

				addNew = !isEqual;
			}

			if(addNew)
			{
				newKey.Value = TCurveProperties<T>::GetComponent(key.Value, j);
				newKey.InTangent = TCurveProperties<T>::GetComponent(key.InTangent, j);
				newKey.OutTangent = TCurveProperties<T>::GetComponent(key.OutTangent, j);

				keyFrames[j].push_back(newKey);
			}
		}
	}
}

template <class T>
void CombineCurve(
	const TAnimationCurve<float>* (&curveComponents)[TCurveProperties<T>::NumComponents],
	Vector<TKeyframe<T>>& output)
{
	constexpr u32 NUM_COMPONENTS = TCurveProperties<T>::NumComponents;

	// Find unique keyframe times
	Map<float, TKeyframe<T>> keyFrames;
	for(u32 i = 0; i < NUM_COMPONENTS; i++)
	{
		u32 numKeyFrames = curveComponents[i]->GetNumKeyFrames();
		for(u32 j = 0; j < numKeyFrames; j++)
		{
			const TKeyframe<float>& keyFrame = curveComponents[i]->GetKeyFrame(j);

			auto iterFind = keyFrames.find(keyFrame.Time);
			if(iterFind == keyFrames.end())
			{
				TKeyframe<T> newKeyFrame;
				newKeyFrame.Time = keyFrame.Time;

				keyFrames.insert(std::make_pair(keyFrame.Time, newKeyFrame));
			}
		}
	}

	// Populate keyframe values
	output.resize(keyFrames.size());
	u32 idx = 0;
	for(auto& entry : keyFrames)
	{
		TKeyframe<T>& keyFrame = entry.second;

		for(u32 j = 0; j < NUM_COMPONENTS; j++)
		{
			TKeyframe<float> currentKey = curveComponents[j]->EvaluateKey(keyFrame.Time, false);
			TCurveProperties<T>::SetComponent(keyFrame.Value, j, currentKey.Value);
			TCurveProperties<T>::SetComponent(keyFrame.InTangent, j, currentKey.InTangent);
			TCurveProperties<T>::SetComponent(keyFrame.OutTangent, j, currentKey.OutTangent);
		}

		output[idx] = keyFrame;
		idx++;
	}
}

Vector<SPtr<TAnimationCurve<float>>> AnimationUtility::SplitCurve3D(const SPtr<TAnimationCurve<Vector3>>& compoundCurve)
{
	Vector<TKeyframe<float>> keyFrames[3];

	if(compoundCurve)
		::SplitCurve(*compoundCurve, keyFrames);

	Vector<SPtr<TAnimationCurve<float>>> output(3);
	for(u32 i = 0; i < 3; i++)
		output[i] = B3DMakeShared<TAnimationCurve<float>>(keyFrames[i]);

	return output;
}

SPtr<TAnimationCurve<Vector3>> AnimationUtility::CombineCurve3D(const Vector<SPtr<TAnimationCurve<float>>>& curveComponents)
{
	Vector<TKeyframe<Vector3>> keyFrames;
	if(curveComponents.size() >= 3)
	{
		const TAnimationCurve<float>* curves[] = { curveComponents[0].get(), curveComponents[1].get(), curveComponents[2].get() };

		::CombineCurve(curves, keyFrames);
	}

	return B3DMakeShared<TAnimationCurve<Vector3>>(keyFrames);
}

Vector<SPtr<TAnimationCurve<float>>> AnimationUtility::SplitCurve2D(const SPtr<TAnimationCurve<Vector2>>& compoundCurve)
{
	Vector<TKeyframe<float>> keyFrames[2];

	if(compoundCurve)
		::SplitCurve(*compoundCurve, keyFrames);

	Vector<SPtr<TAnimationCurve<float>>> output(2);
	for(u32 i = 0; i < 2; i++)
		output[i] = B3DMakeShared<TAnimationCurve<float>>(keyFrames[i]);

	return output;
}

SPtr<TAnimationCurve<Vector2>> AnimationUtility::CombineCurve2D(const Vector<SPtr<TAnimationCurve<float>>>& curveComponents)
{
	Vector<TKeyframe<Vector2>> keyFrames;
	if(curveComponents.size() >= 2)
	{
		const TAnimationCurve<float>* curves[] = { curveComponents[0].get(), curveComponents[1].get() };

		::CombineCurve(curves, keyFrames);
	}

	return B3DMakeShared<TAnimationCurve<Vector2>>(keyFrames);
}

template <class T>
void AnimationUtility::SplitCurve(const TAnimationCurve<T>& compoundCurve, TAnimationCurve<float> (&output)[TCurveProperties<T>::NumComponents])
{
	constexpr u32 NUM_COMPONENTS = TCurveProperties<T>::NumComponents;

	Vector<TKeyframe<float>> keyFrames[NUM_COMPONENTS];
	::SplitCurve(compoundCurve, keyFrames);

	for(u32 i = 0; i < NUM_COMPONENTS; i++)
		output[i] = TAnimationCurve<float>(keyFrames[i]);
}

template <class T>
void AnimationUtility::CombineCurve(
	const TAnimationCurve<float> (&curveComponents)[TCurveProperties<T>::NumComponents],
	TAnimationCurve<T>& output)
{
	constexpr u32 NUM_COMPONENTS = TCurveProperties<T>::NumComponents;

	const TAnimationCurve<float>* curves[NUM_COMPONENTS];
	for(u32 i = 0; i < NUM_COMPONENTS; i++)
		curves[i] = &curveComponents[i];

	Vector<TKeyframe<T>> keyFrames;
	::CombineCurve(curves, keyFrames);

	output = TAnimationCurve<T>(keyFrames);
}

void AnimationUtility::CalculateRange(const Vector<TAnimationCurve<float>>& curves, float& outXMin, float& outXMax, float& outYMin, float& outYMax)
{
	outXMin = std::numeric_limits<float>::infinity();
	outXMax = -std::numeric_limits<float>::infinity();
	outYMin = std::numeric_limits<float>::infinity();
	outYMax = -std::numeric_limits<float>::infinity();

	for(auto& entry : curves)
	{
		const auto timeRange = entry.GetTimeRange();
		const auto valueRange = entry.CalculateRange();

		outXMin = std::min(outXMin, timeRange.first);
		outXMax = std::max(outXMax, timeRange.second);
		outYMin = std::min(outYMin, valueRange.first);
		outYMax = std::max(outYMax, valueRange.second);
	}

	if(outXMin == std::numeric_limits<float>::infinity())
		outXMin = 0.0f;

	if(outXMax == -std::numeric_limits<float>::infinity())
		outXMax = 0.0f;

	if(outYMin == std::numeric_limits<float>::infinity())
		outYMin = 0.0f;

	if(outYMax == -std::numeric_limits<float>::infinity())
		outYMax = 0.0f;
}

void AnimationUtility::CalculateRange(const Vector<SPtr<TAnimationCurve<float>>>& curves, float& outXMin, float& outXMax, float& outYMin, float& outYMax)
{
	outXMin = std::numeric_limits<float>::infinity();
	outXMax = -std::numeric_limits<float>::infinity();
	outYMin = std::numeric_limits<float>::infinity();
	outYMax = -std::numeric_limits<float>::infinity();

	for(auto& entry : curves)
	{
		const auto timeRange = entry->GetTimeRange();
		const auto valueRange = entry->CalculateRange();

		outXMin = std::min(outXMin, timeRange.first);
		outXMax = std::max(outXMax, timeRange.second);
		outYMin = std::min(outYMin, valueRange.first);
		outYMax = std::max(outYMax, valueRange.second);
	}

	if(outXMin == std::numeric_limits<float>::infinity())
		outXMin = 0.0f;

	if(outXMax == -std::numeric_limits<float>::infinity())
		outXMax = 0.0f;

	if(outYMin == std::numeric_limits<float>::infinity())
		outYMin = 0.0f;

	if(outYMax == -std::numeric_limits<float>::infinity())
		outYMax = 0.0f;
}

template <class T>
TAnimationCurve<T> AnimationUtility::ScaleCurve(const TAnimationCurve<T>& curve, float factor)
{
	i32 numKeys = (i32)curve.GetNumKeyFrames();

	Vector<TKeyframe<T>> newKeyframes(numKeys);
	for(i32 i = 0; i < numKeys; i++)
	{
		const TKeyframe<T>& key = curve.GetKeyFrame(i);
		newKeyframes[i].Time = key.Time;
		newKeyframes[i].Value = key.Value * factor;
		newKeyframes[i].InTangent = key.InTangent * factor;
		newKeyframes[i].OutTangent = key.OutTangent * factor;
	}

	return TAnimationCurve<T>(newKeyframes);
}

template <class T>
TAnimationCurve<T> AnimationUtility::OffsetCurve(const TAnimationCurve<T>& curve, float offset)
{
	i32 numKeys = (i32)curve.GetNumKeyFrames();

	Vector<TKeyframe<T>> newKeyframes(numKeys);
	for(i32 i = 0; i < numKeys; i++)
	{
		const TKeyframe<T>& key = curve.GetKeyFrame(i);
		newKeyframes[i].Time = key.Time + offset;
		newKeyframes[i].Value = key.Value;
		newKeyframes[i].InTangent = key.InTangent;
		newKeyframes[i].OutTangent = key.OutTangent;
	}

	return TAnimationCurve<T>(newKeyframes);
}

template <class T>
void AnimationUtility::CalculateTangents(Vector<TKeyframe<T>>& keyframes)
{
	using Keyframe = TKeyframe<T>;
	if(keyframes.empty())
		return;

	if(keyframes.size() == 1)
	{
		keyframes[0].InTangent = TCurveProperties<T>::GetZero();
		keyframes[0].OutTangent = TCurveProperties<T>::GetZero();

		return;
	}

	auto fnCalcTangent = [](const Keyframe& left, const Keyframe& right)
	{
		float diff = right.Time - left.Time;

		if(!Math::ApproxEquals(diff, 0.0f))
			return (right.Value - left.Value) / diff;

		return std::numeric_limits<T>::infinity();
	};

	// First keyframe
	{
		Keyframe& keyThis = keyframes[0];
		const Keyframe& keyNext = keyframes[1];

		keyThis.InTangent = TCurveProperties<T>::GetZero();
		keyThis.OutTangent = fnCalcTangent(keyThis, keyNext);
	}

	// Inner keyframes
	for(u32 i = 1; i < (u32)keyframes.size() - 1; i++)
	{
		const Keyframe& keyPrev = keyframes[i - 1];
		Keyframe& keyThis = keyframes[i];
		const Keyframe& keyNext = keyframes[i + 1];

		keyThis.OutTangent = fnCalcTangent(keyPrev, keyNext);
		keyThis.InTangent = keyThis.OutTangent;
	}

	// Last keyframe
	{
		Keyframe& keyThis = keyframes[keyframes.size() - 1];
		const Keyframe& keyPrev = keyframes[keyframes.size() - 2];

		keyThis.OutTangent = TCurveProperties<T>::GetZero();
		keyThis.InTangent = fnCalcTangent(keyPrev, keyThis);
	}
}

template B3D_EXPORT TAnimationCurve<Vector3> AnimationUtility::ScaleCurve(const TAnimationCurve<Vector3>& curve, float factor);
template B3D_EXPORT TAnimationCurve<Vector2> AnimationUtility::ScaleCurve(const TAnimationCurve<Vector2>& curve, float factor);
template B3D_EXPORT TAnimationCurve<Quaternion> AnimationUtility::ScaleCurve(const TAnimationCurve<Quaternion>& curve, float factor);
template B3D_EXPORT TAnimationCurve<float> AnimationUtility::ScaleCurve(const TAnimationCurve<float>& curve, float factor);

template B3D_EXPORT TAnimationCurve<Vector3> AnimationUtility::OffsetCurve(const TAnimationCurve<Vector3>& curve, float offset);
template B3D_EXPORT TAnimationCurve<Vector2> AnimationUtility::OffsetCurve(const TAnimationCurve<Vector2>& curve, float offset);
template B3D_EXPORT TAnimationCurve<Quaternion> AnimationUtility::OffsetCurve(const TAnimationCurve<Quaternion>& curve, float offset);
template B3D_EXPORT TAnimationCurve<float> AnimationUtility::OffsetCurve(const TAnimationCurve<float>& curve, float offset);

template B3D_EXPORT void AnimationUtility::CalculateTangents(Vector<TKeyframe<Vector3>>& keyframes);
template B3D_EXPORT void AnimationUtility::CalculateTangents(Vector<TKeyframe<Vector2>>& keyframes);
template B3D_EXPORT void AnimationUtility::CalculateTangents(Vector<TKeyframe<Quaternion>>& keyframes);
template B3D_EXPORT void AnimationUtility::CalculateTangents(Vector<TKeyframe<float>>& keyframes);

template B3D_EXPORT void AnimationUtility::SplitCurve(const TAnimationCurve<float>&, TAnimationCurve<float> (&)[1]);
template B3D_EXPORT void AnimationUtility::SplitCurve(const TAnimationCurve<Vector2>&, TAnimationCurve<float> (&)[2]);
template B3D_EXPORT void AnimationUtility::SplitCurve(const TAnimationCurve<Vector3>&, TAnimationCurve<float> (&)[3]);

template B3D_EXPORT void AnimationUtility::CombineCurve(const TAnimationCurve<float> (&)[1], TAnimationCurve<float>&);
template B3D_EXPORT void AnimationUtility::CombineCurve(const TAnimationCurve<float> (&)[2], TAnimationCurve<Vector2>&);
template B3D_EXPORT void AnimationUtility::CombineCurve(const TAnimationCurve<float> (&)[3], TAnimationCurve<Vector3>&);
