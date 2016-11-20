//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsComponent.h"
#include "BsMath.h"
#include "BsVirtualInput.h"

namespace bs
{
	/** 
	 * Component that controls movement and rotation of a Camera component. The Camera component must be attached to the 
	 * same SceneObject this component is on.
	 */
	class CameraFlyer : public Component
	{
	public:
		CameraFlyer(const HSceneObject& parent);

		/** Triggered once per frame. Allows the component to handle input and move. */
		virtual void update();

	private:
		float mCurrentSpeed; /**< Current speed of the camera. */

		Degree mPitch; /**< Current pitch rotation of the camera (looking up or down). */
		Degree mYaw; /**< Current yar rotation of the camera (looking left or right). */
		bool mLastButtonState; /**< Determines was the user rotating the camera last frame. */

		HCamera mCamera; /**< Camera component that is influenced by the flyer. */

		VirtualButton mMoveForward; /**< Key binding for moving the camera forward. */
		VirtualButton mMoveBack; /**< Key binding for moving the camera backward. */
		VirtualButton mMoveLeft; /**< Key binding for moving the camera left. */
		VirtualButton mMoveRight; /**< Key binding for moving the camera right. */
		VirtualButton mFastMove; /**< Key that speeds up movement while held. */
		VirtualButton mRotateCam; /**< Key that allows camera to be rotated while held. */
		VirtualAxis mVerticalAxis; /**< Input device axis used for controlling camera's pitch rotation (up/down). */
		VirtualAxis mHorizontalAxis; /**< Input device axis used for controlling camera's yaw rotation (left/right). */

		static const float START_SPEED; /**< Initial movement speed. */
		static const float TOP_SPEED; /**< Maximum movement speed. */
		static const float ACCELERATION; /**< Acceleration that determines how quickly to go from starting to top speed. */
		static const float FAST_MODE_MULTIPLIER; /**< Multiplier applied to the speed when the fast move button is held. */
		static const float ROTATION_SPEED; /**< Determines speed for rotation, in degrees per second. */
	};
}