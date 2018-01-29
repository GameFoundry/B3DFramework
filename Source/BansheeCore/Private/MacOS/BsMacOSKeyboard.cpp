//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Input/BsKeyboard.h"
#include "Input/BsInput.h"
#include "Private/MacOS/BsMacOSInput.h"

namespace bs
{
	/** Contains private data for the MacOS Keyboard implementation. */
	struct Keyboard::Pimpl
	{
		explicit Pimpl(Input* owner)
			:hid(HIDType::Keyboard, owner)
		{ }

		HIDManager hid;
		bool hasInputFocus = true;
	};

	Keyboard::Keyboard(const String& name, Input* owner)
			: mName(name), mOwner(owner)
	{
		m = bs_new<Pimpl>(owner);
	}

	Keyboard::~Keyboard()
	{
		bs_delete(m);
	}

	void Keyboard::capture()
	{
		m->hid.capture(nullptr, !m->hasInputFocus);
	}

	void Keyboard::changeCaptureContext(UINT64 windowHandle)
	{
		m->hasInputFocus = windowHandle != (UINT64)-1;
	}
}

