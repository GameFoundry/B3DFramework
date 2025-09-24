//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DKeyboard.h"
#include "Input/B3DInput.h"
#include "Private/Linux/B3DLinuxPlatform.h"

using namespace b3d;


/** Contains private data for the Linux Keyboard implementation. */
struct Keyboard::Pimpl
{
	bool hasInputFocus;
};

Keyboard::Keyboard(const String& name, Input* owner)
	: mName(name), mOwner(owner)
{
	m = B3DNew<Pimpl>();
	m->HasInputFocus = true;
}

Keyboard::~Keyboard()
{
	B3DDelete(m);
}

void Keyboard::capture()
{
	Lock lock(LinuxPlatform::eventLock);

	if(m->HasInputFocus)
	{
		while(!LinuxPlatform::buttonEvents.empty())
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
		while(!LinuxPlatform::buttonEvents.empty())
			LinuxPlatform::buttonEvents.pop();
	}
}

void Keyboard::changeCaptureContext(u64 windowHandle)
{
	m->HasInputFocus = windowHandle != (u64)-1;
}
