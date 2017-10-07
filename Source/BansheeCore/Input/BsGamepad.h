//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	struct GamepadInfo;

	/** Represents a single hardware gamepad. Used by the RawInputHandler to report gamepad input events. */
	class BS_CORE_EXPORT Gamepad
	{
	public:
		struct Pimpl;

		Gamepad(const String& name, const GamepadInfo& gamepadInfo, RawInputHandler* owner);
		~Gamepad();

		/** Returns the name of the device. */
		String getName() const { return mName; }

		/** Captures the input since the last call and triggers the events on the parent RawInputHandler. */
		void capture();

		/** Minimum allowed value as reported by the axis movement events. */
		static constexpr int MIN_AXIS = -32768;

		/** Maximum allowed value as reported by the axis movement events. */
		static constexpr int MAX_AXIS = 32767;
	private:
		friend class RawInputHandler;

		/** Changes the capture context. Should be called when focus is moved to a new window. */
		void changeCaptureContext(UINT64 windowHandle);

		String mName;
		RawInputHandler* mOwner;

		Pimpl* m;
	};
}
