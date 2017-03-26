//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "ObjectRotator.h"
#include "BsVector3.h"
#include "BsTime.h"
#include "BsMath.h"
#include "BsSceneObject.h"
#include "BsCursor.h"

namespace bs
{
	const float ObjectRotator::ROTATION_SPEED = 120.0f; // Degrees/second

	/** Wraps an angle so it always stays in [0, 360) range. */
	Degree wrapAngle2(Degree angle)
	{
		if (angle.valueDegrees() < -360.0f)
			angle += Degree(360.0f);

		if (angle.valueDegrees() > 360.0f)
			angle -= Degree(360.0f);

		return angle;
	}

	ObjectRotator::ObjectRotator(const HSceneObject& parent)
		:Component(parent), mPitch(0.0f), mYaw(0.0f), mLastButtonState(false)
	{
		// Set a name for the component, so we can find it later if needed
		setName("ObjectRotator");

		// Get handles for key bindings. Actual keys attached to these bindings will be registered during app start-up.
		mRotateObj = VirtualButton("RotateObj");
		mHorizontalAxis = VirtualAxis("Horizontal");
		mVerticalAxis = VirtualAxis("Vertical");
	}

	void ObjectRotator::update()
	{
		// Check if any movement or rotation keys are being held
		bool isRotating = gVirtualInput().isButtonHeld(mRotateObj);

		// If switch to or from rotation mode, hide or show the cursor
		if (isRotating != mLastButtonState)
		{
			if (isRotating)
				Cursor::instance().hide();
			else
				Cursor::instance().show();

			mLastButtonState = isRotating;
		}

		// If we're rotating, apply new pitch/yaw rotation values depending on the amount of rotation from the
		// vertical/horizontal axes.
		float frameDelta = gTime().getFrameDelta();
		if (isRotating)
		{
			mYaw -= Degree(gVirtualInput().getAxisValue(mHorizontalAxis) * ROTATION_SPEED * frameDelta);
			mPitch -= Degree(gVirtualInput().getAxisValue(mVerticalAxis) * ROTATION_SPEED * frameDelta);

			mYaw = wrapAngle2(mYaw);
			mPitch = wrapAngle2(mPitch);

			Quaternion yRot;
			yRot.fromAxisAngle(Vector3::UNIT_Y, Radian(mYaw));

			Quaternion xRot;
			xRot.fromAxisAngle(Vector3::UNIT_X, Radian(mPitch));

			Quaternion camRot = yRot * xRot;
			camRot.normalize();

			SO()->setRotation(camRot);
		}
	}
}