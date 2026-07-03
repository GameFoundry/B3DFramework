//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/Linux/B3DLinuxInput.h"
#include "Private/Linux/B3DLinuxPlatform.h"

#include <cmath>

using namespace b3d;

LinuxMouse::LinuxMouse(Input& owner)
	: mOwner(owner)
{
}

void LinuxMouse::Capture()
{
	Lock lock(LinuxPlatform::eventLock);

	if(mHasInputFocus)
	{
		const double deltaX = std::round(LinuxPlatform::mouseMotionEvent.DeltaX);
		const double deltaY = std::round(LinuxPlatform::mouseMotionEvent.DeltaY);
		const double deltaZ = std::round(LinuxPlatform::mouseMotionEvent.DeltaZ);

		if(deltaX != 0 || deltaY != 0 || deltaZ != 0)
			mOwner.NotifyMouseMoved((i32)deltaX, (i32)deltaY, (i32)deltaZ);

		// Keep the sub-pixel remainder so slow movement doesn't get lost to rounding
		LinuxPlatform::mouseMotionEvent.DeltaX -= deltaX;
		LinuxPlatform::mouseMotionEvent.DeltaY -= deltaY;
		LinuxPlatform::mouseMotionEvent.DeltaZ -= deltaZ;
	}
	else
	{
		// Discard accumulated data
		LinuxPlatform::mouseMotionEvent.DeltaX = 0;
		LinuxPlatform::mouseMotionEvent.DeltaY = 0;
		LinuxPlatform::mouseMotionEvent.DeltaZ = 0;
	}
}

void LinuxMouse::ChangeCaptureContext(u64 windowHandle)
{
	mHasInputFocus = windowHandle != 0;
}
