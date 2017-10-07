//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** Represents a single hardware keyboard. Used by the RawInputHandler to report raw keyboard input events. */
	class BS_CORE_EXPORT Keyboard
	{
	public:
		struct Pimpl;

		Keyboard(const String& name, RawInputHandler* owner);
		~Keyboard();

		/** Returns the name of the device. */
		String getName() const { return mName; }

		/** Captures the input since the last call and triggers the events on the parent RawInputHandler. */
		void capture();

	private:
		friend class RawInputHandler;

		/** Changes the capture context. Should be called when focus is moved to a new window. */
		void changeCaptureContext(UINT64 windowHandle);

		String mName;
		RawInputHandler* mOwner;

		Pimpl* m;
	};
}
