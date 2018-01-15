//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsMouse.h"
#include "Input/BsInput.h"
#include "Linux/BsLinuxPlatform.h"

namespace bs
{
	/** Contains private data for the Linux Mouse implementation. */
	struct Mouse::Pimpl
	{
		bool hasInputFocus;
	};

	Mouse::Mouse(const String& name, Input* owner)
		: mName(name), mOwner(owner)
	{
		m = bs_new<Pimpl>();
		m->hasInputFocus = true;
	}

	Mouse::~Mouse()
	{
		bs_delete(m);
	}

	void Mouse::capture()
	{
		Lock lock(LinuxPlatform::eventLock);

		if(m->hasInputFocus)
		{
			double deltaX = round(LinuxPlatform::mouseMotionEvent.deltaX);
			double deltaY = round(LinuxPlatform::mouseMotionEvent.deltaY);
			double deltaZ = round(LinuxPlatform::mouseMotionEvent.deltaZ);

			if (deltaX != 0 || deltaY != 0 || deltaZ != 0)
				mOwner->_notifyMouseMoved(deltaX, deltaY, deltaZ);

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

	void Mouse::changeCaptureContext(UINT64 windowHandle)
	{
		m->hasInputFocus = windowHandle != (UINT64)-1;
	}
}

