//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/B3DAnimationEx.h"

using namespace b3d;
Vector<TNamedAnimationCurve<Vector3>> AnimationCurvesEx::GetPositionCurves(const SPtr<AnimationCurves>& thisPtr)
{
	return thisPtr->Position;
}

void AnimationCurvesEx::SetPositionCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Vector3>>& value)
{
	thisPtr->Position = value;
}

Vector<TNamedAnimationCurve<Quaternion>> AnimationCurvesEx::GetRotationCurves(const SPtr<AnimationCurves>& thisPtr)
{
	return thisPtr->Rotation;
}

void AnimationCurvesEx::SetRotationCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Quaternion>>& value)
{
	thisPtr->Rotation = value;
}

Vector<TNamedAnimationCurve<Vector3>> AnimationCurvesEx::GetScaleCurves(const SPtr<AnimationCurves>& thisPtr)
{
	return thisPtr->Scale;
}

void AnimationCurvesEx::SetScaleCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<Vector3>>& value)
{
	thisPtr->Scale = value;
}

Vector<TNamedAnimationCurve<float>> AnimationCurvesEx::GetGenericCurves(const SPtr<AnimationCurves>& thisPtr)
{
	return thisPtr->Generic;
}

void AnimationCurvesEx::SetGenericCurves(const SPtr<AnimationCurves>& thisPtr, const Vector<TNamedAnimationCurve<float>>& value)
{
	thisPtr->Generic = value;
}

TAnimationCurve<Vector3> RootMotionEx::GetPositionCurves(const SPtr<RootMotion>& thisPtr)
{
	return thisPtr->Position;
}

TAnimationCurve<Quaternion> RootMotionEx::GetRotationCurves(const SPtr<RootMotion>& thisPtr)
{
	return thisPtr->Rotation;
}
