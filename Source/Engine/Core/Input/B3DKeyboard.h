//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** Represents a single hardware keyboard. Used by the Input to report raw keyboard input events. */
	class B3D_EXPORT Keyboard
	{
	public:
		struct Pimpl;

		Keyboard(const String& name, Input* owner);
		~Keyboard();

		/** Returns the name of the device. */
		String GetName() const { return mName; }

		/** Captures the input since the last call and triggers the events on the parent Input. */
		void Capture();

	private:
		friend class Input;

		/** Changes the capture context. Should be called when focus is moved to a new window. */
		void ChangeCaptureContext(u64 windowHandle);

		String mName;
		Input* mOwner;

		Pimpl* m;
	};
} // namespace b3d
