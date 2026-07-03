//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DInput.h"
#include "Private/Linux/B3DLinuxInput.h"
#include "Private/Linux/B3DLinuxPlatform.h"

using namespace b3d;

LinuxKeyboard::LinuxKeyboard(Input& owner)
	: mOwner(owner)
{
}

void LinuxKeyboard::Capture()
{
	Lock lock(LinuxPlatform::eventLock);

	if(mHasInputFocus)
	{
		// Note: The platform message pump queues all button events here, including mouse buttons. The button code
		// determines which device the event gets attributed to.
		while(!LinuxPlatform::buttonEvents.empty())
		{
			const LinuxButtonEvent& event = LinuxPlatform::buttonEvents.front();
			if(event.Pressed)
				mOwner.NotifyButtonPressed(0, event.Button, event.Timestamp);
			else
				mOwner.NotifyButtonReleased(0, event.Button, event.Timestamp);

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

void LinuxKeyboard::ChangeCaptureContext(u64 windowHandle)
{
	mHasInputFocus = windowHandle != 0;
}
