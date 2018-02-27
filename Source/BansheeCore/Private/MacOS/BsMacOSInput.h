//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include <IOKit/hid/IOHIDLib.h>

namespace bs
{
	static constexpr UINT32 HID_NUM_MOUSE_AXES = 3;
	static constexpr UINT32 HID_NUM_GAMEPAD_AXES = 24;

	/** Available types of devices supported by the HIDManager. */
	enum class HIDType
	{
		Keyboard,
		Mouse,
		Gamepad
	};

	/**
	 * Contains information about a single element of an input device (e.g. a button, an axis), as reported by the
	 * HIDManager.
	 */
	struct HIDElement
	{
		IOHIDElementRef ref;
		IOHIDElementCookie cookie;

		INT32 min, max;
		mutable INT32 detectedMin, detectedMax;
		UINT32 usage;
	};

	/** Contains information about a single input device and its elements, as reported by the HIDManager. */
	struct HIDDevice
	{
		IOHIDDeviceRef ref;
		IOHIDQueueRef queueRef;

		String name;
		UINT32 id;

		Vector<HIDElement> axes;
		Vector<HIDElement> buttons;
		Vector<HIDElement> hats;

		UINT64 gamepadAxisTimestamps[HID_NUM_GAMEPAD_AXES];
	};

	/** Contains information about all enumerated input devices for a specific HIDManager. */
	struct HIDData
	{
		Vector<HIDDevice> devices;
		HIDType type;
		Input* owner = nullptr;

		INT32 mouseAxisValues[HID_NUM_MOUSE_AXES];
	};

	/**
	 * Provides access to the low level IO HID manager. Enumerates available input devices and reports their input to the
	 * Input object.
	 */
	class HIDManager
	{
	public:
		/**
		 *  Constructs a new HID manager object.
		 *
		 * @param type 		Determines what category of input devices will this manager enumerate and report events for.
		 * @param input		Input object that will by called by the HID manager when input events occur.
		 */
		HIDManager(HIDType type, Input* input);
		~HIDManager();

		/**
		 * Checks if any new input events have been generates and reports them to the Input object.
		 *
		 * @param[in] device		Device to read events from. If null, the events are read from all devices of the
		 * 							compatible type.
		 * @param[in] ignoreEvents 	If true the system will not trigger any external events for the reported input. This
		 * 							can be useful for situations where input is disabled, like an out-of-focus window.
		 */
		void capture(IOHIDDeviceRef device, bool ignoreEvents = false);

	private:
		IOHIDManagerRef mHIDManager = nullptr;
		HIDData mData;
	};

	/** Information about a gamepad. */
	struct GamepadInfo
	{
		UINT32 id;
		String name;
		IOHIDDeviceRef deviceRef;
		HIDManager* hid;
	};

	/**
	 * Data specific to MacOS implementation of the input system. Can be passed to platform specific implementations of
	 * the individual device types.
	 */
	struct InputPrivateData
	{
		Vector<GamepadInfo> gamepadInfos;
		HIDManager* gamepadHIDManager;
	};
}

