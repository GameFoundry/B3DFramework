//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAnimationUtility.h"
#include "BsVector3.h"
#include "BsQuaternion.h"

namespace BansheeEngine
{
	void AnimationUtility::wrapTime(float& time, float start, float end, bool loop)
	{
		float length = end - start;

		// Clamp to start or loop
		if (time < start)
		{
			if (loop)
				time = time - std::floor(time / length) * length;
			else // Clamping
				time = start;
		}

		// Clamp to end or loop
		if (time > end)
		{
			if (loop)
				time = time - std::floor(time / length) * length;
			else // Clamping
				time = end;
		}
	}

	TAnimationCurve<Quaternion> AnimationUtility::eulerToQuaternionCurve(const TAnimationCurve<Vector3>& eulerCurve)
	{
		// TODO: We calculate tangents by sampling. There must be an analytical way to calculate tangents when converting
		// a curve.
		const float FIT_TIME = 0.001f;

		auto eulerToQuaternion = [&](INT32 keyIdx, float time, const Quaternion& lastQuat)
		{
			Vector3 angles = eulerCurve.evaluate(time, false);
			Quaternion quat(
				Degree(angles.x),
				Degree(angles.y),
				Degree(angles.z));

			// Flip quaternion in case rotation is over 180 degrees (use shortest path)
			if (keyIdx > 0)
			{
				float dot = quat.dot(lastQuat);
				if (dot < 0.0f)
					quat = -quat;
			}

			return quat;
		};

		INT32 numKeys = (INT32)eulerCurve.getNumKeyFrames();
		Vector<TKeyframe<Quaternion>> quatKeyframes(numKeys);

		// Calculate key values
		Quaternion lastQuat;
		for (INT32 i = 0; i < numKeys; i++)
		{
			float time = eulerCurve.getKeyFrame(i).time;
			Quaternion quat = eulerToQuaternion(i, time, lastQuat);

			quatKeyframes[i].time = time;
			quatKeyframes[i].value = quat;
			quatKeyframes[i].inTangent = Quaternion::ZERO;
			quatKeyframes[i].outTangent = Quaternion::ZERO;

			lastQuat = quat;
		}

		// Calculate extra values between keys so we can approximate tangents. If we're sampling very close to the key
		// the values should pretty much exactly match the tangent (assuming the curves are cubic hermite)
		for (INT32 i = 0; i < numKeys - 1; i++)
		{
			TKeyframe<Quaternion>& currentKey = quatKeyframes[i];
			TKeyframe<Quaternion>& nextKey = quatKeyframes[i + 1];

			float dt = nextKey.time - currentKey.time;
			float startFitTime = currentKey.time + dt * FIT_TIME;
			float endFitTime = currentKey.time + dt * (1.0f - FIT_TIME);

			Quaternion startFitValue = eulerToQuaternion(i, startFitTime, currentKey.value);
			Quaternion endFitValue = eulerToQuaternion(i, endFitTime, startFitValue);

			float invFitTime = 1.0f / (dt * FIT_TIME);
			currentKey.outTangent = (startFitValue - currentKey.value) * invFitTime;
			nextKey.inTangent = (nextKey.value - endFitValue) * invFitTime;
		}

		return TAnimationCurve<Quaternion>(quatKeyframes);
	}

	TAnimationCurve<Vector3> AnimationUtility::quaternionToEulerCurve(const TAnimationCurve<Quaternion>& quatCurve)
	{
		// TODO: We calculate tangents by sampling. There must be an analytical way to calculate tangents when converting
		// a curve.
		const float FIT_TIME = 0.001f;

		auto quaternionToEuler = [&](float time)
		{
			Quaternion quat = quatCurve.evaluate(time, false);

			Radian x, y, z;
			quat.toEulerAngles(x, y, z);

			Vector3 euler(
				x.valueDegrees(),
				y.valueDegrees(),
				z.valueDegrees()
			);

			return euler;
		};

		INT32 numKeys = (INT32)quatCurve.getNumKeyFrames();
		Vector<TKeyframe<Vector3>> eulerKeyframes(numKeys);

		// Calculate key values
		for (INT32 i = 0; i < numKeys; i++)
		{
			float time = quatCurve.getKeyFrame(i).time;
			Vector3 euler = quaternionToEuler(time);

			eulerKeyframes[i].time = time;
			eulerKeyframes[i].value = euler;
			eulerKeyframes[i].inTangent = Vector3::ZERO;
			eulerKeyframes[i].outTangent = Vector3::ZERO;
		}

		// Calculate extra values between keys so we can approximate tangents. If we're sampling very close to the key
		// the values should pretty much exactly match the tangent (assuming the curves are cubic hermite)
		for (INT32 i = 0; i < numKeys - 1; i++)
		{
			TKeyframe<Vector3>& currentKey = eulerKeyframes[i];
			TKeyframe<Vector3>& nextKey = eulerKeyframes[i + 1];

			float dt = nextKey.time - currentKey.time;
			float startFitTime = currentKey.time + dt * FIT_TIME;
			float endFitTime = currentKey.time + dt * (1.0f - FIT_TIME);

			Vector3 startFitValue = quaternionToEuler(startFitTime);
			Vector3 endFitValue = quaternionToEuler(endFitTime);

			// If fit values rotate for more than 180 degrees, wrap them so they use the shortest path
			for(int j = 0; j < 3; j++)
			{
				startFitValue[j] = fmod(startFitValue[j] - currentKey.value[j] + 180.0f, 360.0f) + currentKey.value[j] - 180.0f;
				endFitValue[j] = nextKey.value[j] + fmod(nextKey.value[j] - endFitValue[j] + 180.0f, 360.0f) - 180.0f;
			}
			
			float invFitTime = 1.0f / (dt * FIT_TIME);
			currentKey.outTangent = (startFitValue - currentKey.value) * invFitTime;
			nextKey.inTangent = (nextKey.value - endFitValue) * invFitTime;
		}

		return TAnimationCurve<Vector3>(eulerKeyframes);
	}
}