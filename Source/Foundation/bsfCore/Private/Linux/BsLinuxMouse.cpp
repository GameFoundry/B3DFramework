//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsMouse.h"
#include "Input/BsInput.h"
#include "Private/Linux/BsLinuxPlatform.h"

namespace bs
{
	/** Contains private data for the Linux Mouse implementation. */
	struct Mouse::Pimpl
	{
		bool hasInputFocus;
	};

	Mouse::Mouse(const String& name, Input* owner)
		: MName(name), mOwner(owner)
	{
		m = bs_new<Pimpl>();
		m->hasInputFocus = true;
	}

	Mouse::~Mouse()
	{
		bs_delete(m);
	}

	void Mouse::Capture()
	{
		Lock Lock(LinuxPlatform::eventLock);

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

	void Mouse::ChangeCaptureContext(UINT64 windowHandle)
	{
		m->hasInputFocus = windowHandle != (UINT64)-1;
	}
}

