//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "CameraFlyer.h"
#include "Math/BsVector3.h"
#include "Utility/BsTime.h"
#include "Math/BsMath.h"
#include "Scene/BsSceneObject.h"
#include "Components/BsCCamera.h"
#include "Platform/BsCursor.h"

namespace bs
{
	const float CameraFlyer::START_SPEED = 4.0f;
	const float CameraFlyer::TOP_SPEED = 13.0f;
	const float CameraFlyer::ACCELERATION = 1.0f;
	const float CameraFlyer::FAST_MODE_MULTIPLIER = 2.0f;
	const float CameraFlyer::ROTATION_SPEED = 3.0f;

	/** Wraps an angle so it always stays in [0, 360) range. */
	Degree wrapAngle(Degree angle)
	{
		if (angle.valueDegrees() < -360.0f)
			angle += Degree(360.0f);

		if (angle.valueDegrees() > 360.0f)
			angle -= Degree(360.0f);

		return angle;
	}

	CameraFlyer::CameraFlyer(const HSceneObject& parent)
		:Component(parent), mPitch(0.0f), mYaw(0.0f), mLastButtonState(false)
	{
		// Set a name for the component, so we can find it later if needed
		setName("CameraFlyer");

		// Get handles for key bindings. Actual keys attached to these bindings will be registered during app start-up.
		mMoveForward = VirtualButton("Forward");
		mMoveBack = VirtualButton("Back");
		mMoveLeft = VirtualButton("Left");
		mMoveRight = VirtualButton("Right");
		mFastMove = VirtualButton("FastMove");
		mRotateCam = VirtualButton("RotateCam");
		mHorizontalAxis = VirtualAxis("Horizontal");
		mVerticalAxis = VirtualAxis("Vertical");
	}

	void CameraFlyer::update()
	{
		// Check if any movement or rotation keys are being held
		bool goingForward = gVirtualInput().isButtonHeld(mMoveForward);
		bool goingBack = gVirtualInput().isButtonHeld(mMoveBack);
		bool goingLeft = gVirtualInput().isButtonHeld(mMoveLeft);
		bool goingRight = gVirtualInput().isButtonHeld(mMoveRight);
		bool fastMove = gVirtualInput().isButtonHeld(mFastMove);
		bool camRotating = gVirtualInput().isButtonHeld(mRotateCam);

		// If switch to or from rotation mode, hide or show the cursor
		if (camRotating != mLastButtonState)
		{
			if (camRotating)
				Cursor::instance().hide();
			else
				Cursor::instance().show();

			mLastButtonState = camRotating;
		}

		// If camera is rotating, apply new pitch/yaw rotation values depending on the amount of rotation from the
		// vertical/horizontal axes.
		float frameDelta = gTime().getFrameDelta();
		if (camRotating)
		{
			mYaw += Degree(gVirtualInput().getAxisValue(mHorizontalAxis) * ROTATION_SPEED);
			mPitch += Degree(gVirtualInput().getAxisValue(mVerticalAxis) * ROTATION_SPEED);

			mYaw = wrapAngle(mYaw);
			mPitch = wrapAngle(mPitch);

			Quaternion yRot;
			yRot.fromAxisAngle(Vector3::UNIT_Y, Radian(mYaw));

			Quaternion xRot;
			xRot.fromAxisAngle(Vector3::UNIT_X, Radian(mPitch));

			Quaternion camRot = yRot * xRot;
			camRot.normalize();

			SO()->setRotation(camRot);
		}

		// If the movement button is pressed, determine direction to move in
		Vector3 direction = Vector3::ZERO;
		if (goingForward) direction += SO()->getForward();
		if (goingBack) direction -= SO()->getForward();
		if (goingRight) direction += SO()->getRight();
		if (goingLeft) direction -= SO()->getRight();

		// If a direction is chosen, normalize it to determine final direction.
		if (direction.squaredLength() != 0)
		{
			direction.normalize();

			// Apply fast move multiplier if the fast move button is held.
			float multiplier = 1.0f;
			if (fastMove)
				multiplier = FAST_MODE_MULTIPLIER;

			// Calculate current speed of the camera
			mCurrentSpeed = Math::clamp(mCurrentSpeed + ACCELERATION * frameDelta, START_SPEED, TOP_SPEED);
			mCurrentSpeed *= multiplier;
		}
		else
		{
			mCurrentSpeed = 0.0f;
		}

		// If the current speed isn't too small, move the camera in the wanted direction
		float tooSmall = std::numeric_limits<float>::epsilon();
		if (mCurrentSpeed > tooSmall)
		{
			Vector3 velocity = direction * mCurrentSpeed;
			SO()->move(velocity * frameDelta);
		}
	}
}