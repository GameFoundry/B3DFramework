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
		: mName(name), mOwner(owner)
	{
		m = bs_new<Pimpl>();
		m->hasInputFocus = true;
	}

	Keyboard::~Keyboard()
	{
		bs_delete(m);
	}

	void Keyboard::capture()
	{
		Lock lock(LinuxPlatform::eventLock);

		if(m->hasInputFocus)
		{
			while (!LinuxPlatform::buttonEvents.empty())
			{
				LinuxButtonEvent& event = LinuxPlatform::buttonEvents.front();
				if(event.pressed)
					mOwner->NotifyButtonPressedInternal(0, event.button, event.timestamp);
				else
					mOwner->NotifyButtonReleasedInternal(0, event.button, event.timestamp);
				LinuxPlatform::buttonEvents.pop();
			}
		}
		else
		{
			// Discard queued data
			while (!LinuxPlatform::buttonEvents.empty())
				LinuxPlatform::buttonEvents.pop();
		}
	}

	void Keyboard::changeCaptureContext(UINT64 windowHandle)
	{
		m->hasInputFocus = windowHandle != (UINT64)-1;
	}
}

