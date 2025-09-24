//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DMouse.h"
#include "Input/B3DInput.h"
#include "Private/Linux/B3DLinuxPlatform.h"

using namespace b3d;

/** Contains private data for the Linux Mouse implementation. */
struct Mouse::Pimpl
{
	bool hasInputFocus;
};

Mouse::Mouse(const String& name, Input* owner)
	: mName(name), mOwner(owner)
{
	m = B3DNew<Pimpl>();
	m->HasInputFocus = true;
}

Mouse::~Mouse()
{
	B3DDelete(m);
}

void Mouse::capture()
{
	Lock lock(LinuxPlatform::eventLock);

	if(m->HasInputFocus)
	{
		double deltaX = round(LinuxPlatform::mouseMotionEvent.deltaX);
		double deltaY = round(LinuxPlatform::mouseMotionEvent.deltaY);
		double deltaZ = round(LinuxPlatform::mouseMotionEvent.deltaZ);

		if(deltaX != 0 || deltaY != 0 || deltaZ != 0)
			mOwner->NotifyMouseMovedInternal(deltaX, deltaY, deltaZ);

		LinuxPlatform::mouseMotionEvent.deltaX -= deltaX;
		LinuxPlatform::mouseMotionEvent.deltaY -= deltaY;
		LinuxPlatform::mouseMotionEvent.deltaZ -= deltaZ;
	}
	else
	{
		// Discard accumulated data
		LinuxPlatform::mouseMotionEvent.deltaX = 0;
		LinuxPlatform::mouseMotionEvent.deltaY = 0;
		LinuxPlatform::mouseMotionEvent.deltaZ = 0;
	}
}

void Mouse::changeCaptureContext(u64 windowHandle)
{
	m->HasInputFocus = windowHandle != (u64)-1;
}
