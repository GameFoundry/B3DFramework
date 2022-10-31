//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsGamepad.h"
#include "Input/BsInput.h"
#include "Private/MacOS/BsMacOSInput.h"

using namespace bs;

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
	m = B3DNew<Pimpl>();
	m->hid = gamepadInfo.hid;
	m->ref = gamepadInfo.deviceRef;
}

Gamepad::~Gamepad()
{
	B3DDelete(m);
}

void Gamepad::capture()
{
	m->hid->capture(m->ref, !m->HasInputFocus);
}

void Gamepad::changeCaptureContext(u64 windowHandle)
{
	m->HasInputFocus = windowHandle != (u64)-1;
}
