//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/BsAnimationUtility.h"
#include "Math/BsVector3.h"
#include "Math/BsQuaternion.h"

namespace bs
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

		if(Math::approxEquals(length, 0.0f))
		{
			time = 0.0f;
			return;
		}

		// Clamp to start or loop
		if (time < start)
		{
			if (loop)
				time = time + (std::floor(end - time) / length) * length;
			else // Clamping
				time = start;
		}

		// Clamp to end or loop
		if (time > end)
		{
			if (loop)
				time = time - std::floor((time - start) / length) * length;
			else // Clamping
				time = end;
		}
	}

	SPtr<TAnimationCurve<Quaternion>> AnimationUtility::eulerToQuaternionCurve(const SPtr<TAnimationCurve<Vector3>>& eulerCurve)
	{
		// TODO: We calculate tangents by sampling which can introduce error in the tangents. The error can be exacerbated
		// by the fact we constantly switch between the two representations, possibly losing precision every time. Instead 
		// there must be an analytical way to calculate tangents when converting a curve, or a better way of dealing with
		// tangents.
		// Consider: 
		//  - Sampling multiple points to calculate tangents to improve precision
		//  - Store the original quaternion curve with the euler curve
		//    - This way conversion from euler to quaternion can be done while individual keyframes are being modified
		//      ensuring the conversion results are immediately visible, and that no accumulation error happens are curves
		//		are converted between two formats back and forth.
		//  - Don't store rotation tangents directly, instead store tangent parameters (TCB) which can be shared between
		//    both curves, and used for tangent calculation.
		//
		// If we decide to keep tangents in the current form, then we should also enforce that all euler curve tangents are
		// the same.
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

		INT32 numKeys = (INT32)eulerCurve->getNumKeyFrames();
		Vector<TKeyframe<Quaternion>> quatKeyframes(numKeys);

		// Calculate key values
		Quaternion lastQuat(BsZero);
		for (INT32 i = 0; i < numKeys; i++)
		{
			float time = eulerCurve->getKeyFrame(i).time;
			Vector3 angles = eulerCurve->getKeyFrame(i).value;
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

			const TKeyframe<Vector3>& currentEulerKey = eulerCurve->getKeyFrame(i);
			const TKeyframe<Vector3>& nextEulerKey = eulerCurve->getKeyFrame(i + 1);

			float dt = nextKey.time - currentKey.time;
			float startFitTime = currentKey.time + dt * FIT_TIME;
			float endFitTime = currentKey.time + dt * (1.0f - FIT_TIME);

			Vector3 anglesStart = eulerCurve->evaluate(startFitTime, false);
			Vector3 anglesEnd = eulerCurve->evaluate(endFitTime, false);
			Quaternion startFitValue = eulerToQuaternion(i, anglesStart, currentKey.value);
			Quaternion endFitValue = eulerToQuaternion(i, anglesEnd, startFitValue);

			float invFitTime = 1.0f / (dt * FIT_TIME);
			currentKey.outTangent = (startFitValue - currentKey.value) * invFitTime;
			nextKey.inTangent = (nextKey.value - endFitValue) * invFitTime;

			setStepTangent(currentEulerKey, nextEulerKey, currentKey, nextKey);
		}

		return bs_shared_ptr_new<TAnimationCurve<Quaternion>>(quatKeyframes);
	}

	SPtr<TAnimationCurve<Vector3>> AnimationUtility::quaternionToEulerCurve(const SPtr<TAnimationCurve<Quaternion>>& quatCurve)
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

		INT32 numKeys = (INT32)quatCurve->getNumKeyFrames();
		Vector<TKeyframe<Vector3>> eulerKeyframes(numKeys);

		// Calculate key values
		for (INT32 i = 0; i < numKeys; i++)
		{
			float time = quatCurve->getKeyFrame(i).time;
			Quaternion quat = quatCurve->getKeyFrame(i).value;
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

			const TKeyframe<Quaternion>& currentQuatKey = quatCurve->getKeyFrame(i);
			const TKeyframe<Quaternion>& nextQuatKey = quatCurve->getKeyFrame(i + 1);

			float dt = nextKey.time - currentKey.time;
			float startFitTime = currentKey.time + dt * FIT_TIME;
			float endFitTime = currentKey.time + dt * (1.0f - FIT_TIME);

			Quaternion startQuat = Quaternion::normalize(quatCurve->evaluate(startFitTime, false));
			Quaternion endQuat = Quaternion::normalize(quatCurve->evaluate(endFitTime, false));
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

		return bs_shared_ptr_new<TAnimationCurve<Vector3>>(eulerKeyframes);
	}

	Vector<SPtr<TAnimationCurve<float>>> AnimationUtility::splitCurve(const SPtr<TAnimationCurve<Vector3>>& compoundCurve)
	{
		UINT32 numKeyFrames = compoundCurve->getNumKeyFrames();
		Vector<TKeyframe<float>> keyFrames[3];

		for (UINT32 i = 0; i < numKeyFrames; i++)
		{
			const TKeyframe<Vector3>& key = compoundCurve->getKeyFrame(i);

			TKeyframe<float> newKey;
			newKey.time = key.time;

			for (UINT32 j = 0; j < 3; j++)
			{
				bool addNew = true;
				if (i > 0)
				{
					const TKeyframe<float>& prevKey = keyFrames[j].back();

					bool isEqual = Math::approxEquals(prevKey.value, key.value[j]) &&
						Math::approxEquals(prevKey.outTangent, key.inTangent[j]);

					addNew = !isEqual;
				}

				if (addNew)
				{
					newKey.value = key.value[j];
					newKey.inTangent = key.inTangent[j];
					newKey.outTangent = key.outTangent[j];

					keyFrames[j].push_back(newKey);
				}
			}
		}

		Vector<SPtr<TAnimationCurve<float>>> output(3);
		for (UINT32 i = 0; i < 3; i++)
			output[i] = bs_shared_ptr_new<TAnimationCurve<float>>(keyFrames[i]);

		return output;
	}

	SPtr<TAnimationCurve<Vector3>> AnimationUtility::combineCurve(const Vector<SPtr<TAnimationCurve<float>>>& curveComponents)
	{
		// Find unique keyframe times
		Map<float, TKeyframe<Vector3>> keyFrames;
		for(UINT32 i = 0; i < 3; i++)
		{
			if (i >= (UINT32)curveComponents.size())
				break;

			UINT32 numKeyFrames = curveComponents[i]->getNumKeyFrames();
			for (UINT32 j = 0; j < numKeyFrames; j++)
			{
				const TKeyframe<float>& keyFrame = curveComponents[i]->getKeyFrame(j);

				auto iterFind = keyFrames.find(keyFrame.time);
				if (iterFind == keyFrames.end())
				{
					TKeyframe<Vector3> newKeyFrame;
					newKeyFrame.time = keyFrame.time;

					keyFrames.insert(std::make_pair(keyFrame.time, newKeyFrame));
				}
			}
		}

		// Populate keyframe values
		Vector<TKeyframe<Vector3>> keyframeList(keyFrames.size());
		UINT32 idx = 0;
		for(auto& entry : keyFrames)
		{
			TKeyframe<Vector3>& keyFrame = entry.second;
			
			for(UINT32 j = 0; j < 3; j++)
			{
				TKeyframe<float> currentKey = curveComponents[j]->evaluateKey(keyFrame.time, false);
				keyFrame.value[j] = currentKey.value;
				keyFrame.inTangent[j] = currentKey.inTangent;
				keyFrame.outTangent[j] = currentKey.outTangent;
			}

			keyframeList[idx] = keyFrame;
			idx++;
		}

		return bs_shared_ptr_new<TAnimationCurve<Vector3>>(keyframeList);
	}

	void AnimationUtility::calculateRange(const Vector<TAnimationCurve<float>>& curves, float& xMin, float& xMax, 
		float& yMin, float& yMax)
	{
		xMin = std::numeric_limits<float>::infinity();
		xMax = -std::numeric_limits<float>::infinity();
		yMin = std::numeric_limits<float>::infinity();
		yMax = -std::numeric_limits<float>::infinity();

		for(auto& entry : curves)
		{
			const auto timeRange = entry.getTimeRange();
			const auto valueRange = entry.calculateRange();

			xMin = std::min(xMin, timeRange.first);
			xMax = std::max(xMax, timeRange.second);
			yMin = std::min(yMin, valueRange.first);
			yMax = std::max(yMax, valueRange.second);
		}

		if (xMin == std::numeric_limits<float>::infinity())
			xMin = 0.0f;

		if (xMax == -std::numeric_limits<float>::infinity())
			xMax = 0.0f;

		if (yMin == std::numeric_limits<float>::infinity())
			yMin = 0.0f;

		if (yMax == -std::numeric_limits<float>::infinity())
			yMax = 0.0f;
	}

	void AnimationUtility::calculateRange(const Vector<SPtr<TAnimationCurve<float>>>& curves, float& xMin, float& xMax, 
		float& yMin, float& yMax)
	{
		xMin = std::numeric_limits<float>::infinity();
		xMax = -std::numeric_limits<float>::infinity();
		yMin = std::numeric_limits<float>::infinity();
		yMax = -std::numeric_limits<float>::infinity();

		for(auto& entry : curves)
		{
			const auto timeRange = entry->getTimeRange();
			const auto valueRange = entry->calculateRange();

			xMin = std::min(xMin, timeRange.first);
			xMax = std::max(xMax, timeRange.second);
			yMin = std::min(yMin, valueRange.first);
			yMax = std::max(yMax, valueRange.second);
		}

		if (xMin == std::numeric_limits<float>::infinity())
			xMin = 0.0f;

		if (xMax == -std::numeric_limits<float>::infinity())
			xMax = 0.0f;

		if (yMin == std::numeric_limits<float>::infinity())
			yMin = 0.0f;

		if (yMax == -std::numeric_limits<float>::infinity())
			yMax = 0.0f;
	}

	template<class T>
	TAnimationCurve<T> AnimationUtility::scaleCurve(const TAnimationCurve<T>& curve, float factor)
	{
		INT32 numKeys = (INT32)curve.getNumKeyFrames();

		Vector<TKeyframe<T>> newKeyframes(numKeys);
		for (INT32 i = 0; i < numKeys; i++)
		{
			const TKeyframe<T>& key = curve.getKeyFrame(i);
			newKeyframes[i].time = key.time;
			newKeyframes[i].value = key.value * factor;
			newKeyframes[i].inTangent = key.inTangent * factor;
			newKeyframes[i].outTangent = key.outTangent * factor;
		}

		return TAnimationCurve<T>(newKeyframes);
	}

	template<class T>
	TAnimationCurve<T> AnimationUtility::offsetCurve(const TAnimationCurve<T>& curve, float offset)
	{
		INT32 numKeys = (INT32)curve.getNumKeyFrames();

		Vector<TKeyframe<T>> newKeyframes(numKeys);
		for (INT32 i = 0; i < numKeys; i++)
		{
			const TKeyframe<T>& key = curve.getKeyFrame(i);
			newKeyframes[i].time = key.time + offset;
			newKeyframes[i].value = key.value;
			newKeyframes[i].inTangent = key.inTangent;
			newKeyframes[i].outTangent = key.outTangent;
		}

		return TAnimationCurve<T>(newKeyframes);
	}

	template BS_CORE_EXPORT TAnimationCurve<Vector3> AnimationUtility::scaleCurve(const TAnimationCurve<Vector3>& curve, float factor);
	template BS_CORE_EXPORT TAnimationCurve<Quaternion> AnimationUtility::scaleCurve(const TAnimationCurve<Quaternion>& curve, float factor);
	template BS_CORE_EXPORT TAnimationCurve<float> AnimationUtility::scaleCurve(const TAnimationCurve<float>& curve, float factor);

	template BS_CORE_EXPORT TAnimationCurve<Vector3> AnimationUtility::offsetCurve(const TAnimationCurve<Vector3>& curve, float offset);
	template BS_CORE_EXPORT TAnimationCurve<Quaternion> AnimationUtility::offsetCurve(const TAnimationCurve<Quaternion>& curve, float offset);
	template BS_CORE_EXPORT TAnimationCurve<float> AnimationUtility::offsetCurve(const TAnimationCurve<float>& curve, float offset);
}