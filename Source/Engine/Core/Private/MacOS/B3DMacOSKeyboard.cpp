//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DKeyboard.h"
#include "Input/B3DInput.h"
#include "Private/MacOS/B3DMacOSInput.h"

using namespace b3d;

/** Contains private data for the MacOS Keyboard implementation. */
struct Keyboard::Pimpl
{
	explicit Pimpl(Input* owner)
		: hid(HIDType::Keyboard, owner)
	{}

	HIDManager hid;
	bool hasInputFocus = true;
};

Keyboard::Keyboard(const String& name, Input* owner)
	: mName(name), mOwner(owner)
{
	m = B3DNew<Pimpl>(owner);
}

Keyboard::~Keyboard()
{
	B3DDelete(m);
}

void Keyboard::capture()
{
	m->hid.capture(nullptr, !m->HasInputFocus);
}

void Keyboard::changeCaptureContext(u64 windowHandle)
{
	m->HasInputFocus = windowHandle != (u64)-1;
}
