//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** Represents a single hardware mouse. Used by the Input to report raw mouse input events. */
	class BS_CORE_EXPORT Mouse
	{
	public:
		struct Pimpl;

		Mouse(const String& name, Input* owner);
		~Mouse();

		/** Returns the name of the device. */
		String getName() const { return mName; }

		/** Captures the input since the last call and triggers the events on the parent Input. */
		void capture();

	private:
		friend class Input;

		/** Changes the capture context. Should be called when focus is moved to a new window. */
		void changeCaptureContext(UINT64 windowHandle);

		String mName;
		Input* mOwner;

		Pimpl* m;
	};
}