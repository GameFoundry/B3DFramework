//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Input/BsMouse.h"
#include "Input/BsInput.h"
#include "Private/MacOS/BsMacOSInput.h"

namespace bs
{
	/** Contains private data for the MacOS Mouse implementation. */
	struct Mouse::Pimpl
	{
		explicit Pimpl(Input* owner)
			:hid(HIDType::Mouse, owner)
		{ }

		HIDManager hid;
		bool hasInputFocus = true;
	};

	Mouse::Mouse(const String& name, Input* owner)
			: MName(name), mOwner(owner)
	{
		m = bs_new<Pimpl>(owner);
	}

	Mouse::~Mouse()
	{
		bs_delete(m);
	}

	void Mouse::Capture()
	{
		m->hid.capture(nullptr, !m->hasInputFocus);
	}

	void Mouse::ChangeCaptureContext(UINT64 windowHandle)
	{
		m->hasInputFocus = windowHandle != (UINT64)-1;
	}
}

