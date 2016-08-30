//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsAnimationUtility.h"
#include "BsVector3.h"
#include "BsQuaternion.h"

namespace BansheeEngine
{
	void setStepTangent(const TKeyframe<Vector3>& lhsIn, const TKeyframe<Vector3>& rhsIn, 
		TKeyframe<Quaternion>& lhsOut, TKeyframe<Quaternion>& rhsOut)
	{
		for (UINT32 i = 0; i < 3; i++)
		{
			if (lhsIn.outTangent[i] != std::numeric_limits<float>::infinity() &&
				rhsIn.inTangent[i] != std::numeric_limits<float>::infinity())
				continue;

			lhsOut.outTangent[i] = std::numeric_limits<float>::infinity();
			rhsOut.inTangent[i] = std::numeric_limits<float>::infinity();
		}
	}

	void setStepTangent(const TKeyframe<Quaternion>& lhsIn, const TKeyframe<Quaternion>& rhsIn, 
		TKeyframe<Vector3>& lhsOut, TKeyframe<Vector3>& rhsOut)
	{
		for (UINT32 i = 0; i < 4; i++)
		{
			if (lhsIn.outTangent[i] != std::numeric_limits<float>::infinity() &&
				rhsIn.inTangent[i] != std::numeric_limits<float>::infinity())
				continue;

			if (i < 3)
			{
				lhsOut.outTangent[i] = std::numeric_limits<float>::infinity();
				rhsOut.inTangent[i] = std::numeric_limits<float>::infinity();
			}
		}
	}

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
		// Consider: 
		//  - Sampling multiple points to calculate tangents to improve precision
		//  - Store the original quaternion curve with the euler curve
		//    - This way conversion from euler to quaternion can be done while individual keyframes are being modified
		//      ensuring the conversion results are immediately visible, and that no accumulation error happens are curves
		//		are converted between two formats back and forth.
		//  - Don't store rotation tangents directly, instead store tangent parameters (TCB) which can be shared between
		//    both curves, and used for tangent calculation.
		const float FIT_TIME = 0.001f;

		auto eulerToQuaternion = [&](INT32 keyIdx, Vector3& angles, const Quaternion& lastQuat)
		{
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
			Vector3 angles = eulerCurve.getKeyFrame(i).value;
			Quaternion quat = eulerToQuaternion(i, angles, lastQuat);

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

			const TKeyframe<Vector3>& currentEulerKey = eulerCurve.getKeyFrame(i);
			const TKeyframe<Vector3>& nextEulerKey = eulerCurve.getKeyFrame(i + 1);

			float dt = nextKey.time - currentKey.time;
			float startFitTime = currentKey.time + dt * FIT_TIME;
			float endFitTime = currentKey.time + dt * (1.0f - FIT_TIME);

			Vector3 anglesStart = eulerCurve.evaluate(startFitTime, false);
			Vector3 anglesEnd = eulerCurve.evaluate(endFitTime, false);
			Quaternion startFitValue = eulerToQuaternion(i, anglesStart, currentKey.value);
			Quaternion endFitValue = eulerToQuaternion(i, anglesEnd, startFitValue);

			float invFitTime = 1.0f / (dt * FIT_TIME);
			currentKey.outTangent = (startFitValue - currentKey.value) * invFitTime;
			nextKey.inTangent = (nextKey.value - endFitValue) * invFitTime;

			setStepTangent(currentEulerKey, nextEulerKey, currentKey, nextKey);
		}

		return TAnimationCurve<Quaternion>(quatKeyframes);
	}

	TAnimationCurve<Vector3> AnimationUtility::quaternionToEulerCurve(const TAnimationCurve<Quaternion>& quatCurve)
	{
		// TODO: We calculate tangents by sampling. There must be an analytical way to calculate tangents when converting
		// a curve.
		const float FIT_TIME = 0.001f;

		auto quaternionToEuler = [&](const Quaternion& quat)
		{
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
			Quaternion quat = quatCurve.getKeyFrame(i).value;
			Vector3 euler = quaternionToEuler(quat);

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

			const TKeyframe<Quaternion>& currentQuatKey = quatCurve.getKeyFrame(i);
			const TKeyframe<Quaternion>& nextQuatKey = quatCurve.getKeyFrame(i + 1);

			float dt = nextKey.time - currentKey.time;
			float startFitTime = currentKey.time + dt * FIT_TIME;
			float endFitTime = currentKey.time + dt * (1.0f - FIT_TIME);

			Quaternion startQuat = Quaternion::normalize(quatCurve.evaluate(startFitTime, false));
			Quaternion endQuat = Quaternion::normalize(quatCurve.evaluate(endFitTime, false));
			Vector3 startFitValue = quaternionToEuler(startQuat);
			Vector3 endFitValue = quaternionToEuler(endQuat);

			// If fit values rotate for more than 180 degrees, wrap them so they use the shortest path
			for(int j = 0; j < 3; j++)
			{
				startFitValue[j] = fmod(startFitValue[j] - currentKey.value[j] + 180.0f, 360.0f) + currentKey.value[j] - 180.0f;
				endFitValue[j] = nextKey.value[j] + fmod(nextKey.value[j] - endFitValue[j] + 180.0f, 360.0f) - 180.0f;
			}
			
			float invFitTime = 1.0f / (dt * FIT_TIME);
			currentKey.outTangent = (startFitValue - currentKey.value) * invFitTime;
			nextKey.inTangent = (nextKey.value - endFitValue) * invFitTime;

			setStepTangent(currentQuatKey, nextQuatKey, currentKey, nextKey);
		}

		return TAnimationCurve<Vector3>(eulerKeyframes);
	}
}