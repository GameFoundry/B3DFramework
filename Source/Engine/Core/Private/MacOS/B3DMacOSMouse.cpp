//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/B3DMouse.h"
#include "Input/B3DInput.h"
#include "Private/MacOS/B3DMacOSInput.h"

using namespace b3d;

/** Contains private data for the MacOS Mouse implementation. */
struct Mouse::Pimpl
{
	explicit Pimpl(Input* owner)
		: hid(HIDType::Mouse, owner)
	{}

	HIDManager hid;
	bool hasInputFocus = true;
};

Mouse::Mouse(const String& name, Input* owner)
	: mName(name), mOwner(owner)
{
	m = B3DNew<Pimpl>(owner);
}

Mouse::~Mouse()
{
	B3DDelete(m);
}

void Mouse::capture()
{
	m->hid.capture(nullptr, !m->HasInputFocus);
}

void Mouse::changeCaptureContext(u64 windowHandle)
{
	m->HasInputFocus = windowHandle != (u64)-1;
}
