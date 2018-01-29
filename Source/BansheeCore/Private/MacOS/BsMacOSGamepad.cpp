//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		: mName(name), mOwner(owner)
	{
		m = bs_new<Pimpl>();
		m->hid = gamepadInfo.hid;
		m->ref = gamepadInfo.deviceRef;
	}

	Gamepad::~Gamepad()
	{
		bs_delete(m);
	}

	void Gamepad::capture()
	{
		m->hid->capture(m->ref, !m->hasInputFocus);
	}

	void Gamepad::changeCaptureContext(UINT64 windowHandle)
	{
		m->hasInputFocus = windowHandle != (UINT64)-1;
	}
}

