//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Input/B3DInputBackend.h"
#include "Input/B3DInputFwd.h"

#include <IOKit/hid/IOHIDLib.h>

namespace b3d
{
	/** @addtogroup Platform-Internal
	 *  @{
	 */

	static constexpr u32 kHIDMouseAxisCount = 3;

	// Number of axis slots tracked per gamepad. Covers the common InputAxis entries plus a few extra slots that
	// unrecognized device axes get mapped to (starting past InputAxis::RightTrigger).
	static constexpr u32 kHIDGamepadAxisCount = 24;

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
		IOHIDElementRef Ref;
		IOHIDElementCookie Cookie;

		i32 Min, Max;
		mutable i32 DetectedMin, DetectedMax;
		u32 Usage;
	};

	/** Contains information about a single input device and its elements, as reported by the HIDManager. */
	struct HIDDevice
	{
		IOHIDDeviceRef Ref;
		IOHIDQueueRef QueueRef;

		String Name;
		u32 Id;

		Vector<HIDElement> Axes;
		Vector<HIDElement> Buttons;
		Vector<HIDElement> Hats;

		u64 GamepadAxisTimestamps[kHIDGamepadAxisCount];

		/** Currently pressed 8-way POV button, tracked so hat events can be reported with press/release semantics. */
		ButtonCode PovState;
	};

	/** Contains information about all enumerated input devices for a specific HIDManager. */
	struct HIDData
	{
		Vector<HIDDevice> Devices;
		HIDType Type;
		Input* Owner = nullptr;

		i32 MouseAxisValues[kHIDMouseAxisCount];
	};

	/**
	 * Provides access to the low level IO HID manager. Enumerates available input devices and reports their input to the
	 * Input object. Hot-plug is handled internally: the IOKit device matching/removal callbacks fire while events are
	 * pumped during Capture().
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
		HIDManager(HIDType type, Input& input);
		~HIDManager();

		/**
		 * Checks if any new input events have been generated and reports them to the Input object.
		 *
		 * @param[in] device		Device to read events from. If null, the events are read from all devices of the
		 * 							compatible type.
		 * @param[in] ignoreEvents 	If true the system will not trigger any external events for the reported input. This
		 * 							can be useful for situations where input is disabled, like an out-of-focus window.
		 */
		void Capture(IOHIDDeviceRef device, bool ignoreEvents = false);

		/** Returns the number of currently attached devices. */
		u32 GetDeviceCount() const { return (u32)mData.Devices.size(); }

		/** Returns the name of the device with the specified id, or blank if no such device is attached. */
		String GetDeviceName(u32 deviceId) const;

	private:
		IOHIDManagerRef mHIDManager = nullptr;
		HIDData mData;
	};

	/** MacOS implementation of the input backend, based on the IOKit HID manager. */
	class MacOSInputBackend final : public IInputBackend
	{
	public:
		MacOSInputBackend(Input& owner);
		~MacOSInputBackend();

		void Update() override;
		u32 GetDeviceCount(InputDevice type) const override;
		String GetDeviceName(InputDevice type, u32 deviceIndex) const override;
		void ChangeCaptureContext(u64 windowHandle) override;

	private:
		HIDManager* mKeyboard = nullptr;
		HIDManager* mMouse = nullptr;
		HIDManager* mGamepad = nullptr;
		bool mHasInputFocus = true;
	};

	/** @} */
} // namespace b3d
