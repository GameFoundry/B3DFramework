//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsGamepad.h"
#include "Input/BsInput.h"
#include "Private/MacOS/BsMacOSInput.h"

namespace bs
{
	/** Contains private data for the MacOS Gamepad implementation. */
	struct Gamepad::Pimpl
	{
		HIDManager* hid;
		IOHIDDeviceRef ref;
		bool hasInputFocus = true;
	};

	Gamepad::Gamepad(const String& name, const GamepadInfo& gamepadInfo, Input* owner)
		: MName(name), mOwner(owner)
	{
		m = bs_new<Pimpl>();
		m->hid = gamepadInfo.hid;
		m->ref = gamepadInfo.deviceRef;
	}

	Gamepad::~Gamepad()
	{
		bs_delete(m);
	}

	void Gamepad::Capture()
	{
		m->hid->Capture(m->ref, !m->hasInputFocus);
	}

	void Gamepad::ChangeCaptureContext(UINT64 windowHandle)
	{
		m->hasInputFocus = windowHandle != (UINT64)-1;
	}
}

