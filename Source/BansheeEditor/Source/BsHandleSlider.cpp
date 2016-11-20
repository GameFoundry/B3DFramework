//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsHandleSlider.h"
#include "BsCCamera.h"
#include "BsHandleManager.h"
#include "BsDebug.h"

namespace bs
{
	HandleSlider::HandleSlider(bool fixedScale, UINT64 layer)
		: mFixedScale(fixedScale), mLayer(layer), mPosition(BsZero), mRotation(BsZero), mScale(Vector3::ONE)
		, mDistanceScale(1.0f), mState(State::Inactive), mEnabled(true), mTransformDirty(true)
	{

	}

	void HandleSlider::update(const SPtr<Camera>& camera)
	{
		if (mFixedScale)
		{
			mDistanceScale = HandleManager::instance().getHandleSize(camera, mPosition);
			mTransformDirty = true;
		}
	}

	void HandleSlider::setPosition(const Vector3& position)
	{
		mPosition = position;
		mTransformDirty = true;
	}

	void HandleSlider::setRotation(const Quaternion& rotation)
	{
		mRotation = rotation;
		mTransformDirty = true;
	}

	void HandleSlider::setScale(const Vector3& scale)
	{
		mScale = scale;
		mTransformDirty = true;
	}

	void HandleSlider::setEnabled(bool enabled)
	{
		mEnabled = enabled;
	}

	const Matrix4& HandleSlider::getTransform() const
	{
		if (mTransformDirty)
			updateCachedTransform();

		return mTransform;
	}

	const Matrix4& HandleSlider::getTransformInv() const
	{
		if (mTransformDirty)
			updateCachedTransform();

		return mTransformInv;
	}

	void HandleSlider::updateCachedTransform() const
	{
		if (mFixedScale)
		{
			mTransform.setTRS(mPosition, mRotation, mScale * mDistanceScale);
			mTransformInv.setInverseTRS(mPosition, mRotation, mScale * mDistanceScale);
		}
		else
		{
			mTransform.setTRS(mPosition, mRotation, mScale);
			mTransformInv.setInverseTRS(mPosition, mRotation, mScale);
		}

		mTransformDirty = false;
	}

	void HandleSlider::setInactive() 
	{ 
		mState = State::Inactive; 
		reset(); 
	}

	void HandleSlider::setActive(const SPtr<Camera>& camera, const Vector2I& pointerPos)
	{ 
		mState = State::Active; 
		mStartPointerPos = pointerPos; 
		mCurrentPointerPos = pointerPos;
		activate(camera, pointerPos);
	}

	void HandleSlider::setHover() 
	{ 
		mState = State::Hover; 
		reset(); 
	}

	float HandleSlider::calcDelta(const SPtr<Camera>& camera, const Vector3& position, const Vector3& direction,
		const Vector2I& pointerStart, const Vector2I& pointerEnd)
	{
		// position + direction can sometimes project behind the camera (if the camera is looking at position
		// from very close and at an angle), which will cause the delta to be reversed, so we compensate.

		float negate = 1.0f;
		Vector3 cameraDir = -camera->getRotation().zAxis();
		if (cameraDir.dot((position + direction) - camera->getPosition()) <= 0.0f)
			negate = -1.0f; // Point behind the camera

		Vector2I handleStart2D = camera->worldToScreenPoint(position);
		Vector2I handleEnd2D = camera->worldToScreenPoint(position + direction);

		Vector2I handleDir2D = handleEnd2D - handleStart2D;

		INT32 sqrdMag = handleDir2D.squaredLength();

		if (sqrdMag == 0)
			return 0.0f;

		Vector2I diffStart = pointerStart - handleStart2D;
		Vector2I diffEnd = pointerEnd - handleStart2D;

		float mag = sqrt((float)sqrdMag);
		float tStart = handleDir2D.dot(diffStart) / mag;
		float tEnd = handleDir2D.dot(diffEnd) / mag;

		float arbitraryScale = 1.0f / 100.0f;
		return negate * (tEnd - tStart) * arbitraryScale;
	}
}