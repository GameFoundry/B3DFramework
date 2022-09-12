//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsAnimationEx.h"

using namespace std::placeholders;

namespace bs
{
	Vector<TNamedAnimationCurve<Vector3>> AnimationCurvesEx::GetPositionCurves(const SPtr<AnimationCurves>& thisPtr)
	{
		return thisPtr->position;
	}

	void AnimationCurvesEx::SetPositionCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Vector3>>& value)
	{
		thisPtr->position = value;
	}

	Vector<TNamedAnimationCurve<Quaternion>> AnimationCurvesEx::GetRotationCurves(const SPtr<AnimationCurves>& thisPtr)
	{
		return thisPtr->rotation;
	}

	void AnimationCurvesEx::SetRotationCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Quaternion>>& value)
	{
		thisPtr->rotation = value;
	}

	Vector<TNamedAnimationCurve<Vector3>> AnimationCurvesEx::GetScaleCurves(const SPtr<AnimationCurves>& thisPtr)
	{
		return thisPtr->scale;
	}

	void AnimationCurvesEx::SetScaleCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Vector3>>& value)
	{
		thisPtr->scale = value;
	}

	Vector<TNamedAnimationCurve<float>> AnimationCurvesEx::GetGenericCurves(const SPtr<AnimationCurves>& thisPtr)
	{
		return thisPtr->generic;
	}

	void AnimationCurvesEx::SetGenericCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<float>>& value)
	{
		thisPtr->generic = value;
	}

	TAnimationCurve<Vector3> RootMotionEx::GetPositionCurves(const SPtr<RootMotion>& thisPtr)
	{
		return thisPtr->position;
	}

	TAnimationCurve<Quaternion> RootMotionEx::GetRotationCurves(const SPtr<RootMotion>& thisPtr)
	{
		return thisPtr->rotation;
	}
}
