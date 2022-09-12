//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsKeyboard.h"
#include "Input/BsInput.h"
#include "Private/Linux/BsLinuxPlatform.h"

namespace bs
{

	/** Contains private data for the Linux Keyboard implementation. */
	struct Keyboard::Pimpl
	{
		bool hasInputFocus;
	};

	Keyboard::Keyboard(const String& name, Input* owner)
		: MName(name), mOwner(owner)
	{
		m = bs_new<Pimpl>();
		m->hasInputFocus = true;
	}

	Keyboard::~Keyboard()
	{
		bs_delete(m);
	}

	void Keyboard::Capture()
	{
		Lock Lock(LinuxPlatform::eventLock);

		if(m->hasInputFocus)
		{
			while (!LinuxPlatform::buttonEvents.Empty())
			{
				LinuxButtonEvent& event = LinuxPlatform::buttonEvents.Front();
				if(event.pressed)
					mOwner->_notifyButtonPressed(0, event.button, event.timestamp);
				else
					mOwner->_notifyButtonReleased(0, event.button, event.timestamp);
				LinuxPlatform::buttonEvents.Pop();
			}
		}
		else
		{
			// Discard queued data
			while (!LinuxPlatform::buttonEvents.Empty())
				LinuxPlatform::buttonEvents.Pop();
		}
	}

	void Keyboard::ChangeCaptureContext(UINT64 windowHandle)
	{
		m->hasInputFocus = windowHandle != (UINT64)-1;
	}
}

