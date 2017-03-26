//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsComponent.h"
#include "BsMath.h"
#include "BsVirtualInput.h"

namespace bs
{
	/** Component that controls rotation of its scene object. */
	class ObjectRotator : public Component
	{
	public:
		ObjectRotator(const HSceneObject& parent);

		/** Triggered once per frame. Allows the component to handle input and move. */
		virtual void update();

	private:
		Degree mPitch; /**< Current pitch rotation of the object (up or down). */
		Degree mYaw; /**< Current yar rotation of the object (left or right). */
		bool mLastButtonState; /**< Determines was the user rotating the object last frame. */

		VirtualButton mRotateObj; /**< Key that allows object to be rotated while held. */
		VirtualAxis mVerticalAxis; /**< Input device axis used for controlling object's pitch rotation (up/down). */
		VirtualAxis mHorizontalAxis; /**< Input device axis used for controlling object's yaw rotation (left/right). */

		static const float ROTATION_SPEED; /**< Determines speed for rotation, in degrees per second. */
	};
}