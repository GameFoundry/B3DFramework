//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		m->hid.capture(nullptr, !m->HasInputFocus);
	}

	void Keyboard::changeCaptureContext(UINT64 windowHandle)
	{
		m->HasInputFocus = windowHandle != (UINT64)-1;
	}
}

